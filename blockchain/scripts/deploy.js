async function main() {
    const [deployer] = await ethers.getSigners();
  
    console.log("Deploying contracts with the account:", deployer.address);
  
    let balanceRau = await deployer.getBalance();
    let balanceIOTX = balanceRau / Math.pow(10,18);
    console.log("Account balance:", balanceIOTX, " IOTX");
  
    const DeviceBinding = await ethers.getContractFactory("DeviceBinding");
    console.log("Deploying DeviceBinding contract");
    const deviceBinding = await DeviceBinding.deploy();

    const DevicesRegistry = await ethers.getContractFactory("DevicesRegistry");
    console.log("Deploying DevicesRegistry contract");
    const devicesRegistry = await DevicesRegistry.deploy();

    blockNumber = devicesRegistry.deployTransaction.blockNumber;
  
    console.log("DevicesRegistry Contract")
    console.log("address:", devicesRegistry.address);
    console.log("block:", blockNumber);
  }
  
  main()
    .then(() => process.exit(0))
    .catch((error) => {
      console.error(error);
      process.exit(1);
    });
