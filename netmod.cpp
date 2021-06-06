#include "netmod.h"

Netmod::Netmod()
{
    server=new QTcpServer(this);
    server->listen(QHostAddress::Any,8891);
    QObject::connect(server,SIGNAL(newConnection()),this,SLOT(getConnect()));

}

Netmod::~Netmod()
{
}

void Netmod::getConnect()
{
    std::cout<<"here_1"<<std::endl;
    socket=server->nextPendingConnection();
    std::cout<<"here_2"<<std::endl;
    std::cout<<"here_3"<<std::endl;
    QString strMesg="Hello,World!是不是？";
    //socket->write(strMesg.toStdString().c_str(),strlen(strMesg.toStdString().c_str()));
}
