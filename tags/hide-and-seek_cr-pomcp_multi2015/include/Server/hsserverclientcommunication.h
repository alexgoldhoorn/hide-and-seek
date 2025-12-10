#ifndef HSSERVERCLIENTCOMMUNICATION_H
#define HSSERVERCLIENTCOMMUNICATION_H

//AG140530: block size type used in the network communication betwen client/server
#define BLOCK_SIZE quint32

//AG150119: type for message type
#define MESSAGE_TYPE_SIZE quint8

//AG150119: message type
//! Message type as sent by server/client
enum MessageType {MT_ClientRequestConn=0, MT_GameParams, MT_InitPos, MT_Update, MT_Action,
                  MT_SeekerGoals, MT_SeekerHB,
                  MT_Message=98, MT_StopServer=99};

//AG150119: indicate where to send a message to
//! Message send to field, where the first 3 values should be equal to the
enum MessageSendTo {MST_Hider=0, MST_Seeker1, MST_Seeker2, MST_All, MST_AllClients, MST_AllSeekers, MST_Server};

#endif // HSSERVERCLIENTCOMMUNICATION_H

