#ifndef QBUTTPLUGMESSAGES_H
#define QBUTTPLUGMESSAGES_H

#include "qbuttplugenums.h"

#include <QMap>
#include <QStringList>

struct ButtplugMessageBase
{
  virtual ~ButtplugMessageBase() {}
  qint64 Id = -1;
  QString MessageType;
};

struct ButtplugOk : ButtplugMessageBase
{
  ButtplugOk() :  ButtplugMessageBase() { MessageType = "Ok"; }
  ~ButtplugOk() override {}
};

struct ButtplugError : ButtplugMessageBase
{
  ButtplugError() :  ButtplugMessageBase() { MessageType = "Error"; }
  ~ButtplugError() override {}
  QString ErrorMessage;
  QtButtplug::Error ErrorCode = QtButtplug::ERROR_OK;
};

struct ButtplugPing : ButtplugMessageBase
{
  ButtplugPing() :  ButtplugMessageBase() { MessageType = "Ping"; }
  ~ButtplugPing() override {}
};

struct ButtplugRequestServerInfo : ButtplugMessageBase
{
  ButtplugRequestServerInfo() :  ButtplugMessageBase() { MessageType = "RequestServerInfo"; }
  ~ButtplugRequestServerInfo() override {}
  QString ClientName;
  // ProtocolV1 and up only
  QtButtplug::ButtplugProtocolVersion MessageVersion = QtButtplug::ProtocolV3;
};

struct ButtplugServerInfo : ButtplugMessageBase
{
  ButtplugServerInfo() :  ButtplugMessageBase() { MessageType = "ServerInfo"; }
  ~ButtplugServerInfo() override {}
  QString ServerName;
  QtButtplug::ButtplugProtocolVersion MessageVersion = QtButtplug::ProtocolV3;
  qint64 MaxPingTime = 10;
  // ProtocolV0
  qint64 MajorVersion;
  // ProtocolV0
  qint64 MinorVersion;
  // ProtocolV0
  qint64 BuildVersion;
};

struct ButtplugStartScanning : ButtplugMessageBase
{
  ButtplugStartScanning() :  ButtplugMessageBase() { MessageType = "StartScanning"; }
  ~ButtplugStartScanning() override {}
};

struct ButtplugStopScanning : ButtplugMessageBase
{
  ButtplugStopScanning() :  ButtplugMessageBase() { MessageType = "StopScanning"; }
  ~ButtplugStopScanning() override {}
};

struct ButtplugScanningFinished : ButtplugMessageBase
{
  ButtplugScanningFinished() :  ButtplugMessageBase() { MessageType = "ScanningFinished"; }
  ~ButtplugScanningFinished() override {}
};

struct ButtplugRequestDeviceList : ButtplugMessageBase
{
  ButtplugRequestDeviceList() :  ButtplugMessageBase() { MessageType = "RequestDeviceList"; }
  ~ButtplugRequestDeviceList() override {}
};

struct ButtplugClientDeviceMessageAttribute
{
  // ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3
  QString MessageType;
  // ProtocolV2, ProtocolV3
  quint32 StepCount = 0;
  // ProtocolV3
  QString FeatureDescriptor;
  // ProtocolV3
  QString ActuatorType;
  // ProtocolV3
  QString SensorType;
  // ProtocolV3
  QList<QPair<qint32, qint32>> SensorRange;
  // ProtocolV3
  QStringList Endpoints;
};

struct ButtplugDevice
{
  // ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3
  QString DeviceName;
  // ProtocolV3
  QString DeviceDisplayName;
  // ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3
  qint64 DeviceIndex = -1;
  // ProtocolV3
  qint64 DeviceMessageTimingGap = 10;
  // ProtocolV0, ProtocolV1, ProtocolV2, ProtocolV3
  QMap<QString, QList<ButtplugClientDeviceMessageAttribute>> DeviceMessages;
};

struct ButtplugDeviceList : ButtplugMessageBase
{
  ButtplugDeviceList() :  ButtplugMessageBase() { MessageType = "DeviceList"; }
  ~ButtplugDeviceList() override {}
  QList<ButtplugDevice> Devices;
};

struct ButtplugDeviceAdded : ButtplugMessageBase, ButtplugDevice
{
  ButtplugDeviceAdded() :  ButtplugMessageBase() { MessageType = "DeviceAdded"; }
  ~ButtplugDeviceAdded() override {}
};

struct ButtplugDeviceRemoved : ButtplugMessageBase
{
  ButtplugDeviceRemoved() :  ButtplugMessageBase() { MessageType = "DeviceRemoved"; }
  ~ButtplugDeviceRemoved() override {}
  qint64 DeviceIndex = -1;
};

struct ButtplugStopDeviceCmd : ButtplugMessageBase
{
  ButtplugStopDeviceCmd() :  ButtplugMessageBase() { MessageType = "StopDeviceCmd"; }
  ~ButtplugStopDeviceCmd() override {}
  qint64 DeviceIndex = -1;
};

struct ButtplugStopAllDevices : ButtplugMessageBase
{
  ButtplugStopAllDevices() :  ButtplugMessageBase() { MessageType = "StopAllDevices"; }
  ~ButtplugStopAllDevices() override {}
};

// ProtocolV3
struct ButtplugScalarCmdElem
{
  qint64 Index = -1;
  double Scalar = 0.0; // range of [0.0-1.0]
  QString ActuatorType;
};

// ProtocolV3
struct ButtplugScalarCmd : ButtplugMessageBase
{
  ButtplugScalarCmd() :  ButtplugMessageBase() { MessageType = "ScalarCmd"; }
  ~ButtplugScalarCmd() override {}
  qint64 DeviceIndex = -1;
  QList<ButtplugScalarCmdElem> Scalars;
};

// ProtocolV1, ProtocolV2
struct ButtplugVibrateCmdElem
{
  qint64 Index = -1;
  double Speed = 0.0; // range of [0.0-1.0]
};

// ProtocolV1, ProtocolV2
struct ButtplugVibrateCmd : ButtplugMessageBase
{
  ButtplugVibrateCmd() :  ButtplugMessageBase() { MessageType = "VibrateCmd"; }
  ~ButtplugVibrateCmd() override {}
  qint64 DeviceIndex = -1;
  QList<ButtplugVibrateCmdElem> Speeds;
};

// ProtocolV0
struct ButtplugSingleMotorVibrateCmd : ButtplugMessageBase
{
  ~ButtplugSingleMotorVibrateCmd() override {}
  qint64 DeviceIndex = -1;
  double Speed = 0.0; // Vibration speed with a range of [0.0-1.0]
};

// ProtocolV1, ProtocolV2, ProtocolV3
struct ButtplugLinearCmdElem
{
  qint64 Index = -1;
  qint64 Duration = 0; // ms
  double Position = 0.0; // range of [0.0-1.0]
};

struct ButtplugLinearCmd : ButtplugMessageBase
{
  ButtplugLinearCmd() :  ButtplugMessageBase() { MessageType = "LinearCmd"; }
  ~ButtplugLinearCmd() override {}
  qint64 DeviceIndex = -1;
  QList<ButtplugLinearCmdElem> Vectors;
};

// ProtocolV1, ProtocolV2, ProtocolV3
struct ButtplugRotateCmdElem
{
  qint64 Index = -1;
  double Speed = 0.0; // range of [0.0-1.0]
  bool Clockwise = true;
};

// ProtocolV1, ProtocolV2, ProtocolV3
struct ButtplugRotateCmd : ButtplugMessageBase
{
  ButtplugRotateCmd() :  ButtplugMessageBase() { MessageType = "RotateCmd"; }
  ~ButtplugRotateCmd() override {}
  qint64 DeviceIndex = -1;
  QList<ButtplugRotateCmdElem> Rotations;
};

struct ButtplugSensorCommandBase : ButtplugMessageBase
{
  ~ButtplugSensorCommandBase() override {}
  qint64 DeviceIndex = -1;
  qint64 SensorIndex = -1;
  QString SensorType;
};

// ProtocolV3
struct ButtplugSensorReadCmd : ButtplugSensorCommandBase
{
  ButtplugSensorReadCmd() :  ButtplugSensorCommandBase() { MessageType = "SensorReadCmd"; }
  ~ButtplugSensorReadCmd() override {}
};

// ProtocolV3
struct ButtplugSensorReading : ButtplugSensorCommandBase
{
  ButtplugSensorReading() :  ButtplugSensorCommandBase() { MessageType = "SensorReading"; }
  ~ButtplugSensorReading() override {}
  // Signed integers are used due to varying return values (for instance, RSSI is negative,
  // battery is [0, 100], buttons are [0, 1], etc...).
  // Information on formatting/units of measurement/etc may be included in feature descriptors.
  QList<qint32> Data;
};

// ProtocolV3
struct ButtplugSensorSubscribeCmd : ButtplugSensorCommandBase
{
  ButtplugSensorSubscribeCmd() :  ButtplugSensorCommandBase() { MessageType = "SensorSubscribeCmd"; }
  ~ButtplugSensorSubscribeCmd() override {}
};

// ProtocolV3
struct ButtplugSensorUnsubscribeCmd : ButtplugSensorCommandBase
{
  ButtplugSensorUnsubscribeCmd() :  ButtplugSensorCommandBase() { MessageType = "SensorUnsubscribeCmd"; }
  ~ButtplugSensorUnsubscribeCmd() override {}
};

// ProtocolV2
struct ButtplugBatteryLevelCmd : ButtplugMessageBase
{
  ButtplugBatteryLevelCmd() :  ButtplugMessageBase() { MessageType = "BatteryLevelCmd"; }
  ~ButtplugBatteryLevelCmd() override {}
  qint64 DeviceIndex = -1;
};

// ProtocolV2
struct ButtplugBatteryLevelReading : ButtplugMessageBase
{
  ButtplugBatteryLevelReading() :  ButtplugMessageBase() { MessageType = "BatteryLevelReading"; }
  ~ButtplugBatteryLevelReading() override {}
  qint64 DeviceIndex = -1;
  double BatteryLevel = 0.0;
};

// ProtocolV2
struct ButtplugRSSILevelCmd : ButtplugMessageBase
{
  ButtplugRSSILevelCmd() :  ButtplugMessageBase() { MessageType = "RSSILevelCmd"; }
  ~ButtplugRSSILevelCmd() override {}
  qint64 DeviceIndex = -1;
};

// ProtocolV2
struct ButtplugRSSILevelReading : ButtplugMessageBase
{
  ButtplugRSSILevelReading() :  ButtplugMessageBase() { MessageType = "RSSILevelReading"; }
  ~ButtplugRSSILevelReading() override {}
  qint64 DeviceIndex = -1;
  qint32 RSSILevel = 0; // RSSI Level, usually expressed as db gain, usually [-100:0]
};

// ProtocolV2
struct ButtplugRawWriteCmd : ButtplugMessageBase
{
  ButtplugRawWriteCmd() :  ButtplugMessageBase() { MessageType = "RawWriteCmd"; }
  ~ButtplugRawWriteCmd() override {}
  qint64 DeviceIndex = -1;
  QString Endpoint;
  QByteArray Data;
  bool WriteWithResponse = true;
};

// ProtocolV2
struct ButtplugRawReadCmd : ButtplugMessageBase
{
  ButtplugRawReadCmd() :  ButtplugMessageBase() { MessageType = "RawReadCmd"; }
  ~ButtplugRawReadCmd() override {}
  qint64 DeviceIndex = -1;
  QString Endpoint;
  qint64 ExpectedLength = 0;
  bool WaitForData = true;
};

// ProtocolV2
struct ButtplugRawReading : ButtplugMessageBase
{
  ButtplugRawReading() :  ButtplugMessageBase() { MessageType = "RawReading"; }
  ~ButtplugRawReading() override {}
  qint64 DeviceIndex = -1;
  QString Endpoint;
  QByteArray Data;
};

// ProtocolV2
struct ButtplugRawSubscribeCmd : ButtplugMessageBase
{
  ButtplugRawSubscribeCmd() :  ButtplugMessageBase() { MessageType = "RawSubscribeCmd"; }
  ~ButtplugRawSubscribeCmd() override {}
  qint64 DeviceIndex = -1;
  QString Endpoint;
};

// ProtocolV2
struct ButtplugRawUnsubscribeCmd : ButtplugMessageBase
{
  ButtplugRawUnsubscribeCmd() :  ButtplugMessageBase() { MessageType = "RawUnsubscribeCmd"; }
  ~ButtplugRawUnsubscribeCmd() override {}
  qint64 DeviceIndex = -1;
  QString Endpoint;
};

#endif //QBUTTPLUGMESSAGES_H
