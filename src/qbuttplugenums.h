#ifndef QBUTTPLUGENUMS_H
#define QBUTTPLUGENUMS_H

#include "qobjectdefs.h"

#include <QMetaType>
#include <QtGlobal>

QT_BEGIN_NAMESPACE

namespace QtButtplug
{
  enum ConnectionState
  {
    Disconnected,
    Connecting,
    Handshake,
    Connected
  };

  enum ButtplugProtocolVersion
  {
    AnyProtocolVersion = -1,
    ProtocolV0 = 0,
    ProtocolV1 = 1,
    ProtocolV2 = 2,
    ProtocolV3 = 3,
  };

  enum Error
  {
    ERROR_OK = -1, // No error.
    ERROR_UNKNOWN = 0, // An unknown error occurred.
    ERROR_INIT = 1, // Handshake did not succeed.
    ERROR_PING = 2, // A ping was not sent in the expected time.
    ERROR_MSG = 3, // A message parsing or permission error occurred.
    ERROR_DEVICE = 4, // A command sent to a device returned an error.
    ERROR_PING_TIMEOUT = 0xff, // Ping timeout
    ERROR_SOCKET_ERR,  // Socket error.
    ERROR_TIMEOUT, // Respoonse timeout.
    ERROR_NOT_SUPPORTED // Feature is not supported by the current server or client.
  };
}

Q_DECLARE_METATYPE(QtButtplug::ConnectionState)
Q_DECLARE_METATYPE(QtButtplug::Error)

QT_END_NAMESPACE

#endif // QBUTTPLUGENUMS_H
