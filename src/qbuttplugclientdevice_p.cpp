#include "qbuttplugclient_p.h"
#include "qbuttplugclientdevice_p.h"

QT_BEGIN_NAMESPACE

QButtplugClientDevicePrivate::QButtplugClientDevicePrivate(QButtplugClientPrivate* const pParent,
                                                           const QtButtplug::Device* const pMsg) :
    QObject(pParent),
    m_pParent(pParent)
{
  m_iId = pMsg->DeviceIndex;
  m_sName = pMsg->DeviceName;
  m_sDisplayName = pMsg->DeviceDisplayName;
  m_iTimingGap = pMsg->DeviceMessageTimingGap;
  m_messages = pMsg->DeviceMessages;
}
QButtplugClientDevicePrivate::~QButtplugClientDevicePrivate()
{
}

//----------------------------------------------------------------------------------------
//
double QButtplugClientDevicePrivate::batteryLevel(QtButtplug::Error* pErr)
{
  QtButtplug::MessageBase* pMsg = nullptr;
  QStringList vsExpectedResponses;
  QPair<qint32, qint32> v3SensorRanges;
  switch (m_pParent->m_iMsgVersionUsed)
  {
    case QtButtplug::ProtocolV3:
    {
      auto it = m_messages.find(QtButtplug::V3::DeviceMessageTypeSensorRead);
      if (m_messages.end() != it) {
        auto itSensorAttr = std::find_if(it.value().begin(), it.value().end(),
                     [](const QtButtplug::ClientDeviceMessageAttribute& attr){
                        return attr.SensorType == QtButtplug::V3::DeviceFeatureBattery;
                     });
        if (it.value().end() != itSensorAttr) {
          auto pCmd = new QtButtplug::SensorReadCmd;
          v3SensorRanges = itSensorAttr->SensorRange.size() > 0 ?
                               itSensorAttr->SensorRange[0] :
                               QPair<qint32, qint32>{0, 1};
          pCmd->Id = m_pParent->getNextId();
          pCmd->DeviceIndex = m_iId;
          pCmd->SensorIndex = std::distance(it.value().begin(), itSensorAttr);
          pCmd->SensorType = QtButtplug::V3::DeviceFeatureBattery;
          pMsg = pCmd;
          vsExpectedResponses << QtButtplug::MessageTypeSensorReading << QtButtplug::MessageTypeError;
        } else {
          q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
          if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
          return 0.0;
        }
      } else {
        q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
        if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
        return 0.0;
      }
    } break;
    case QtButtplug::ProtocolV2:
    {
      auto pCmd = new QtButtplug::BatteryLevelCmd;
      pCmd->Id = m_pParent->getNextId();
      pCmd->DeviceIndex = m_iId;
      pMsg = pCmd;
      vsExpectedResponses << QtButtplug::MessageTypeBatteryLevelReading << QtButtplug::MessageTypeError;
    } break;
    default:
      q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
      if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
      return 0.0;
  }

  double dValue = 0.0;
  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pMsg, vsExpectedResponses,
       [this, &dValue, &err, &sError, v3SensorRanges] (QtButtplug::MessageBase* pMsg) mutable {
         if (QtButtplug::MessageTypeError == pMsg->MessageType) {
           auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
           err = pErr->ErrorCode;
           sError = pErr->ErrorMessage;
         } else if (QtButtplug::MessageTypeSensorReading == pMsg->MessageType) {
           auto pCasted = dynamic_cast<QtButtplug::SensorReading*>(pMsg);
           dValue = pCasted->Data.size() > 0.0 ? double(pCasted->Data[0]) : 0.0;
           // normalize to 0.0 - 1.0
           qint32 iMin = v3SensorRanges.first;
           qint32 iMax = v3SensorRanges.second;
           dValue = (dValue - iMin) / (iMax - iMin);
         } else if (QtButtplug::MessageTypeBatteryLevelReading == pMsg->MessageType) {
           auto pCasted = dynamic_cast<QtButtplug::BatteryLevelReading*>(pMsg);
           dValue = pCasted->BatteryLevel;
         }
         emit m_pParent->q_await_response();
       });

  if (nullptr != pErr) *pErr = QtButtplug::ERROR_OK;

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    if (nullptr != pErr) *pErr = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return dValue;
}

//----------------------------------------------------------------------------------------
//
double QButtplugClientDevicePrivate::rssiLevel(QtButtplug::Error* pErr)
{
  QtButtplug::MessageBase* pMsg = nullptr;
  QStringList vsExpectedResponses;
  QPair<qint32, qint32> v3SensorRanges;
  switch (m_pParent->m_iMsgVersionUsed)
  {
    case QtButtplug::ProtocolV3:
      {
      auto it = m_messages.find(QtButtplug::V3::DeviceMessageTypeSensorRead);
      if (m_messages.end() != it) {
        auto itSensorAttr = std::find_if(it.value().begin(), it.value().end(),
                                         [](const QtButtplug::ClientDeviceMessageAttribute& attr){
                                           return attr.SensorType == QtButtplug::V3::DeviceFeatureRSSI;
                                         });
        if (it.value().end() != itSensorAttr) {
          auto pCmd = new QtButtplug::SensorReadCmd;
          v3SensorRanges = itSensorAttr->SensorRange.size() > 0 ?
                               itSensorAttr->SensorRange[0] :
                               QPair<qint32, qint32>{0, 1};
          pCmd->Id = m_pParent->getNextId();
          pCmd->DeviceIndex = m_iId;
          pCmd->SensorIndex = std::distance(it.value().begin(), itSensorAttr);
          pCmd->SensorType = QtButtplug::V3::DeviceFeatureRSSI;
          pMsg = pCmd;
          vsExpectedResponses << QtButtplug::MessageTypeSensorReading << QtButtplug::MessageTypeError;
        } else {
          q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
          if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
          return 0.0;
        }
      } else {
        q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
        if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
        return 0.0;
      }
      } break;
    case QtButtplug::ProtocolV2:
      {
      auto pCmd = new QtButtplug::RSSILevelCmd;
      pCmd->Id = m_pParent->getNextId();
      pCmd->DeviceIndex = m_iId;
      pMsg = pCmd;
      vsExpectedResponses << QtButtplug::MessageTypeRSSILevelReading << QtButtplug::MessageTypeError;
      } break;
    default:
      q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
      if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
      return 0.0;
  }

  double dValue = 0.0;
  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pMsg, vsExpectedResponses,
                  [this, &dValue, &err, &sError, v3SensorRanges] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    } else if (QtButtplug::MessageTypeSensorReading == pMsg->MessageType) {
                      auto pCasted = dynamic_cast<QtButtplug::SensorReading*>(pMsg);
                      dValue = pCasted->Data.size() > 0.0 ? double(pCasted->Data[0]) : 0.0;
                      // normalize to -100.0 - 0.0
                      qint32 iMin = v3SensorRanges.first;
                      qint32 iMax = v3SensorRanges.second;
                      dValue = (dValue - iMin) / (iMax - iMin);
                      dValue = (dValue - 1.0) * 100.0;
                    } else if (QtButtplug::MessageTypeBatteryLevelReading == pMsg->MessageType) {
                      auto pCasted = dynamic_cast<QtButtplug::RSSILevelReading*>(pMsg);
                      dValue = pCasted->RSSILevel;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (nullptr != pErr) *pErr = QtButtplug::ERROR_OK;

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    if (nullptr != pErr) *pErr = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return dValue;
}

//----------------------------------------------------------------------------------------
//
QList<int> QButtplugClientDevicePrivate::readSensor(quint32 sensorIndex, QtButtplug::Error* pErr)
{
  if (QtButtplug::ProtocolV3 != m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
    return {};
  }

  auto it = m_messages.find(QtButtplug::V3::DeviceMessageTypeSensorRead);
  if (m_messages.end() == it) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
    return {};
  }

  if (it.value().size() <= static_cast<qint32>(sensorIndex) || 0 > sensorIndex) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
    return {};
  }

  const auto& attr = it->at(sensorIndex);

  auto pCmd = new QtButtplug::SensorReadCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->SensorIndex = sensorIndex;
  pCmd->SensorType = attr.SensorType;

  QList<int> vValues;
  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeSensorReading << QtButtplug::MessageTypeError,
                  [this, &vValues, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    } else if (QtButtplug::MessageTypeSensorReading == pMsg->MessageType) {
                      auto pCasted = dynamic_cast<QtButtplug::SensorReading*>(pMsg);
                      vValues = pCasted->Data;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (nullptr != pErr) *pErr = QtButtplug::ERROR_OK;

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    if (nullptr != pErr) *pErr = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return vValues;
}

//----------------------------------------------------------------------------------------
//
QByteArray QButtplugClientDevicePrivate::sendRawReadCmd(const QString& endpoint, qint64 iExpectedLength,
                                                        bool bWaitForData, QtButtplug::Error* pErr)
{
  if (QtButtplug::ProtocolV0 == m_pParent->m_iMsgVersionUsed ||
      QtButtplug::ProtocolV1 == m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    if (nullptr != pErr) *pErr = QtButtplug::ERROR_NOT_SUPPORTED;
    return {};
  }

  auto pCmd = new QtButtplug::RawReadCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->Endpoint = endpoint;
  pCmd->ExpectedLength = iExpectedLength;
  pCmd->WaitForData = bWaitForData;

  QByteArray data;
  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeRawReading << QtButtplug::MessageTypeError,
                  [this, &data, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    } else if (QtButtplug::MessageTypeRawReading == pMsg->MessageType) {
                      auto pCasted = dynamic_cast<QtButtplug::RawReading*>(pMsg);
                      data = pCasted->Data;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (nullptr != pErr) *pErr = QtButtplug::ERROR_OK;

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    if (nullptr != pErr) *pErr = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return data;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendStopDeviceCmd()
{
  auto pCmd = new QtButtplug::StopDeviceCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendScalarCmd(double dValue, qint32 accuatorIndex)
{
  if (QtButtplug::ProtocolV0 == m_pParent->m_iMsgVersionUsed ||
      QtButtplug::ProtocolV1 == m_pParent->m_iMsgVersionUsed ||
      QtButtplug::ProtocolV2 == m_pParent->m_iMsgVersionUsed)
  {
    return q_sendVibrateCmd(dValue, accuatorIndex);
  }

  return q_sendScalarCmd(dValue, accuatorIndex);
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendLinearCmd(qint64 iDurationMs, double dPosition, qint32 accuatorIndex)
{
  if (QtButtplug::ProtocolV0 == m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  QMap<QString, QList<QtButtplug::ClientDeviceMessageAttribute>>::Iterator it;
  switch(m_pParent->m_iMsgVersionUsed)
  {
    case QtButtplug::ProtocolV3:
      it = m_messages.find(QtButtplug::V3::DeviceMessageTypeLinear);
      break;
    case QtButtplug::ProtocolV2:
      it = m_messages.find(QtButtplug::V2::DeviceMessageTypeLinear);
      break;
    case QtButtplug::ProtocolV1:
      it = m_messages.find(QtButtplug::V1::DeviceMessageTypeLinear);
      break;
    default:
      q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
      return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  if (m_messages.end() == it) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  if (it.value().size() <= accuatorIndex || -1 > accuatorIndex) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto pCmd = new QtButtplug::LinearCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  if (accuatorIndex >= 0) {
    pCmd->Vectors = {
      QtButtplug::LinearCmdElem{accuatorIndex, iDurationMs, std::min(1.0, std::max(0.0, dPosition))}
    };
  } else {
    QList<QtButtplug::LinearCmdElem> vElems;
    for (qint32 i = 0; it->size() > i; ++i) {
      vElems << QtButtplug::LinearCmdElem{i, iDurationMs, std::min(1.0, std::max(0.0, dPosition))};
    }
    pCmd->Vectors = vElems;
  }

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendRotateCmd(bool bClockwise, double dSpeed, qint32 accuatorIndex)
{
  if (QtButtplug::ProtocolV0 == m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  QMap<QString, QList<QtButtplug::ClientDeviceMessageAttribute>>::Iterator it;
  switch(m_pParent->m_iMsgVersionUsed)
  {
    case QtButtplug::ProtocolV3:
      it = m_messages.find(QtButtplug::V3::DeviceMessageTypeRotate);
      break;
    case QtButtplug::ProtocolV2:
      it = m_messages.find(QtButtplug::V2::DeviceMessageTypeRotate);
      break;
    case QtButtplug::ProtocolV1:
      it = m_messages.find(QtButtplug::V1::DeviceMessageTypeRotate);
      break;
    default:
      q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
      return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  if (m_messages.end() == it) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  if (it.value().size() <= accuatorIndex || -1 > accuatorIndex) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto pCmd = new QtButtplug::RotateCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  if (accuatorIndex >= 0) {
    pCmd->Rotations = {
        QtButtplug::RotateCmdElem{accuatorIndex, std::min(1.0, std::max(0.0, dSpeed)), bClockwise}
    };
  } else {
    QList<QtButtplug::RotateCmdElem> vElems;
    for (qint32 i = 0; it->size() > i; ++i) {
      vElems << QtButtplug::RotateCmdElem{i, std::min(1.0, std::max(0.0, dSpeed)), bClockwise};
    }
    pCmd->Rotations = vElems;
  }

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendVibrateCmd(double dSpeed, qint32 accuatorIndex)
{
  if (QtButtplug::ProtocolV0 != m_pParent->m_iMsgVersionUsed &&
      QtButtplug::ProtocolV1 != m_pParent->m_iMsgVersionUsed &&
      QtButtplug::ProtocolV2 != m_pParent->m_iMsgVersionUsed)
  {
    return q_sendScalarCmd(dSpeed, accuatorIndex);
  }

  return q_sendVibrateCmd(dSpeed, accuatorIndex);
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendKiirooCmd(const QString& cmd)
{
  auto pCmd = new QtButtplug::KiirooCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->Command = cmd;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendFleshlightLaunchFW12Cmd(quint32 iPosition, quint32 iSpeed)
{
  auto pCmd = new QtButtplug::FleshlightLaunchFW12Cmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->Position = iPosition;
  pCmd->Speed = iSpeed;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendLovenseCmd(const QString& cmd)
{
  auto pCmd = new QtButtplug::LovenseCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->Command = cmd;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendVorzeA10CycloneCmd(quint32 iSpeed, bool bClockwise)
{
  auto pCmd = new QtButtplug::VorzeA10CycloneCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->Speed = iSpeed;
  pCmd->Clockwise = bClockwise;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendSensorSubscribeCmd(qint32 sensorIndex)
{
  if (QtButtplug::ProtocolV3 != m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto it = m_messages.find(QtButtplug::V3::DeviceMessageTypeSensorSubscribe);
  if (m_messages.end() == it) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  if (it.value().size() <= sensorIndex || 0 > sensorIndex) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto pCmd = new QtButtplug::SensorSubscribeCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->SensorIndex = sensorIndex;
  pCmd->SensorType = it->at(sensorIndex).SensorType;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendSensorUnsubscribeCmd(qint32 sensorIndex)
{
  if (QtButtplug::ProtocolV3 != m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto it = m_messages.find(QtButtplug::V3::DeviceMessageTypeSensorSubscribe);
  if (m_messages.end() == it) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  if (it.value().size() <= sensorIndex || 0 > sensorIndex) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto pCmd = new QtButtplug::SensorUnsubscribeCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->SensorIndex = sensorIndex;
  pCmd->SensorType = it->at(sensorIndex).SensorType;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendRawWriteCmd(const QString& endpoint,
                                                                QByteArray data,
                                                                bool bWriteWithResponse)
{
  if (QtButtplug::ProtocolV0 == m_pParent->m_iMsgVersionUsed ||
      QtButtplug::ProtocolV1 == m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto pCmd = new QtButtplug::RawWriteCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->Endpoint = endpoint;
  pCmd->Data = data;
  pCmd->WriteWithResponse = bWriteWithResponse;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendRawSubscribeCmd(const QString& endpoint)
{
  if (QtButtplug::ProtocolV0 == m_pParent->m_iMsgVersionUsed ||
      QtButtplug::ProtocolV1 == m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto pCmd = new QtButtplug::RawSubscribeCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->Endpoint = endpoint;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::sendRawUnsubscribeCmd(const QString& endpoint)
{
  if (QtButtplug::ProtocolV0 == m_pParent->m_iMsgVersionUsed ||
      QtButtplug::ProtocolV1 == m_pParent->m_iMsgVersionUsed)
  {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto pCmd = new QtButtplug::RawUnsubscribeCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  pCmd->Endpoint = endpoint;

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::q_sendScalarCmd(double dValue, qint32 accuatorIndex)
{
  QMap<QString, QList<QtButtplug::ClientDeviceMessageAttribute>>::Iterator it =
      m_messages.find(QtButtplug::V3::DeviceMessageTypeScalar);

  if (m_messages.end() == it) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  if (it.value().size() <= accuatorIndex || -1 > accuatorIndex) {
    q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
    return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  auto pCmd = new QtButtplug::ScalarCmd;
  pCmd->Id = m_pParent->getNextId();
  pCmd->DeviceIndex = m_iId;
  if (accuatorIndex >= 0) {
    pCmd->Scalars = {
        QtButtplug::ScalarCmdElem{accuatorIndex, std::min(1.0, std::max(0.0, dValue)), it->at(accuatorIndex).ActuatorType}
    };
  } else {
    QList<QtButtplug::ScalarCmdElem> vElems;
    for (qint32 i = 0; it->size() > i; ++i) {
      vElems << QtButtplug::ScalarCmdElem{i, std::min(1.0, std::max(0.0, dValue)), it->at(i).ActuatorType};
    }
    pCmd->Scalars = vElems;
  }

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pCmd, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientDevicePrivate::q_sendVibrateCmd(double dValue, qint32 accuatorIndex)
{
  QtButtplug::MessageBase* pMsg = nullptr;
  switch (m_pParent->m_iMsgVersionUsed)
  {
    case QtButtplug::ProtocolV0:
    {
      QtButtplug::SingleMotorVibrateCmd* pCmd = new QtButtplug::SingleMotorVibrateCmd;
      pCmd->Id = m_pParent->getNextId();
      pCmd->DeviceIndex = m_iId;
      pCmd->Speed = std::min(1.0, std::max(0.0, dValue));
      pMsg = pCmd;
    } break;
    case QtButtplug::ProtocolV1:
    {
      QMap<QString, QList<QtButtplug::ClientDeviceMessageAttribute>>::Iterator it =
          m_messages.find(QtButtplug::V1::DeviceMessageTypeVibrate);

      if (m_messages.end() == it) {
        q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
        return QtButtplug::ERROR_NOT_SUPPORTED;
      }

      if (it.value().size() <= accuatorIndex || -1 > accuatorIndex) {
        q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
        return QtButtplug::ERROR_NOT_SUPPORTED;
      }

      QtButtplug::VibrateCmd* pCmd = new QtButtplug::VibrateCmd;
      pCmd->Id = m_pParent->getNextId();
      pCmd->DeviceIndex = m_iId;
      if (accuatorIndex >= 0) {
        pCmd->Speeds = {
          QtButtplug::VibrateCmdElem{accuatorIndex, dValue}
        };
      } else {
        QList<QtButtplug::VibrateCmdElem> vElems;
        for (qint32 i = 0; it->size() > i; ++i) {
          vElems << QtButtplug::VibrateCmdElem{i, dValue};
        }
        pCmd->Speeds = vElems;
      }
      pMsg = pCmd;
    } break;
    case QtButtplug::ProtocolV2:
    {
      QMap<QString, QList<QtButtplug::ClientDeviceMessageAttribute>>::Iterator it =
          m_messages.find(QtButtplug::V2::DeviceMessageTypeVibrate);

      if (m_messages.end() == it) {
        q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
        return QtButtplug::ERROR_NOT_SUPPORTED;
      }

      if (it.value().size() <= accuatorIndex || -1 > accuatorIndex) {
        q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
        return QtButtplug::ERROR_NOT_SUPPORTED;
      }

      QtButtplug::VibrateCmd* pCmd = new QtButtplug::VibrateCmd;
      pCmd->Id = m_pParent->getNextId();
      pCmd->DeviceIndex = m_iId;
      if (accuatorIndex >= 0) {
        pCmd->Speeds = {
            QtButtplug::VibrateCmdElem{accuatorIndex, dValue}
        };
      } else {
        QList<QtButtplug::VibrateCmdElem> vElems;
        for (qint32 i = 0; it->size() > i; ++i) {
          vElems << QtButtplug::VibrateCmdElem{i, dValue};
        }
        pCmd->Speeds = vElems;
      }
      pMsg = pCmd;
    } break;
    default:
      q_setErr(QtButtplug::ERROR_NOT_SUPPORTED, QString());
      return QtButtplug::ERROR_NOT_SUPPORTED;
  }

  QtButtplug::Error err = QtButtplug::ERROR_OK;
  QString sError;
  m_pParent->send(pMsg, QStringList() << QtButtplug::MessageTypeOk << QtButtplug::MessageTypeError,
                  [this, &err, &sError] (QtButtplug::MessageBase* pMsg) mutable {
                    if (QtButtplug::MessageTypeError == pMsg->MessageType) {
                      auto pErr = dynamic_cast<QtButtplug::ErrorMsg*>(pMsg);
                      err = pErr->ErrorCode;
                      sError = pErr->ErrorMessage;
                    }
                    emit m_pParent->q_await_response();
                  });

  if (!q_busy_wait(m_pParent, &QButtplugClientPrivate::q_await_response,
                   QtButtplug::c_iGlobalTimeout)) {
    err = QtButtplug::ERROR_TIMEOUT;
    sError = QString();
  }

  q_setErr(err, sError);

  return err;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientDevicePrivate::q_setErr(QtButtplug::Error err, const QString& sErr)
{
  m_lastError = err;
  m_errorDetailString = sErr;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientDevicePrivate::q_resetErr()
{
  m_lastError = QtButtplug::ERROR_OK;
  m_errorDetailString = QString();
}

QT_END_NAMESPACE
