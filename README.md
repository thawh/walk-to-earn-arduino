# MachineFi - Walk to Earn with Arduino Nano 33 IoT and IoTeX

**This is a WIP.**

This project is based on the IoTeX MachineFi Get Started at:
https://developers.iotex.io/posts/deploy-a-machinefi-dapp

Please check out the devices/nano-iot-33 folder: configure `secrets.h` and flash the `verifiable-steps-counter.ino` to your Arduino Nano 33 IoT board.

Follow the MachineFi Get Started up to the [simulated data section](https://developers.iotex.io/posts/deploy-a-machinefi-dapp#send-simulated-data) (excluded). 

Shake your Nano 33 board to send some steps to the W3bStream-Preview service!

Remember to register the board id using the hardhat script:

```
npx hardhat registerDevice --deviceid <see your device id in the logs> --registrycontract <your contract address> --network testnet
```
