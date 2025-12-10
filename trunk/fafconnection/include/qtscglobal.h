#ifndef QTSCGLOBAL
#define QTSCGLOBAL

#include <QDataStream>

//AG140530: block size type used in the network communication betwen client/server
#define BLOCK_SIZE quint16

//AG150119: type for message type
#define MESSAGE_TYPE_SIZE quint8

//AG150119: message type
//! Message type as sent by server/client
enum MessageType {MT_ClientRequestConn=0, MT_Obs, MT_Goals,
                  MT_Message=98, MT_StopServer=99};

static const QDataStream::Version DATASTREAM_VERSION = QDataStream::Qt_5_3;

#endif // QTSCGLOBAL

