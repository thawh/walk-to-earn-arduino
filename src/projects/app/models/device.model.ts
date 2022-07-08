import { Table, Column, DataType, PrimaryKey } from 'sequelize-typescript';
import BaseModel from './base';

enum deviceStatus {
    IS_ENABLED,
    IS_SUSPENDED
}

@Table({
  tableName: 'device',
  indexes: [
    { name: 'device_id', fields: ['device_id'], unique: true }
  ]
})
export class DeviceModel extends BaseModel<DeviceModel> {

  @PrimaryKey
  @Column({
    type: DataType.STRING(64)
  })
  // We actually store the keccak256 of the device's public key
  public device_id!: string;
  
}