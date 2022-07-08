const { func } = require('joi');
const mqtt = require('mqtt')
var wallet = require('ethereumjs-wallet').default;
var ethUtil = require('ethereumjs-util');


// Mqtt connection
const host = 'localhost'
const port = '1883'
const DATA_INTERVAL = 60000;

// Simulated device data
const PRIVATE_KEY="1111111111111111111111111111111111111111111111111111111111111111"
/*const deviceId = wallet.fromPrivateKey(
    Buffer.from(PRIVATE_KEY, 'hex'))
    .getPublicKey()
    .slice(0,20)
    .toString('hex');*/
const deviceId = "0422a23806d731d6864f0c65551400627d6e60a1";

const topic = `/device/${deviceId}/data`

main();

async function main() {
  const mqttClient = await mqtt.connect(`mqtt://${host}:${port}`);
  console.log(`Device ID: ${deviceId}`)   
  console.log(`Topic: ${topic}`)
  console.log("Connecting to MQTT broker on " + host + ":" + port)

  publishData(mqttClient);
  setInterval(publishData, DATA_INTERVAL, mqttClient);
}

function publishData(mqttClient) {
    let message = generateDataMessage();
    let signature = signMessage(message);
    let payload = buildPayload(message, signature);
    mqttClient.publish(topic, payload);
    console.log(`Published message: ${payload}`)
}

function generateDataMessage() {
    let str = '{"steps":';
    str += Math.floor(Math.random() * 100)
    str += ',"timestamp":';
    str += Date.now();
    str += '}';
    return str;
}

function signMessage(message) {
    let str = JSON.stringify(message);
    let hash = ethUtil.keccak256(Buffer.from(str));
    console.log(`Message Hash: ${hash.toString('hex')}`)
    let signature = ethUtil.ecsign(hash, Buffer.from(PRIVATE_KEY, 'hex'));
    return (signature.r.toString("hex")+signature.s.toString("hex"));
}


function buildPayload(message, signature) {
    let str = '{"message":';
    str += message;
    str += ',"deviceId":"' + deviceId
    str += '","signature":"';
    str += signature;
    str += '","hash":"keccak256","curve":"secp256k1"}';
    return str;
}