#ifndef NETMOD_H
#define NETMOD_H

#include <iostream>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpSocket>

class Netmod : public QObject
{
  Q_OBJECT
public:
   Netmod();
   ~Netmod();

public slots:
   void getConnect();
   private:
   QTcpServer *server;
   QTcpSocket *socket;
};

#endif
