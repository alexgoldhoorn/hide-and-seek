#ifndef HSSERVER_H
#define HSSERVER_H

//AG111201: created server class

#include <QString>
#include "hsserverconfig.h"
#include "hsgamelog.h"


class HSTCPServer; //can't put include because it is a cyclic reference, now is included in cpp

class HSServer {
public:

    //! MODE waiting for clients
   /* HSServer(int port);

    //! MODE serving client
    HSServer(QString ip, int port);
*/
    HSServer(QString xmlConfigFile);

private:
  /*  void init(QString ip, int port);
    void init(int port);
*/


    HSTCPServer* _hstcpserver;

    //! Server config
    //AG120525: config is a pointer because when it was local, in hstcpserver it was not readable after constructor
    // maybe to do with 'threads'??? but here no threads...
    HSServerConfig* _config;

    //Player* _player;

    HSGameLog* _gameLog;

};

#endif // HSSERVER_H
