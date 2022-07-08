#include <FlashStorage.h>
#include <FlashAsEEPROM.h>

#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include <ArduinoECCX08.h>
#include <Arduino_LSM6DS3.h>


// Use accelerometer as a pedometer
#include <SparkFunLSM6DS3.h>
#include "Wire.h"
#include "SPI.h"


// User configuration
// ------------------------------------------------ 
// MQTT broker, eg. "test.mosquitto.org".
const char mqttBroker[] = "172.20.10.5";
// MQTT port, eg. 1883. 
const int mqttPort = 1883;
// Wifi ssid.
const char wifiSsid[] = "iPhone di Simone";
// Wifi password.
const char wifiPass[] = "simonewifi123";
// ------------------------------------------------ 

LSM6DS3Core myIMU( I2C_MODE, 0x6A );


WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// The slot to use in the ATECC608A chip.
const int slot = 0;

// Variables to store the deviceId and public key.
String deviceId = "04b687e298ad52eec4fe32b27af45247f3659062";
byte publicKey[64] = {0};

// Variable to store the MQTT topic.
String topic = "";

// Variable to store total steps taken
int steps;

// Variable to store steps taken since device power on
int chipSteps;

// We store the accumulated steps in flash on every steps read.
FlashStorage(steps_storage, int);


void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Verifiable IoT data from Arduino");
  Serial.println();

  // Init the pedometer
  setupIMU();

  // Init steps from flash
  steps = steps_storage.read();
  Serial.print("Stored steps: ");
  Serial.println(steps); 
  
  // Init the ATECC608A crypto chip.
  if (!ECCX08.begin()) {
    Serial.println("Failed to communicate with ECC508/ECC608!");
    while (1);
  }

  // Verify the crypto chip is locked (ie. configured.).
  if (!ECCX08.locked()) {
    Serial.println("The ECC508/ECC608 is not locked!");
    Serial.println("ECC508/ECC608 needs to be configured and locked before proceeding");
    while (1);
  }

  // Connecto to the WiFi network.
  initWiFi();

  // Connect to the MQTT broker
  initMqtt();

  // Retrieve the public key for the corresponding slot in the ECC508/ECC608.
  ECCX08.generatePublicKey(slot, publicKey);
  String pubKey = BufferHexToStr(publicKey, sizeof(publicKey));
  
  Serial.print("Public key of slot ");
  Serial.print(slot);
  Serial.println(" is: " + pubKey);

  topic = "/device/" + pubKey + "/data";
  Serial.print("Topic: ");
  Serial.println(topic);
}

void loop() {
  Serial.println("\n---- NEW MESSAGE ----");
  // Update steps taken
  while (!readSteps()) { delay(1000); };

  // Build a message using data from the accelerometer.
  
  // TODO Sync the RTC using WiFi and get the real epoch. Using a hardcoded one for now.
  String timestamp = "1655543438"; 
  String message = buildMessage(steps, timestamp);

  // Hash the message using sha256.
  byte hash[32] = {0};
  ECCX08.beginSHA256();
  ECCX08.endSHA256((byte*)message.c_str(), message.length(), hash);
  Serial.println("Message hash is: "+ BufferHexToStr(hash, sizeof(hash)));

  // Sign the message.
  byte signature[64];
  ECCX08.ecSign(slot, hash, signature);
  Serial.println("Signature is " + BufferHexToStr(signature, sizeof(signature)));

  // Publish the message over MQTT.
  String mqttMessage = buildSignedMessage(message, signature, sizeof(signature));
  Serial.print("Sending mqtt message: ");
  Serial.println(mqttMessage);
  mqttClient.beginMessage(topic);
  mqttClient.print(mqttMessage);
  mqttClient.endMessage();

  delay(3000);
  
  //while(1);
}

String BufferHexToStr(const byte input[], int inputLength) {
  String hexString = "";
  for (int i = 0; i < inputLength; i++) {
    hexString+=String(input[i] >> 4, HEX);
    hexString+=String(input[i] & 0x0f, HEX);
  }
  return hexString;
}

// Connects to the wifi network.
void initWiFi() 
{
    #if defined(ESP8266) || defined(ESP32)
    WiFi.mode(WIFI_STA);
    #endif
    WiFi.begin(wifiSsid, wifiPass);
    Serial.print(F("Connecting to WiFi .."));
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(F("\r\nConnected. IP: "));
    Serial.println(WiFi.localIP());
}

// Connects to the MQTT broker.
void initMqtt()
{
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(mqttBroker);
  if (!mqttClient.connect(mqttBroker, mqttPort)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");
}

bool readSteps() {
  uint8_t readDataByte = 0;
  int stepsTaken;
  
  //Read the 16bit value by two 8bit operations
  myIMU.readRegister(&readDataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_H);
  stepsTaken = ((uint16_t)readDataByte) << 8;
  
  myIMU.readRegister(&readDataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_L);
  stepsTaken |= readDataByte;

  if (stepsTaken == chipSteps) return false;
  steps += (stepsTaken - chipSteps);
  chipSteps = stepsTaken;

  // Store current total steps in ase of power off 
  steps_storage.write(steps);
  return true;
}


// Constructs a signed message from a message and a signature.
String buildSignedMessage(String message, byte* signature, int signatureSize)
{
  String messageWithSignature = "{\"message\":";
  messageWithSignature += message;
  messageWithSignature += ",\"signature\":\"";
  messageWithSignature += BufferHexToStr(signature, signatureSize);

  /* Add some device details like S/N, IMEI etc... */
  messageWithSignature += "\",\"deviceId\":\"";
  messageWithSignature += deviceId;
  
  // Add some specs about the cryptography used
  messageWithSignature += "\",\"cryptography\":\"ECC\"";
  messageWithSignature += ",\"curve\":\"secp256r1\"";
  messageWithSignature += ",\"hash\":\"sha256\"";

  // close the message
  messageWithSignature += "}";

  return messageWithSignature;
}

// Builds a data message given steps taken and timestamp.
String buildMessage(int steps, String timestamp)
{
  Serial.println("Building message:");
  String message = "{\"steps\":";
  message += String(steps);
  message += ",\"timestamp\":";
  message += timestamp;
  message += "}";
  return message;
}

void setupIMU()
{
  //Call .beginCore() to configure the IMU
  if( myIMU.beginCore() != 0 )
  {
    Serial.println("Error at beginCore() configure the IMU.");
  }
  else
  {
    Serial.println("IMU beginCore() passed.");
  }


  //Error accumulation variable
  uint8_t errorAccumulator = 0;

  uint8_t dataToWrite = 0;  //Temporary variable

  //Setup the accelerometer******************************
  dataToWrite = 0; //Start Fresh!
  dataToWrite |= LSM6DS3_ACC_GYRO_BW_XL_200Hz;
  dataToWrite |= LSM6DS3_ACC_GYRO_FS_XL_2g;
  dataToWrite |= LSM6DS3_ACC_GYRO_ODR_XL_26Hz;

  // //Now, write the patched together data
  errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, dataToWrite);

  //Set the ODR bit
  errorAccumulator += myIMU.readRegister(&dataToWrite, LSM6DS3_ACC_GYRO_CTRL4_C);
  dataToWrite &= ~((uint8_t)LSM6DS3_ACC_GYRO_BW_SCAL_ODR_ENABLED);

  // Enable embedded functions -- ALSO clears the pdeo step count
  errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0x3E);
  // Enable pedometer algorithm
  errorAccumulator += myIMU.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0x40);
  // Step Detector interrupt driven to INT1 pin
  errorAccumulator += myIMU.writeRegister( LSM6DS3_ACC_GYRO_INT1_CTRL, 0x10 );
  
  if( errorAccumulator )
  {
    Serial.println("Problem configuring the IMU.");
  }
  else
  {
    Serial.println("IMU O.K.");
  } 
}

void resetIMU() {
  setupIMU();
  chipSteps = 0;
  steps = 0;
  steps_storage.write(0);
}