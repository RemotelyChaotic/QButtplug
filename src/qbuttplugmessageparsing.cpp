#include "qbuttplugmessageparsing.h"

#include <QDebug>
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
    std::function<QtButtplug::MessageBase*(const QJsonObject&, QtButtplug::ButtplugProtocolVersion)> m_fnFromJson;
    std::function<QJsonObject(QtButtplug::MessageBase*, QtButtplug::ButtplugProtocolVersion)> m_fnToJson;
  };

  //--------------------------------------------------------------------------------------
  void ParseDevice(QJsonObject devObj, QtButtplug::ButtplugProtocolVersion v,
                   QtButtplug::Device& dev)
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
            QtButtplug::ClientDeviceMessageAttribute attr;
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
        for (const QString& sKey : msgs.keys())
        {
          auto itmsg = msgs.find(sKey);
          if (itmsg->isObject()) {
            QtButtplug::ClientDeviceMessageAttribute attr;
            attr.MessageType = sKey;
            auto itAttrS = itmsg->toObject().find("StepCount");
            auto itAttrF = itmsg->toObject().find("FeatureCount");
            if (itmsg->toObject().end() != itAttrF) {
              qint32 iCount = itAttrF->toInt();
              QJsonArray stepCount;
              if (itmsg->toObject().end() != itAttrS && itAttrS->isArray()) {
                stepCount = itAttrS->toArray();
              }
              for (qint32 j = 0; iCount > j; ++j) {
                auto newMsg = QtButtplug::ClientDeviceMessageAttribute(attr);
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
        for (const QString& sKey : msgs.keys())
        {
          auto itmsg = msgs.find(sKey);
          if (itmsg->isObject() || itmsg->isArray()) {
            QtButtplug::ClientDeviceMessageAttribute attr;
            attr.MessageType = sKey;

            auto fnParseAttrs =  [&attr](QJsonObject o) -> QtButtplug::ClientDeviceMessageAttribute {
              auto ret = QtButtplug::ClientDeviceMessageAttribute(attr);
              auto itAttr = o.find("FeatureDescriptor");
              if (o.end() != itAttr)
              {
                ret.FeatureDescriptor = itAttr->toString();
              }
              itAttr = o.find("StepCount");
              if (o.end() != itAttr)
              {
                ret.StepCount = itAttr->toInt();
              }
              itAttr = o.find("ActuatorType");
              if (o.end() != itAttr)
              {
                ret.ActuatorType = itAttr->toString();
              }
              itAttr = o.find("SensorType");
              if (o.end() != itAttr)
              {
                ret.SensorType = itAttr->toString();
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
                      ret.SensorRange.push_back(minMax);
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
                    ret.Endpoints << itAtrArr->toString();
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

  void SerializeDevice(const QtButtplug::Device& dev, QtButtplug::ButtplugProtocolVersion v,
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
        for (const QtButtplug::ClientDeviceMessageAttribute& attr : it.value()) {
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
          for (const QtButtplug::ClientDeviceMessageAttribute& attr : it.value()) {
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
          QtButtplug::MessageTypeOk,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::Ok();
              auto root = sIn.find(QtButtplug::MessageTypeOk);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::Ok*>(pMsg);
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
          QtButtplug::MessageTypeError,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::ErrorMsg();
              auto root = sIn.find(QtButtplug::MessageTypeError);
              if (sIn.end() != root || !root->isObject()) {
                auto objElem = root->toObject();
                auto it = objElem.find("Id");
                if (it != objElem.end())
                {
                  ret->Id = it->toInt();
                }
                it = objElem.find("ErrorMessage");
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
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
          QtButtplug::MessageTypePing,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::Ping();
              auto root = sIn.find(QtButtplug::MessageTypePing);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::Ping*>(pMsg);
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
          QtButtplug::MessageTypeRequestServerInfo,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion v) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RequestServerInfo();
              auto root = sIn.find(QtButtplug::MessageTypeRequestServerInfo);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion v) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RequestServerInfo*>(pMsg);
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
          QtButtplug::MessageTypeServerInfo,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion v) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::ServerInfo();
              auto root = sIn.find(QtButtplug::MessageTypeServerInfo);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion v) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::ServerInfo*>(pMsg);
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
          QtButtplug::MessageTypeStartScanning,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::StartScanning();
              auto root = sIn.find(QtButtplug::MessageTypeStartScanning);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::StartScanning*>(pMsg);
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
          QtButtplug::MessageTypeStopScanning,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::StopScanning();
              auto root = sIn.find(QtButtplug::MessageTypeStopScanning);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::StopScanning*>(pMsg);
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
          QtButtplug::MessageTypeScanningFinished,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::ScanningFinished();
              auto root = sIn.find(QtButtplug::MessageTypeScanningFinished);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::ScanningFinished*>(pMsg);
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
          QtButtplug::MessageTypeRequestDeviceList,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RequestDeviceList();
              auto root = sIn.find(QtButtplug::MessageTypeRequestDeviceList);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RequestDeviceList*>(pMsg);
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
          QtButtplug::MessageTypeDeviceList,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion v) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::DeviceList();
              auto root = sIn.find(QtButtplug::MessageTypeDeviceList);
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
                      QtButtplug::Device dev;
                      auto devObj = arr.at(i).toObject();
                      ParseDevice(devObj, v, dev);
                      ret->Devices.push_back(dev);
                    }
                  }
                }
              }
              return ret;
            },
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion v) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::DeviceList*>(pMsg);
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
          QtButtplug::MessageTypeDeviceAdded,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion v) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::DeviceAdded();
              auto root = sIn.find(QtButtplug::MessageTypeDeviceAdded);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion v) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::DeviceAdded*>(pMsg);
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
          QtButtplug::MessageTypeDeviceRemoved,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::DeviceRemoved();
              auto root = sIn.find(QtButtplug::MessageTypeDeviceRemoved);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::DeviceRemoved*>(pMsg);
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
          QtButtplug::MessageTypeStopDeviceCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::StopDeviceCmd();
              auto root = sIn.find(QtButtplug::MessageTypeStopDeviceCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::StopDeviceCmd*>(pMsg);
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
          QtButtplug::MessageTypeStopAllDevices,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::StopAllDevices();
              auto root = sIn.find(QtButtplug::MessageTypeStopAllDevices);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::StopAllDevices*>(pMsg);
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
          QtButtplug::MessageTypeScalarCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::ScalarCmd();
              auto root = sIn.find(QtButtplug::MessageTypeScalarCmd);
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
                      QtButtplug::ScalarCmdElem elem;
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::ScalarCmd*>(pMsg);
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
          QtButtplug::MessageTypeVibrateCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::VibrateCmd();
              auto root = sIn.find(QtButtplug::MessageTypeVibrateCmd);
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
                      QtButtplug::VibrateCmdElem elem;
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::VibrateCmd*>(pMsg);
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
          QtButtplug::MessageTypeSingleMotorVibrateCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::SingleMotorVibrateCmd();
              auto root = sIn.find(QtButtplug::MessageTypeSingleMotorVibrateCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::SingleMotorVibrateCmd*>(pMsg);
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
          QtButtplug::MessageTypeLinearCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::LinearCmd();
              auto root = sIn.find(QtButtplug::MessageTypeLinearCmd);
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
                      QtButtplug::LinearCmdElem elem;
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::LinearCmd*>(pMsg);
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
          QtButtplug::MessageTypeRotateCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RotateCmd();
              auto root = sIn.find(QtButtplug::MessageTypeRotateCmd);
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
                      QtButtplug::RotateCmdElem elem;
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RotateCmd*>(pMsg);
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
          QtButtplug::MessageTypeSensorReadCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::SensorReadCmd();
              auto root = sIn.find(QtButtplug::MessageTypeSensorReadCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::SensorReadCmd*>(pMsg);
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
          QtButtplug::MessageTypeSensorReading,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::SensorReading();
              auto root = sIn.find(QtButtplug::MessageTypeSensorReading);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::SensorReading*>(pMsg);
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
          QtButtplug::MessageTypeSensorSubscribeCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::SensorSubscribeCmd();
              auto root = sIn.find(QtButtplug::MessageTypeSensorSubscribeCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::SensorSubscribeCmd*>(pMsg);
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
          QtButtplug::MessageTypeSensorUnsubscribeCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::SensorUnsubscribeCmd();
              auto root = sIn.find(QtButtplug::MessageTypeSensorUnsubscribeCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::SensorUnsubscribeCmd*>(pMsg);
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
          QtButtplug::MessageTypeBatteryLevelCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::BatteryLevelCmd();
              auto root = sIn.find(QtButtplug::MessageTypeBatteryLevelCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::BatteryLevelCmd*>(pMsg);
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
          QtButtplug::MessageTypeBatteryLevelReading,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::BatteryLevelReading();
              auto root = sIn.find(QtButtplug::MessageTypeBatteryLevelReading);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::BatteryLevelReading*>(pMsg);
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
          QtButtplug::MessageTypeRSSILevelCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RSSILevelCmd();
              auto root = sIn.find(QtButtplug::MessageTypeRSSILevelCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RSSILevelCmd*>(pMsg);
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
          QtButtplug::MessageTypeRSSILevelReading,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RSSILevelReading();
              auto root = sIn.find(QtButtplug::MessageTypeRSSILevelReading);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RSSILevelReading*>(pMsg);
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
          QtButtplug::MessageTypeRawWriteCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RawWriteCmd();
              auto root = sIn.find(QtButtplug::MessageTypeRawWriteCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RawWriteCmd*>(pMsg);
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
          QtButtplug::MessageTypeRawReadCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RawReadCmd();
              auto root = sIn.find(QtButtplug::MessageTypeRawReadCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RawReadCmd*>(pMsg);
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
          QtButtplug::MessageTypeRawReading,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RawReading();
              auto root = sIn.find(QtButtplug::MessageTypeRawReading);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RawReading*>(pMsg);
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
          QtButtplug::MessageTypeRawSubscribeCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RawSubscribeCmd();
              auto root = sIn.find(QtButtplug::MessageTypeRawSubscribeCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RawSubscribeCmd*>(pMsg);
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
          QtButtplug::MessageTypeRawUnsubscribeCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::RawUnsubscribeCmd();
              auto root = sIn.find(QtButtplug::MessageTypeRawUnsubscribeCmd);
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
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::RawUnsubscribeCmd*>(pMsg);
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
          QtButtplug::MessageTypeKiirooCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::KiirooCmd();
              auto root = sIn.find(QtButtplug::MessageTypeKiirooCmd);
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
                it = objElem.find("Command");
                if (it != objElem.end())
                {
                  ret->Command = it->toString();
                }
              }
              return ret;
            },
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::KiirooCmd*>(pMsg);
              QJsonObject o = {
                  {
                      ms->MessageType, QJsonObject {
                          {"Id", ms->Id},
                          {"DeviceIndex", ms->DeviceIndex},
                          {"Command", ms->Command}
                      }
                  }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          QtButtplug::MessageTypeFleshlightLaunchFW12Cmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::FleshlightLaunchFW12Cmd();
              auto root = sIn.find(QtButtplug::MessageTypeFleshlightLaunchFW12Cmd);
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
                it = objElem.find("Position");
                if (it != objElem.end())
                {
                  ret->Position = it->toInt();
                }
                it = objElem.find("Speed");
                if (it != objElem.end())
                {
                  ret->Speed = it->toInt();
                }
              }
              return ret;
            },
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::FleshlightLaunchFW12Cmd*>(pMsg);
              QJsonObject o = {
                  {
                      ms->MessageType, QJsonObject {
                          {"Id", ms->Id},
                          {"DeviceIndex", ms->DeviceIndex},
                          {"Position", ms->Position},
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
          QtButtplug::MessageTypeLovenseCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::LovenseCmd();
              auto root = sIn.find(QtButtplug::MessageTypeLovenseCmd);
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
                it = objElem.find("Command");
                if (it != objElem.end())
                {
                  ret->Command = it->toString();
                }
              }
              return ret;
            },
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::LovenseCmd*>(pMsg);
              QJsonObject o = {
                  {
                      ms->MessageType, QJsonObject {
                          {"Id", ms->Id},
                          {"DeviceIndex", ms->DeviceIndex},
                          {"Command", ms->Command}
                      }
                  }
              };
              return o;
            }
          }
        },
        //--------------------------------------------------------------------------------
        {
          QtButtplug::MessageTypeVorzeA10CycloneCmd,
          QButtplugMessageParser{
            [](const QJsonObject& sIn, QtButtplug::ButtplugProtocolVersion) -> QtButtplug::MessageBase* {
              auto ret = new QtButtplug::VorzeA10CycloneCmd();
              auto root = sIn.find(QtButtplug::MessageTypeVorzeA10CycloneCmd);
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
                  ret->Speed = it->toInt();
                }
                it = objElem.find("Clockwise");
                if (it != objElem.end())
                {
                  ret->Speed = it->toBool();
                }
              }
              return ret;
            },
            [](QtButtplug::MessageBase* pMsg, QtButtplug::ButtplugProtocolVersion) -> QJsonObject {
              auto ms = dynamic_cast<QtButtplug::VorzeA10CycloneCmd*>(pMsg);
              QJsonObject o = {
                  {
                      ms->MessageType, QJsonObject {
                          {"Id", ms->Id},
                          {"DeviceIndex", ms->DeviceIndex},
                          {"Speed", ms->Speed},
                          {"Clockwise", ms->Clockwise}
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
                             std::function<QtButtplug::MessageBase*(const QJsonObject&, QtButtplug::ButtplugProtocolVersion)> fnFromJson,
                             std::function<QJsonObject(QtButtplug::MessageBase*, QtButtplug::ButtplugProtocolVersion)> fnToJson)
  {
    auto it = detail::staticRegistry.m_map.find(sMsg);
    if (detail::staticRegistry.m_map.end() != it)
    {
      detail::staticRegistry.m_map.insert(sMsg, detail::QButtplugMessageParser{fnFromJson, fnToJson});
    }
  }
}

//----------------------------------------------------------------------------------------
//
QT_BEGIN_NAMESPACE

QButtplugMessageSerializer::QButtplugMessageSerializer()
{}

//----------------------------------------------------------------------------------------
//
void QButtplugMessageSerializer::setProtocolVersion(QtButtplug::ButtplugProtocolVersion v)
{
  m_v = v;
}

//----------------------------------------------------------------------------------------
//
QString QButtplugMessageSerializer::Serialize(QtButtplug::MessageBase* pMsg)
{
  auto it = detail::staticRegistry.m_map.find(pMsg->MessageType);
  if (detail::staticRegistry.m_map.end() != it) {
    auto obj = it->m_fnToJson(pMsg, m_v);
    QJsonArray arr;
    arr.append(obj);
    QJsonDocument doc(arr);
    return doc.toJson(QJsonDocument::Compact);
  }
  qWarning() << "Unknown buttplug message type.";
  return QString();
}

//----------------------------------------------------------------------------------------
//
QList<QtButtplug::MessageBase*> QButtplugMessageSerializer::Deserialize(const QString& sMsg)
{
  QList<QtButtplug::MessageBase*> ret;
  QJsonDocument doc = QJsonDocument::fromJson(sMsg.toUtf8());
  if (doc.isArray()) {
    QJsonArray arr = doc.array();
    for (qint32 i = 0; arr.count() > i; ++i) {
      auto it = arr.at(i);
      if (!it.isObject()) {
        qWarning() << "QButtplugMessageSerializer: Received JSON element is not an object.";
        continue;
      }

      auto obj = it.toObject();
      QStringList vsKeys = obj.keys();
      if (vsKeys.empty()) {
        qWarning() << "QButtplugMessageSerializer: No keys found in received JSON object.";
        continue;
      }

      auto& map = detail::staticRegistry.m_map;
      auto itMap = map.find(vsKeys[0]);
      if (map.end() == itMap) {
        qWarning() << "Unknown buttplug message type.";
      }

      ret << itMap->m_fnFromJson(obj, m_v);
    }
    return ret;
  }

  qWarning() << "QButtplugMessageSerializer: Received JSON is malformed.";
  return {};
}

QT_END_NAMESPACE
