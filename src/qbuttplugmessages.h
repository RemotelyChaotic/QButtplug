#ifndef QBUTTPLUGMESSAGES_H
#define QBUTTPLUGMESSAGES_H

#include "qbuttplugenums.h"

#include <QMap>
#include <QStringList>

// https://github.com/buttplugio/buttplug/blob/master/buttplug/buttplug-schema/schema/buttplug-schema.json
// https://github.com/buttplugio/buttplug/blob/master/buttplug/buttplug-device-config/device-config-v3/buttplug-device-config-schema-v2.json
// https://github.com/buttplugio/buttplug/blob/master/buttplug/buttplug-device-config/device-config-v3/buttplug-device-config-schema-v3.json

namespace QtButtplug
{
  // message types
  [[maybe_unused]] const char* const MessageTypeOk = "OK";
  [[maybe_unused]] const char* const MessageTypeError = "Error";
  [[maybe_unused]] const char* const MessageTypePing = "Ping";
  [[maybe_unused]] const char* const MessageTypeRequestServerInfo = "RequestServerInfo";
  [[maybe_unused]] const char* const MessageTypeServerInfo = "ServerInfo";
  [[maybe_unused]] const char* const MessageTypeStartScanning = "StartScanning";
  [[maybe_unused]] const char* const MessageTypeStopScanning = "StopScanning";
  [[maybe_unused]] const char* const MessageTypeScanningFinished = "ScanningFinished";
  [[maybe_unused]] const char* const MessageTypeRequestDeviceList = "RequestDeviceList";
  [[maybe_unused]] const char* const MessageTypeDeviceList = "DeviceList";
  [[maybe_unused]] const char* const MessageTypeDeviceAdded = "DeviceAdded";
  [[maybe_unused]] const char* const MessageTypeDeviceRemoved = "DeviceRemoved";
  [[maybe_unused]] const char* const MessageTypeStopDeviceCmd = "StopDeviceCmd";
  [[maybe_unused]] const char* const MessageTypeStopAllDevices = "StopAllDevices";
  [[maybe_unused]] const char* const MessageTypeScalarCmd = "ScalarCmd";
  [[maybe_unused]] const char* const MessageTypeVibrateCmd = "VibrateCmd";
  [[maybe_unused]] const char* const MessageTypeSingleMotorVibrateCmd = "SingleMotorVibrateCmd";
  [[maybe_unused]] const char* const MessageTypeLinearCmd = "LinearCmd";
  [[maybe_unused]] const char* const MessageTypeRotateCmd = "RotateCmd";
  [[maybe_unused]] const char* const MessageTypeSensorReadCmd = "SensorReadCmd";
  [[maybe_unused]] const char* const MessageTypeSensorReading = "SensorReading";
  [[maybe_unused]] const char* const MessageTypeSensorSubscribeCmd = "SensorSubscribeCmd";
  [[maybe_unused]] const char* const MessageTypeSensorUnsubscribeCmd = "SensorUnsubscribeCmd";
  [[maybe_unused]] const char* const MessageTypeBatteryLevelCmd = "BatteryLevelCmd";
  [[maybe_unused]] const char* const MessageTypeBatteryLevelReading = "BatteryLevelReading";
  [[maybe_unused]] const char* const MessageTypeRSSILevelCmd = "RSSILevelCmd";
  [[maybe_unused]] const char* const MessageTypeRSSILevelReading = "RSSILevelReading";
  [[maybe_unused]] const char* const MessageTypeRawWriteCmd = "RawWriteCmd";
  [[maybe_unused]] const char* const MessageTypeRawReadCmd = "RawReadCmd";
  [[maybe_unused]] const char* const MessageTypeRawReading = "RawReading";
  [[maybe_unused]] const char* const MessageTypeRawSubscribeCmd = "RawSubscribeCmd";
  [[maybe_unused]] const char* const MessageTypeRawUnsubscribeCmd = "RawUnsubscribeCmd";
  [[maybe_unused]] const char* const MessageTypeKiirooCmd = "KiirooCmd";
  [[maybe_unused]] const char* const MessageTypeFleshlightLaunchFW12Cmd = "FleshlightLaunchFW12Cmd";
  [[maybe_unused]] const char* const MessageTypeLovenseCmd = "LovenseCmd";
  [[maybe_unused]] const char* const MessageTypeVorzeA10CycloneCmd = "VorzeA10CycloneCmd";

  namespace V3
  {
    [[maybe_unused]] const char* const DeviceMessageTypeStopDevice = "StopDeviceCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeScalar = "ScalarCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeLinear = "LinearCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRotate = "RotateCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeSensorRead = "SensorReadCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeSensorSubscribe = "SensorSubscribeCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRawRead = "RawReadCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRawWrite = "RawWriteCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRawSubscribe = "RawSubscribeCmd";

    [[maybe_unused]] const char* const DeviceEndPointCommand = "command";
    [[maybe_unused]] const char* const DeviceEndpointFirmware = "firmware";
    [[maybe_unused]] const char* const DeviceEndpointRx = "rx";
    [[maybe_unused]] const char* const DeviceEndpointRxaccel = "rxaccel";
    [[maybe_unused]] const char* const DeviceEndpointRxblebattery = "rxblebattery";
    [[maybe_unused]] const char* const DeviceEndpointRxblemodel = "rxblemodel";
    [[maybe_unused]] const char* const DeviceEndpointRxpressure = "rxpressure";
    [[maybe_unused]] const char* const DeviceEndpointRxtouch = "rxtouch";
    [[maybe_unused]] const char* const DeviceEndpointTx = "tx";
    [[maybe_unused]] const char* const DeviceEndpointTxmode = "txmode";
    [[maybe_unused]] const char* const DeviceEndpointTxshock = "txshock";
    [[maybe_unused]] const char* const DeviceEndpointTxvibrate = "txvibrate";
    [[maybe_unused]] const char* const DeviceEndpointTxvendorcontrol = "txvendorcontrol";
    [[maybe_unused]] const char* const DeviceEndpointWhitelist = "whitelist";
    [[maybe_unused]] const char* const DeviceEndpointGeneric = "generic"; // [1-2]?[0-9]
    [[maybe_unused]] const char* const DeviceEndpointGeneric3 = "generic3"; // [0-1]

    [[maybe_unused]] const char* const DeviceFeatureVibrate = "Vibrate";
    [[maybe_unused]] const char* const DeviceFeatureRotate = "Rotate";
    [[maybe_unused]] const char* const DeviceFeatureOscillate = "Oscillate";
    [[maybe_unused]] const char* const DeviceFeatureConstrict = "Constrict";
    [[maybe_unused]] const char* const DeviceFeatureInflate = "Inflate";
    [[maybe_unused]] const char* const DeviceFeaturePosition = "Position";
    [[maybe_unused]] const char* const DeviceFeatureBattery = "Battery";
    [[maybe_unused]] const char* const DeviceFeatureRSSI = "RSSI";
    [[maybe_unused]] const char* const DeviceFeaturePressure = "Pressure";
  }

  namespace V2
  {
    [[maybe_unused]] const char* const DeviceMessageTypeStopDevice = "StopDeviceCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeVibrate = "VibrateCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeLinear = "LinearCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRotate = "RotateCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeBatteryLevel = "BatteryLevelCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRSSILevel = "RSSILevelCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRawRead = "RawReadCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRawWrite = "RawWriteCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRawSubscribe = "RawSubscribeCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRawUnsubscribe = "RawUnsubscribeCmd";

    [[maybe_unused]] const char* const DeviceEndPointCommand = "command";
    [[maybe_unused]] const char* const DeviceEndpointFirmware = "firmware";
    [[maybe_unused]] const char* const DeviceEndpointRx = "rx";
    [[maybe_unused]] const char* const DeviceEndpointRxaccel = "rxaccel";
    [[maybe_unused]] const char* const DeviceEndpointRxblebattery = "rxblebattery";
    [[maybe_unused]] const char* const DeviceEndpointRxblemodel = "rxblemodel";
    [[maybe_unused]] const char* const DeviceEndpointRxpressure = "rxpressure";
    [[maybe_unused]] const char* const DeviceEndpointRxtouch = "rxtouch";
    [[maybe_unused]] const char* const DeviceEndpointTx = "tx";
    [[maybe_unused]] const char* const DeviceEndpointTxmode = "txmode";
    [[maybe_unused]] const char* const DeviceEndpointTxshock = "txshock";
    [[maybe_unused]] const char* const DeviceEndpointTxvibrate = "txvibrate";
    [[maybe_unused]] const char* const DeviceEndpointTxvendorcontrol = "txvendorcontrol";
    [[maybe_unused]] const char* const DeviceEndpointWhitelist = "whitelist";
    [[maybe_unused]] const char* const DeviceEndpointGeneric = "generic"; // [1-2]?[0-9]
    [[maybe_unused]] const char* const DeviceEndpointGeneric3 = "generic3"; // [0-1]

    [[maybe_unused]] const char* const DeviceAccuatorVibrate = "Vibrate";
    [[maybe_unused]] const char* const DeviceAccuatorRotate = "Rotate";
    [[maybe_unused]] const char* const DeviceAccuatorOscillate = "Oscillate";
    [[maybe_unused]] const char* const DeviceAccuatorConstrict = "Constrict";
    [[maybe_unused]] const char* const DeviceAccuatorInflate = "Inflate";
    [[maybe_unused]] const char* const DeviceAccuatorPosition = "Position";

    [[maybe_unused]] const char* const DeviceSensorBattery = "Battery";
    [[maybe_unused]] const char* const DeviceSensorPressure = "Pressure";
  }

  namespace V1
  {
    [[maybe_unused]] const char* const DeviceMessageTypeStopDevice = "StopDeviceCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeVibrate = "VibrateCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeLinear = "LinearCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeRotate = "RotateCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeLovense = "LovenseCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeVorzeA10Cyclone = "VorzeA10CycloneCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeKiiroo = "KiirooCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeSingleMotorVibrate = "SingleMotorVibrateCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeFleshlightLaunchFW12 = "FleshlightLaunchFW12Cmd";
  }

  namespace V0
  {
    [[maybe_unused]] const char* const DeviceMessageTypeFleshlightLaunchFW12 = "FleshlightLaunchFW12Cmd";
    [[maybe_unused]] const char* const DeviceMessageTypeSingleMotorVibrate = "SingleMotorVibrateCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeKiiroo = "KiirooCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeLovense = "LovenseCmd";
    [[maybe_unused]] const char* const DeviceMessageTypeVorzeA10Cyclone = "VorzeA10CycloneCmd";
  }

  struct MessageBase
  {
    virtual ~MessageBase() {}
    qint64 Id = -1;
    QString MessageType;
  };

  struct Ok : MessageBase
  {
    Ok() :  MessageBase() { MessageType = MessageTypeOk; }
    ~Ok() override {}
  };

  struct ErrorMsg : MessageBase
  {
    ErrorMsg() :  MessageBase() { MessageType = MessageTypeError; }
    ~ErrorMsg() override {}
    QString ErrorMessage;
    Error ErrorCode = ERROR_OK;
  };

  struct Ping : MessageBase
  {
    Ping() :  MessageBase() { MessageType = MessageTypePing; }
    ~Ping() override {}
  };

  struct RequestServerInfo : MessageBase
  {
    RequestServerInfo() :  MessageBase() { MessageType = MessageTypeRequestServerInfo; }
    ~RequestServerInfo() override {}
    QString ClientName;
    // ProtocolV1 and up only
    ButtplugProtocolVersion MessageVersion = ProtocolV3;
  };

  struct ServerInfo : MessageBase
  {
    ServerInfo() :  MessageBase() { MessageType = MessageTypeServerInfo; }
    ~ServerInfo() override {}
    QString ServerName;
    ButtplugProtocolVersion MessageVersion = ProtocolV3;
    qint64 MaxPingTime = 10;
    // ProtocolV0
    qint64 MajorVersion;
    // ProtocolV0
    qint64 MinorVersion;
    // ProtocolV0
    qint64 BuildVersion;
  };

  struct StartScanning : MessageBase
  {
    StartScanning() :  MessageBase() { MessageType = MessageTypeStartScanning; }
    ~StartScanning() override {}
  };

  struct StopScanning : MessageBase
  {
    StopScanning() :  MessageBase() { MessageType = MessageTypeStopScanning; }
    ~StopScanning() override {}
  };

  struct ScanningFinished : MessageBase
  {
    ScanningFinished() :  MessageBase() { MessageType = MessageTypeScanningFinished; }
    ~ScanningFinished() override {}
  };

  struct RequestDeviceList : MessageBase
  {
    RequestDeviceList() :  MessageBase() { MessageType = MessageTypeRequestDeviceList; }
    ~RequestDeviceList() override {}
  };

  struct ClientDeviceMessageAttribute
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

  struct Device
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
    QMap<QString, QList<ClientDeviceMessageAttribute>> DeviceMessages;
  };

  struct DeviceList : MessageBase
  {
    DeviceList() :  MessageBase() { MessageType = MessageTypeDeviceList; }
    ~DeviceList() override {}
    QList<Device> Devices;
  };

  struct DeviceAdded : MessageBase, Device
  {
    DeviceAdded() :  MessageBase() { MessageType = MessageTypeDeviceAdded; }
    ~DeviceAdded() override {}
  };

  struct DeviceRemoved : MessageBase
  {
    DeviceRemoved() :  MessageBase() { MessageType = MessageTypeDeviceRemoved; }
    ~DeviceRemoved() override {}
    qint64 DeviceIndex = -1;
  };

  struct StopDeviceCmd : MessageBase
  {
    StopDeviceCmd() :  MessageBase() { MessageType = MessageTypeStopDeviceCmd; }
    ~StopDeviceCmd() override {}
    qint64 DeviceIndex = -1;
  };

  struct StopAllDevices : MessageBase
  {
    StopAllDevices() :  MessageBase() { MessageType = MessageTypeStopAllDevices; }
    ~StopAllDevices() override {}
  };

  // ProtocolV3
  struct ScalarCmdElem
  {
    qint64 Index = -1;
    double Scalar = 0.0; // range of [0.0-1.0]
    QString ActuatorType;
  };

  // ProtocolV3
  struct ScalarCmd : MessageBase
  {
    ScalarCmd() :  MessageBase() { MessageType = MessageTypeScalarCmd; }
    ~ScalarCmd() override {}
    qint64 DeviceIndex = -1;
    QList<ScalarCmdElem> Scalars;
  };

  // ProtocolV1, ProtocolV2
  struct VibrateCmdElem
  {
    qint64 Index = -1;
    double Speed = 0.0; // range of [0.0-1.0]
  };

  // ProtocolV1, ProtocolV2
  struct VibrateCmd : MessageBase
  {
    VibrateCmd() :  MessageBase() { MessageType = MessageTypeVibrateCmd; }
    ~VibrateCmd() override {}
    qint64 DeviceIndex = -1;
    QList<VibrateCmdElem> Speeds;
  };

  // ProtocolV0
  struct SingleMotorVibrateCmd : MessageBase
  {
    SingleMotorVibrateCmd() :  MessageBase() { MessageType = MessageTypeSingleMotorVibrateCmd; }
    ~SingleMotorVibrateCmd() override {}
    qint64 DeviceIndex = -1;
    double Speed = 0.0; // Vibration speed with a range of [0.0-1.0]
  };

  // ProtocolV1, ProtocolV2, ProtocolV3
  struct LinearCmdElem
  {
    qint64 Index = -1;
    qint64 Duration = 0; // ms
    double Position = 0.0; // range of [0.0-1.0]
  };

  struct LinearCmd : MessageBase
  {
    LinearCmd() :  MessageBase() { MessageType = MessageTypeLinearCmd; }
    ~LinearCmd() override {}
    qint64 DeviceIndex = -1;
    QList<LinearCmdElem> Vectors;
  };

  // ProtocolV1, ProtocolV2, ProtocolV3
  struct RotateCmdElem
  {
    qint64 Index = -1;
    double Speed = 0.0; // range of [0.0-1.0]
    bool Clockwise = true;
  };

  // ProtocolV1, ProtocolV2, ProtocolV3
  struct RotateCmd : MessageBase
  {
    RotateCmd() :  MessageBase() { MessageType = MessageTypeRotateCmd; }
    ~RotateCmd() override {}
    qint64 DeviceIndex = -1;
    QList<RotateCmdElem> Rotations;
  };

  struct SensorCommandBase : MessageBase
  {
    ~SensorCommandBase() override {}
    qint64 DeviceIndex = -1;
    qint64 SensorIndex = -1;
    QString SensorType;
  };

  // ProtocolV3
  struct SensorReadCmd : SensorCommandBase
  {
    SensorReadCmd() :  SensorCommandBase() { MessageType = MessageTypeSensorReadCmd; }
    ~SensorReadCmd() override {}
  };

  // ProtocolV3
  struct SensorReading : SensorCommandBase
  {
    SensorReading() :  SensorCommandBase() { MessageType = MessageTypeSensorReading; }
    ~SensorReading() override {}
    // Signed integers are used due to varying return values (for instance, RSSI is negative,
    // battery is [0, 100], buttons are [0, 1], etc...).
    // Information on formatting/units of measurement/etc may be included in feature descriptors.
    QList<qint32> Data;
  };

  // ProtocolV3
  struct SensorSubscribeCmd : SensorCommandBase
  {
    SensorSubscribeCmd() :  SensorCommandBase() { MessageType = MessageTypeSensorSubscribeCmd; }
    ~SensorSubscribeCmd() override {}
  };

  // ProtocolV3
  struct SensorUnsubscribeCmd : SensorCommandBase
  {
    SensorUnsubscribeCmd() :  SensorCommandBase() { MessageType = MessageTypeSensorUnsubscribeCmd; }
    ~SensorUnsubscribeCmd() override {}
  };

  // ProtocolV2
  struct BatteryLevelCmd : MessageBase
  {
    BatteryLevelCmd() :  MessageBase() { MessageType = MessageTypeBatteryLevelCmd; }
    ~BatteryLevelCmd() override {}
    qint64 DeviceIndex = -1;
  };

  // ProtocolV2
  struct BatteryLevelReading : MessageBase
  {
    BatteryLevelReading() :  MessageBase() { MessageType = MessageTypeBatteryLevelReading; }
    ~BatteryLevelReading() override {}
    qint64 DeviceIndex = -1;
    double BatteryLevel = 0.0;
  };

  // ProtocolV2
  struct RSSILevelCmd : MessageBase
  {
    RSSILevelCmd() :  MessageBase() { MessageType = MessageTypeRSSILevelCmd; }
    ~RSSILevelCmd() override {}
    qint64 DeviceIndex = -1;
  };

  // ProtocolV2
  struct RSSILevelReading : MessageBase
  {
    RSSILevelReading() :  MessageBase() { MessageType = MessageTypeRSSILevelReading; }
    ~RSSILevelReading() override {}
    qint64 DeviceIndex = -1;
    qint32 RSSILevel = 0; // RSSI Level, usually expressed as db gain, usually [-100:0]
  };

  // ProtocolV2, ProtocolV3
  struct RawWriteCmd : MessageBase
  {
    RawWriteCmd() :  MessageBase() { MessageType = MessageTypeRawWriteCmd; }
    ~RawWriteCmd() override {}
    qint64 DeviceIndex = -1;
    QString Endpoint;
    QByteArray Data;
    bool WriteWithResponse = true;
  };

  // ProtocolV2, ProtocolV3
  struct RawReadCmd : MessageBase
  {
    RawReadCmd() :  MessageBase() { MessageType = MessageTypeRawReadCmd; }
    ~RawReadCmd() override {}
    qint64 DeviceIndex = -1;
    QString Endpoint;
    qint64 ExpectedLength = 0;
    bool WaitForData = true;
  };

  // ProtocolV2, ProtocolV3
  struct RawReading : MessageBase
  {
    RawReading() :  MessageBase() { MessageType = MessageTypeRawReading; }
    ~RawReading() override {}
    qint64 DeviceIndex = -1;
    QString Endpoint;
    QByteArray Data;
  };

  // ProtocolV2, ProtocolV3
  struct RawSubscribeCmd : MessageBase
  {
    RawSubscribeCmd() :  MessageBase() { MessageType = MessageTypeRawSubscribeCmd; }
    ~RawSubscribeCmd() override {}
    qint64 DeviceIndex = -1;
    QString Endpoint;
  };

  // ProtocolV2, ProtocolV3
  struct RawUnsubscribeCmd : MessageBase
  {
    RawUnsubscribeCmd() :  MessageBase() { MessageType = MessageTypeRawUnsubscribeCmd; }
    ~RawUnsubscribeCmd() override {}
    qint64 DeviceIndex = -1;
    QString Endpoint;
  };

  // ProtocolV0
  struct KiirooCmd : MessageBase
  {
    KiirooCmd() :  MessageBase() { MessageType = MessageTypeKiirooCmd; }
    ~KiirooCmd() override {}
    qint64 DeviceIndex = -1;
    QString Command;
  };

  // ProtocolV0
  struct FleshlightLaunchFW12Cmd : MessageBase
  {
    FleshlightLaunchFW12Cmd() :  MessageBase() { MessageType = MessageTypeFleshlightLaunchFW12Cmd; }
    ~FleshlightLaunchFW12Cmd() override {}
    qint64 DeviceIndex = -1;
    qint64 Position;
    qint64 Speed;
  };

  // ProtocolV0
  struct LovenseCmd : MessageBase
  {
    LovenseCmd() :  MessageBase() { MessageType = MessageTypeLovenseCmd; }
    ~LovenseCmd() override {}
    qint64 DeviceIndex = -1;
    QString Command;
  };

  // ProtocolV0
  struct VorzeA10CycloneCmd : MessageBase
  {
    VorzeA10CycloneCmd() :  MessageBase() { MessageType = MessageTypeVorzeA10CycloneCmd; }
    ~VorzeA10CycloneCmd() override {}
    qint64 DeviceIndex = -1;
    qint64 Speed;
    bool Clockwise = true;
  };
}

#endif //QBUTTPLUGMESSAGES_H
