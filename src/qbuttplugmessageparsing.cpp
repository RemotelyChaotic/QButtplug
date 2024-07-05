#include "qbuttplugmessageparsing.h"

#include <QGlobalStatic>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>

#include <set>

namespace detail
{
  struct QButtplugMessageParser
  {
    std::function<ButtplugMessageBase*(const QJsonObject&, QtButtplug::ButtplugProtocolVersion)> m_fnFromJson;
    std::function<QJsonObject(ButtplugMessageBase*, QtButtplug::ButtplugProtocolVersion)> m_fnToJson;
  };

  //--------------------------------------------------------------------------------------
  void ParseDevice(QJsonObject devObj, QtButtplug::ButtplugProtocolVersion v,
                   ButtplugDevice& dev)
  {
    auto itDev = devObj.find("DeviceName");
    if (itDev != devObj.end())
    {
      dev.DeviceName = itDev->toString();
    }
    itDev = devObj.find("DeviceIndex");
    if (itDev != devObj.end())
    {
      dev.DeviceIndex = itDev->toInt();
    }
    if (QtButtplug::ProtocolV1 < v)
    {
      itDev = devObj.find("DeviceDisplayName");
      if (itDev != devObj.end())
      {
        dev.DeviceDisplayName = itDev->toString();
      }
      itDev = devObj.find("DeviceMessageTimingGap");
      if (itDev != devObj.end())
      {
        dev.DeviceMessageTimingGap = itDev->toInt();
      }
    }
    else
    {
      dev.DeviceDisplayName = dev.DeviceName;
      dev.DeviceMessageTimingGap = 10;
    }
    if (QtButtplug::ProtocolV0 == v) {
      itDev = devObj.find("DeviceMessages");
      if (itDev != devObj.end())
      {
        QJsonArray arrMsg = itDev->toArray();
        for (qint32 j = 0; arrMsg.count() > j; ++j) {
          if (arrMsg.at(j).isString()) {
            ButtplugClientDeviceMessageAttribute attr;
            attr.MessageType = arrMsg.at(j).toString();
            dev.DeviceMessages.insert(attr.MessageType, {attr});
          }
        }
      }
    } else if (QtButtplug::ProtocolV1 == v  || QtButtplug::ProtocolV2 == v) {
      itDev = devObj.find("DeviceMessages");
      if (itDev != devObj.end() && itDev->isObject())
      {
        auto msgs = itDev->toObject();
        for (auto itmsg = msgs.begin(); msgs.end() != itmsg; ++itmsg) {
          if (itmsg->isObject()) {
            ButtplugClientDeviceMessageAttribute attr;
            attr.MessageType = itmsg.key();
            auto itAttrS = itmsg->toObject().find("StepCount");
            auto itAttrF = itmsg->toObject().find("FeatureCount");
            if (itmsg->toObject().end() != itAttrF) {
              qint32 iCount = itAttrF->toInt();
              QJsonArray stepCount;
              if (itmsg->toObject().end() != itAttrS && itAttrS->isArray()) {
                stepCount = itAttrS->toArray();
              }
              for (qint32 j = 0; iCount > j; ++j) {
                auto newMsg = ButtplugClientDeviceMessageAttribute(attr);
                if (QtButtplug::ProtocolV2 == v && stepCount.count() > j) {
                  newMsg.StepCount = stepCount.at(j).toInt();
                }
                dev.DeviceMessages[attr.MessageType].push_back(newMsg);
              }
            }
          }
        }
      }
    } else {
      itDev = devObj.find("DeviceMessages");
      if (itDev != devObj.end() && itDev->isObject())
      {
        auto msgs = itDev->toObject();
        for (auto itmsg = msgs.begin(); msgs.end() != itmsg; ++itmsg) {
          if (itmsg->isObject()) {
            ButtplugClientDeviceMessageAttribute attr;
            attr.MessageType = itmsg.key();

            auto fnParseAttrs =  [&attr](QJsonObject o) -> ButtplugClientDeviceMessageAttribute {
              auto ret = ButtplugClientDeviceMessageAttribute(attr);
              auto itAttr = o.find("FeatureDescriptor");
              if (o.end() != itAttr)
              {
                attr.FeatureDescriptor = itAttr->toString();
              }
              itAttr = o.find("StepCount");
              if (o.end() != itAttr)
              {
                attr.StepCount = itAttr->toInt();
              }
              itAttr = o.find("ActuatorType");
              if (o.end() != itAttr)
              {
                attr.ActuatorType = itAttr->toString();
              }
              itAttr = o.find("SensorType");
              if (o.end() != itAttr)
              {
                attr.SensorType = itAttr->toString();
              }
              itAttr = o.find("SensorRange");
              if (o.end() != itAttr && itAttr->isArray())
              {
                auto attrArr = itAttr->toArray();
                for (auto itAtrArr = attrArr.begin(); attrArr.end() != itAtrArr; ++itAtrArr) {
                  if (itAtrArr->isArray()) {
                    auto itSensorRangeElem = itAtrArr->toArray();
                    if (itSensorRangeElem.count() == 2) {
                      QPair<qint32, qint32> minMax = { itSensorRangeElem.at(0).toInt(),
                                                       itSensorRangeElem.at(1).toInt()};
                      attr.SensorRange.push_back(minMax);
                    }
                  }
                }
              }
              itAttr = o.find("Endpoints");
              if (o.end() != itAttr && itAttr->isArray())
              {
                auto attrArr = itAttr->toArray();
                for (auto itAtrArr = attrArr.begin(); attrArr.end() != itAtrArr; ++itAtrArr) {
                  if (itAtrArr->isString()) {
                    attr.Endpoints << itAtrArr->toString();
                  }
                }
              }
              return ret;
            };

            if (itmsg.value().isArray()) {
              // Scalar etc.
              auto msgArr = itmsg.value().toArray();
              for (qint32 h = 0; msgArr.count() > h; ++h) {
                if (msgArr.at(h).isObject())
                  dev.DeviceMessages[attr.MessageType].push_back(
                    fnParseAttrs(msgArr.at(h).toObject()));
              }
            } else if (itmsg.value().isObject()) {
              dev.DeviceMessages[attr.MessageType].push_back(
                    fnParseAttrs(itmsg.value().toObject()));
            }
          }
        }
      }
    }
  }

  void SerializeDevice(const ButtplugDevice& dev, QtButtplug::ButtplugProtocolVersion v,
                       QJsonObject& o)
  {
    o["DeviceName"] = dev.DeviceName;
    o["DeviceIndex"] = dev.DeviceIndex;
    if (QtButtplug::ProtocolV0 == v) {
      QJsonArray arr;
      std::set<QString> set;
      for (auto it = dev.DeviceMessages.begin(); dev.DeviceMessages.end() != it; ++it) {
        set.insert(it.key());
      }
      for (const auto& s : set) {
        arr.push_back(s);
      }
      o["DeviceMessages"] = arr;
    } else if (QtButtplug::ProtocolV1 == v) {
      QJsonObject msgs;
      for (auto it = dev.DeviceMessages.begin(); dev.DeviceMessages.end() != it; ++it) {
        msgs[it.key()] = QJsonObject{ { "FeatureCount", it.value().size() } };
      }
      o["DeviceMessages"] = msgs;
    } else if (QtButtplug::ProtocolV2 == v) {
      QJsonObject msgs;
      for (auto it = dev.DeviceMessages.begin(); dev.DeviceMessages.end() != it; ++it) {
        QJsonArray arr;
        for (const ButtplugClientDeviceMessageAttribute& attr : it.value()) {
          arr.push_back(static_cast<qint32>(attr.StepCount));
        }
        msgs[it.key()] = QJsonObject{ { "FeatureCount", it.value().count() },
                                      { "StepCount", arr }};
      }
      o["DeviceMessages"] = msgs;
    } else {
      QJsonObject msgs;
      for (auto it = dev.DeviceMessages.begin(); dev.DeviceMessages.end() != it; ++it) {
        if (it.value().count() == 1 && it.value()[0].SensorType.isEmpty()) {
          msgs[it.key()] = QJsonObject();
        }
        else {
          QJsonArray arr;
          for (const ButtplugClientDeviceMessageAttribute& attr : it.value()) {
            QJsonObject o;
            if (!attr.FeatureDescriptor.isEmpty()) {
              o["FeatureDescriptor"] = attr.FeatureDescriptor;
            }
            if (attr.StepCount > 0) {
              o["StepCount"] = static_cast<qint32>(attr.StepCount);
            }
            if (!attr.ActuatorType.isEmpty()) {
              o["ActuatorType"] = attr.ActuatorType;
            }
            if (!attr.SensorType.isEmpty()) {
              o["SensorType"] = attr.SensorType;
            }
            if (!attr.SensorRange.isEmpty()) {
              QJsonArray arrSen;
              for (const auto& r : attr.SensorRange)
              {
                QJsonArray arrD;
                arrD.push_back(r.first);
                arrD.push_back(r.second);
                arrSen.push_back(arrD);
              }
              o["SensorRange"] = arrSen;
            }
            if (!attr.Endpoints.isEmpty()) {
              QJsonArray arrEnd;
              for (const auto& s : attr.Endpoints) arrEnd.push_back(s);
              o["Endpoints"] = arrEnd;
            }
            arr.push_back(o);
          }
          msgs[it.key()] = arr;
        }
      }
      o["DeviceMessages"] = msgs;
    }
  }

  //--------------------------------------------------------------------------------------
  struct QButtplugMessageParserRegistry
  {
    QButtplugMessageParserRegistry()
    {
      m_map = {
        //--------------------------------------------------------------------------------
        {
          "Ok",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugOk();
              ret->MessageType = "Ok";
              auto root = sIn.find("Ok");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugOk*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "Error",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugError();
              ret->MessageType = "Error";
              auto root = sIn.find("Error");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("ErrorMessage");
                if (it != objElem.end())
                {
                  ret->ErrorMessage = it->toString();
                }
                it = objElem.find("ErrorCode");
                if (it != objElem.end())
                {
                  ret->ErrorCode = QtButtplug::Error(it->toInt());
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugError*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"ErrorMessage", ms->ErrorMessage},
                    {"ErrorCode", static_cast<qint32>(ms->ErrorCode)},
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "Ping",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugPing();
              ret->MessageType = "Ping";
              auto root = sIn.find("Ping");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugPing*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RequestServerInfo",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion v) -> ButtplugMessageBase* {
              auto ret = new ButtplugRequestServerInfo();
              ret->MessageType = "RequestServerInfo";
              auto root = sIn.find("RequestServerInfo");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("ClientName");
                if (it != objElem.end())
                {
                  ret->ClientName = it->toString();
                }
                if (QtButtplug::ProtocolV0 < v)
                {
                  it = objElem.find("MessageVersion");
                  if (it != objElem.end())
                  {
                    ret->MessageVersion = QtButtplug::ButtplugProtocolVersion(it->toInt());
                  }
                }
                else
                {
                  ret->MessageVersion = QtButtplug::ProtocolV0;
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion v) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRequestServerInfo*>(pMsg);
              QJsonObject o1 = QJsonObject {
                {"Id", ms->Id},
                {"ClientName", ms->ClientName}
              };
              if (QtButtplug::ProtocolV0 < v)
              {
                o1["MessageVersion"] = static_cast<qint32>(ms->MessageVersion);
              }
              QJsonObject o = {
                {
                  ms->MessageType, o1
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "ServerInfo",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion v) -> ButtplugMessageBase* {
              auto ret = new ButtplugServerInfo();
              ret->MessageType = "ServerInfo";
              auto root = sIn.find("ServerInfo");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("ServerName");
                if (it != objElem.end())
                {
                  ret->ServerName = it->toString();
                }
                it = objElem.find("MessageVersion");
                if (it != objElem.end())
                {
                  ret->MessageVersion = QtButtplug::ButtplugProtocolVersion(it->toInt());
                }
                it = objElem.find("MaxPingTime");
                if (it != objElem.end())
                {
                  ret->MaxPingTime = it->toInt();
                }
                ret->MajorVersion = -1;
                ret->MinorVersion = -1;
                ret->BuildVersion = -1;
                if (QtButtplug::ProtocolV0 == v)
                {
                  it = objElem.find("MajorVersion");
                  if (it != objElem.end())
                  {
                    ret->MajorVersion = it->toInt();
                  }
                  it = objElem.find("MinorVersion");
                  if (it != objElem.end())
                  {
                    ret->MinorVersion = it->toInt();
                  }
                  it = objElem.find("BuildVersion");
                  if (it != objElem.end())
                  {
                    ret->BuildVersion = it->toInt();
                  }
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion v) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugServerInfo*>(pMsg);
              QJsonObject o1 = QJsonObject {
                {"Id", ms->Id},
                {"ServerName", ms->ServerName},
                {"MessageVersion", static_cast<qint32>(ms->MessageVersion)},
                {"MaxPingTime", ms->MaxPingTime}
              };
              if (QtButtplug::ProtocolV0 == v) {
                o1["MajorVersion"] = static_cast<qint32>(ms->MajorVersion);
                o1["MinorVersion"] = static_cast<qint32>(ms->MinorVersion);
                o1["BuildVersion"] = static_cast<qint32>(ms->BuildVersion);
              }
              QJsonObject o = {
                {
                  ms->MessageType, o1
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "StartScanning",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugStartScanning();
              ret->MessageType = "StartScanning";
              auto root = sIn.find("StartScanning");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugStartScanning*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "StopScanning",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugStopScanning();
              ret->MessageType = "StopScanning";
              auto root = sIn.find("StopScanning");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugStopScanning*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "ScanningFinished",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugScanningFinished();
              ret->MessageType = "ScanningFinished";
              auto root = sIn.find("ScanningFinished");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugScanningFinished*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RequestDeviceList",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRequestDeviceList();
              ret->MessageType = "RequestDeviceList";
              auto root = sIn.find("RequestDeviceList");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRequestDeviceList*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "DeviceList",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion v) -> ButtplugMessageBase* {
              auto ret = new ButtplugDeviceList();
              ret->MessageType = "DeviceList";
              auto root = sIn.find("DeviceList");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("Devices");
                if (it != objElem.end() && it->isArray())
                {
                  QJsonArray arr = it->toArray();
                  for (qint32 i = 0; arr.count() > i; ++i) {
                    if (arr.at(i).isObject()) {
                      ButtplugDevice dev;
                      auto devObj = arr.at(i).toObject();
                      ParseDevice(devObj, v, dev);
                      ret->Devices.push_back(dev);
                    }
                  }
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion v) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugDeviceList*>(pMsg);
              QJsonArray devices;
              for (const auto& dev : qAsConst(ms->Devices)) {
                QJsonObject oDev;
                SerializeDevice(dev, v, oDev);
                devices.append(oDev);
              }
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"Devices", devices}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "DeviceAdded",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion v) -> ButtplugMessageBase* {
              auto ret = new ButtplugDeviceAdded();
              ret->MessageType = "DeviceAdded";
              auto root = sIn.find("DeviceAdded");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                ParseDevice(objElem, v, *ret);
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion v) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugDeviceAdded*>(pMsg);
              QJsonObject oDev = {
                {"Id", ms->Id}
              };
              SerializeDevice(*ms, v, oDev);
              QJsonObject o = {
                {
                  {ms->MessageType, oDev}
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "DeviceRemoved",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugDeviceRemoved();
              ret->MessageType = "DeviceRemoved";
              auto root = sIn.find("DeviceRemoved");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugDeviceRemoved*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "StopDeviceCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugStopDeviceCmd();
              ret->MessageType = "StopDeviceCmd";
              auto root = sIn.find("StopDeviceCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugStopDeviceCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "StopAllDevices",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugStopAllDevices();
              ret->MessageType = "StopAllDevices";
              auto root = sIn.find("StopAllDevices");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugStopAllDevices*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "ScalarCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugScalarCmd();
              ret->MessageType = "ScalarCmd";
              auto root = sIn.find("ScalarCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Scalars");
                if (it != objElem.end() && it->isArray())
                {
                  QJsonArray arrScalars = it->toArray();
                  for (auto itScal = arrScalars.begin(); arrScalars.end() != itScal; itScal++) {
                    if (itScal->isObject()) {
                      auto objScal = itScal->toObject();
                      ButtplugScalarCmdElem elem;
                      auto itScal = objScal.find("Index");
                      if (itScal != objElem.end())
                      {
                        elem.Index = itScal->toInt();
                      }
                      itScal = objScal.find("Scalar");
                      if (itScal != objElem.end())
                      {
                        elem.Scalar = itScal->toDouble();
                      }
                      itScal = objScal.find("ActuatorType");
                      if (itScal != objElem.end())
                      {
                        elem.ActuatorType = itScal->toString();
                      }
                      ret->Scalars.push_back(elem);
                    }
                  }
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugScalarCmd*>(pMsg);
              QJsonArray scals;
              for (const auto& scal : qAsConst(ms->Scalars)) {
                scals.push_back(QJsonObject{
                  {"Index", scal.Index},
                  {"Scalar", scal.Scalar},
                  {"ActuatorType", scal.ActuatorType}
                });
              }
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Scalars", scals}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "VibrateCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugVibrateCmd();
              ret->MessageType = "VibrateCmd";
              auto root = sIn.find("VibrateCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Speeds");
                if (it != objElem.end() && it->isArray())
                {
                  QJsonArray arr = it->toArray();
                  for (auto itArr = arr.begin(); arr.end() != itArr; itArr++) {
                    if (itArr->isObject()) {
                      auto objArr = itArr->toObject();
                      ButtplugVibrateCmdElem elem;
                      auto itArr = objArr.find("Index");
                      if (itArr != objElem.end())
                      {
                        elem.Index = itArr->toInt();
                      }
                      itArr = objArr.find("Speed");
                      if (itArr != objElem.end())
                      {
                        elem.Speed = itArr->toDouble();
                      }
                      ret->Speeds.push_back(elem);
                    }
                  }
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugVibrateCmd*>(pMsg);
              QJsonArray scals;
              for (const auto& scal : qAsConst(ms->Speeds)) {
                scals.push_back(QJsonObject{
                  {"Index", scal.Index},
                  {"Speed", scal.Speed}
                });
              }
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Speeds", scals}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "SingleMotorVibrateCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugSingleMotorVibrateCmd();
              ret->MessageType = "SingleMotorVibrateCmd";
              auto root = sIn.find("SingleMotorVibrateCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Speed");
                if (it != objElem.end())
                {
                  ret->Speed = it->toDouble();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugSingleMotorVibrateCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Speed", ms->Speed}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "LinearCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugLinearCmd();
              ret->MessageType = "LinearCmd";
              auto root = sIn.find("LinearCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Vectors");
                if (it != objElem.end() && it->isArray())
                {
                  QJsonArray arr = it->toArray();
                  for (auto itArr = arr.begin(); arr.end() != itArr; itArr++) {
                    if (itArr->isObject()) {
                      auto objArr = itArr->toObject();
                      ButtplugLinearCmdElem elem;
                      auto itArr = objArr.find("Index");
                      if (itArr != objElem.end())
                      {
                        elem.Index = itArr->toInt();
                      }
                      itArr = objArr.find("Duration");
                      if (itArr != objElem.end())
                      {
                        elem.Duration = static_cast<qint64>(itArr->toDouble());
                      }
                      itArr = objArr.find("Position");
                      if (itArr != objElem.end())
                      {
                        elem.Position = itArr->toDouble();
                      }
                      ret->Vectors.push_back(elem);
                    }
                  }
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugLinearCmd*>(pMsg);
              QJsonArray scals;
              for (const auto& scal : qAsConst(ms->Vectors)) {
                scals.push_back(QJsonObject{
                  {"Index", scal.Index},
                  {"Duration", scal.Duration},
                  {"Position", scal.Position}
                });
              }
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Vectors", scals}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RotateCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRotateCmd();
              ret->MessageType = "RotateCmd";
              auto root = sIn.find("RotateCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Rotations");
                if (it != objElem.end() && it->isArray())
                {
                  QJsonArray arr = it->toArray();
                  for (auto itArr = arr.begin(); arr.end() != itArr; itArr++) {
                    if (itArr->isObject()) {
                      auto objArr = itArr->toObject();
                      ButtplugRotateCmdElem elem;
                      auto itArr = objArr.find("Index");
                      if (itArr != objElem.end())
                      {
                        elem.Index = itArr->toInt();
                      }
                      itArr = objArr.find("Speed");
                      if (itArr != objElem.end())
                      {
                        elem.Speed = itArr->toDouble();
                      }
                      itArr = objArr.find("Clockwise");
                      if (itArr != objElem.end())
                      {
                        elem.Clockwise = itArr->toBool();
                      }
                      ret->Rotations.push_back(elem);
                    }
                  }
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRotateCmd*>(pMsg);
              QJsonArray scals;
              for (const auto& scal : qAsConst(ms->Rotations)) {
                scals.push_back(QJsonObject{
                  {"Index", scal.Index},
                  {"Speed", scal.Speed},
                  {"Clockwise", scal.Clockwise}
                });
              }
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Rotations", scals}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "SensorReadCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugSensorReadCmd();
              ret->MessageType = "SensorReadCmd";
              auto root = sIn.find("SensorReadCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("SensorIndex");
                if (it != objElem.end())
                {
                  ret->SensorIndex = it->toInt();
                }
                it = objElem.find("SensorType");
                if (it != objElem.end())
                {
                  ret->SensorType = it->toString();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugSensorReadCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"SensorIndex", ms->SensorIndex},
                    {"SensorType", ms->SensorType}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "SensorReading",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugSensorReading();
              ret->MessageType = "SensorReading";
              auto root = sIn.find("SensorReading");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("SensorIndex");
                if (it != objElem.end())
                {
                  ret->SensorIndex = it->toInt();
                }
                it = objElem.find("SensorType");
                if (it != objElem.end())
                {
                  ret->SensorType = it->toString();
                }
                it = objElem.find("Data");
                if (it != objElem.end() && it->isArray())
                {
                  QJsonArray dataArr = it->toArray();
                  for (auto itDat = dataArr.begin(); dataArr.end() != itDat; ++itDat) {
                    ret->Data.push_back(itDat->toInt());
                  }
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugSensorReading*>(pMsg);
              QJsonArray dataArr;
              for (qint32 d : qAsConst(ms->Data)) {
                dataArr.push_back(d);
              }
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"SensorIndex", ms->SensorIndex},
                    {"SensorType", ms->SensorType},
                    {"Data", dataArr}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "SensorSubscribeCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugSensorSubscribeCmd();
              ret->MessageType = "SensorSubscribeCmd";
              auto root = sIn.find("SensorSubscribeCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("SensorIndex");
                if (it != objElem.end())
                {
                  ret->SensorIndex = it->toInt();
                }
                it = objElem.find("SensorType");
                if (it != objElem.end())
                {
                  ret->SensorType = it->toString();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugSensorSubscribeCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"SensorIndex", ms->SensorIndex},
                    {"SensorType", ms->SensorType}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "SensorUnsubscribeCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugSensorUnsubscribeCmd();
              ret->MessageType = "SensorUnsubscribeCmd";
              auto root = sIn.find("SensorUnsubscribeCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("SensorIndex");
                if (it != objElem.end())
                {
                  ret->SensorIndex = it->toInt();
                }
                it = objElem.find("SensorType");
                if (it != objElem.end())
                {
                  ret->SensorType = it->toString();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugSensorUnsubscribeCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"SensorIndex", ms->SensorIndex},
                    {"SensorType", ms->SensorType}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "BatteryLevelCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugBatteryLevelCmd();
              ret->MessageType = "BatteryLevelCmd";
              auto root = sIn.find("BatteryLevelCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugBatteryLevelCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "BatteryLevelReading",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugBatteryLevelReading();
              ret->MessageType = "BatteryLevelReading";
              auto root = sIn.find("BatteryLevelReading");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("BatteryLevel");
                if (it != objElem.end())
                {
                  ret->BatteryLevel = it->toDouble();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugBatteryLevelReading*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"BatteryLevel", ms->BatteryLevel}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RSSILevelCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRSSILevelCmd();
              ret->MessageType = "RSSILevelCmd";
              auto root = sIn.find("RSSILevelCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRSSILevelCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RSSILevelReading",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRSSILevelReading();
              ret->MessageType = "RSSILevelReading";
              auto root = sIn.find("RSSILevelReading");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("RSSILevel");
                if (it != objElem.end())
                {
                  ret->RSSILevel = it->toInt();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRSSILevelReading*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"RSSILevel", ms->RSSILevel}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RawWriteCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRawWriteCmd();
              ret->MessageType = "RawWriteCmd";
              auto root = sIn.find("RawWriteCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Endpoint");
                if (it != objElem.end())
                {
                  ret->Endpoint = it->toString();
                }
                it = objElem.find("Data");
                if (it != objElem.end() && it->isArray())
                {
                  QJsonArray datArr = it->toArray();
                  for (auto itDat = datArr.begin(); datArr.end() != itDat; ++itDat) {
                    ret->Data.push_back(uchar(it->toInt()));
                  }
                }
                it = objElem.find("WriteWithResponse");
                if (it != objElem.end())
                {
                  ret->WriteWithResponse = it->toBool();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRawWriteCmd*>(pMsg);
              QJsonArray datArr;
              for (qint32 i = 0; ms->Data.size() > i; ++i) {
                datArr.push_back(ms->Data.at(i));
              }
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Endpoint", ms->Endpoint},
                    {"Data", datArr},
                    {"WriteWithResponse", ms->WriteWithResponse}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RawReadCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRawReadCmd();
              ret->MessageType = "RawReadCmd";
              auto root = sIn.find("RawReadCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Endpoint");
                if (it != objElem.end())
                {
                  ret->Endpoint = it->toString();
                }
                it = objElem.find("ExpectedLength");
                if (it != objElem.end())
                {
                  ret->ExpectedLength = static_cast<qint64>(it->toDouble());
                }
                it = objElem.find("WaitForData");
                if (it != objElem.end())
                {
                  ret->WaitForData = it->toBool();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRawReadCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Endpoint", ms->Endpoint},
                    {"ExpectedLength", ms->ExpectedLength},
                    {"WaitForData", ms->WaitForData}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RawReading",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRawReading();
              ret->MessageType = "RawReading";
              auto root = sIn.find("RawReading");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Endpoint");
                if (it != objElem.end())
                {
                  ret->Endpoint = it->toString();
                }
                it = objElem.find("Data");
                if (it != objElem.end() && it->isArray())
                {
                  QJsonArray datArr = it->toArray();
                  for (auto itDat = datArr.begin(); datArr.end() != itDat; ++itDat) {
                    ret->Data.push_back(uchar(it->toInt()));
                  }
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRawReading*>(pMsg);
              QJsonArray datArr;
              for (qint32 i = 0; ms->Data.size() > i; ++i) {
                datArr.push_back(ms->Data.at(i));
              }
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Endpoint", ms->Endpoint},
                    {"Data", datArr}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RawSubscribeCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRawSubscribeCmd();
              ret->MessageType = "RawSubscribeCmd";
              auto root = sIn.find("RawSubscribeCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Endpoint");
                if (it != objElem.end())
                {
                  ret->Endpoint = it->toString();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRawSubscribeCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Endpoint", ms->Endpoint}
                  }
                }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          "RawUnsubscribeCmd",
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> ButtplugMessageBase* {
              auto ret = new ButtplugRawUnsubscribeCmd();
              ret->MessageType = "RawUnsubscribeCmd";
              auto root = sIn.find("RawUnsubscribeCmd");
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("DeviceIndex");
                if (it != objElem.end())
                {
                  ret->DeviceIndex = it->toInt();
                }
                it = objElem.find("Endpoint");
                if (it != objElem.end())
                {
                  ret->Endpoint = it->toString();
                }
              }
              return ret;
            },
            [](ButtplugMessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<ButtplugRawUnsubscribeCmd*>(pMsg);
              QJsonObject o = {
                {
                  ms->MessageType, QJsonObject {
                    {"Id", ms->Id},
                    {"DeviceIndex", ms->DeviceIndex},
                    {"Endpoint", ms->Endpoint}
                  }
                }
              };
              return o;
            }
          }
        } //,
      };
    }

  public:
    QMap<QString, QButtplugMessageParser> m_map;
  };

  QButtplugMessageParserRegistry staticRegistry;
}

//----------------------------------------------------------------------------------------
//
namespace QtButtplug
{
  void qRegisterMssageParser(const QString& sMsg,
                             std::function<ButtplugMessageBase*(const QJsonObject&, QtButtplug::ButtplugProtocolVersion)> fnFromJson,
                             std::function<QJsonObject(ButtplugMessageBase*, QtButtplug::ButtplugProtocolVersion)> fnToJson)
  {
    auto it = detail::staticRegistry.m_map.find(sMsg);
    if (detail::staticRegistry.m_map.end() != it)
    {
      detail::staticRegistry.m_map.insert(sMsg, detail::QButtplugMessageParser{fnFromJson, fnToJson});
    }
  }
}
