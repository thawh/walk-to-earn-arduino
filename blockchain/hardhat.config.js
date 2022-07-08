require("@nomiclabs/hardhat-waffle");
require("@nomiclabs/hardhat-web3");
require('dotenv').config()
const fs = require('fs');

const IOTEX_PRIVATE_KEY = process.env.IOTEX_PRIVATE_KEY;

task("registerDevice", "Authorize a new device by adding it to the DevicesRegistry contract")
  .addParam("deviceid", "The device id (can be anything of the size of an EVM address)")
  .addParam("registrycontract", "The DevicesRegistry contract address.")
  .setAction(async ({deviceid,registrycontract}) => {
    console.log("Registering device:", deviceid, ", to registry contract: ", registrycontract);

    const DevicesRegistry = await ethers.getContractFactory("DevicesRegistry");
    const devicesRegistry = await DevicesRegistry.attach(registrycontract);
    let ret = await devicesRegistry.registerDevice(deviceid);
    console.log ("registerDevice:", ret);
  });

task("bindDevice", "Binding device to an owner's account")
  .addParam("deviceaddress", "The device address (matching the private key the device uses to sign its data).")
  .addParam("owneraddress", "The device owner address.")
  .addParam("contractaddress", "The DevicesRegistry contract address.")
  .setAction(async ({deviceaddress,owneraddress, contractaddress}) => {
    console.log("Binding device:", deviceaddress, ",to owner: ", owneraddress);

    const DeviceBinding = await ethers.getContractFactory("DeviceBinding");
    const deviceBinding = await DeviceBinding.attach(contractaddress);
    let ret = await deviceBinding.bindDevice(deviceaddress, owneraddress );
    console.log ("bindingDevice:", ret);
  });


module.exports = {
  solidity: "0.8.4",
  networks: {
    testnet: {
      // These are the official IoTeX endpoints to be used by Ethereum clients
      // Testnet https://babel-api.testnet.iotex.io
      // Mainnet https://babel-api.mainnet.iotex.io
      url: `https://babel-api.testnet.iotex.io`,

      // Input your Metamask testnet account private key here
      accounts: [`${IOTEX_PRIVATE_KEY}`],
    },
  },
};
