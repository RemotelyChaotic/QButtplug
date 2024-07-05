#include "qbuttplugclient_p.h"

#include <QObject>
#include <QMap>
#include <QString>

//----------------------------------------------------------------------------------------
//
bool QButtplugClientPrivate::waitConnected(qint64 iTimeoutMs)
{
  Q_UNUSED(iTimeoutMs)
  return false;
}

//----------------------------------------------------------------------------------------
//
bool QButtplugClientPrivate::waitDisconnected(qint64 iTimeoutMs)
{
  Q_UNUSED(iTimeoutMs)
  return false;
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::connect(const QHostAddress& sAddr, qint16 iPort)
{
  Q_UNUSED(sAddr)
  Q_UNUSED(iPort)
}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::disconnect()
{

}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::startScan()
{

}

//----------------------------------------------------------------------------------------
//
void QButtplugClientPrivate::stopScan()
{

}

//----------------------------------------------------------------------------------------
//
QtButtplug::Error QButtplugClientPrivate::stopAllDevices()
{
  return QtButtplug::ERROR_OK;
}

QT_END_NAMESPACE
