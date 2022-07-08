// SPDX-License-Identifier: MIT
pragma solidity ^0.8.4;

import "@openzeppelin/contracts/access/Ownable.sol";
import "hardhat/console.sol";

contract DeviceBinding is Ownable {

    struct Device {
        address deviceAddress;
        address owner;
    }

    mapping (address => Device) public devices;
    

    event OwnershipAssigned (address _deviceAddress, address _ownerAddress);
    event OwnershipRenounced (address _deviceAddress);

    constructor() {
        console.log("Deploying DeviceBinding contract");
    }

    function bindDevice(address _deviceAddress, address _ownerAddress) public onlyOwner returns (bool) {
        require(devices[_deviceAddress].owner == address(0), "device has already been bound");
        Device memory newDevice = Device(_deviceAddress, _ownerAddress);
        devices[_deviceAddress] = newDevice;
        
        emit OwnershipAssigned(_deviceAddress,_ownerAddress );
        return true;
    }

    function renounceOwnership(address _deviceAddress) public returns (bool) {
        require((devices[_deviceAddress].owner == msg.sender) || (msg.sender == this.owner()) , "you don't have the rights to renounce ownership of this device");
        delete devices[_deviceAddress];
        emit OwnershipRenounced(_deviceAddress);
        return true;
    }

}