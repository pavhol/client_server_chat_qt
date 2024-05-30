#include "myserver.h"

MyServer::MyServer(QWidget *widget, QObject *parent) :QTcpServer(parent)
{
    _widget = widget;
}

bool MyServer::doStartServer(QHostAddress addr, qint16 port)
{
    if (!listen(addr, port))
    {
        qDebug() << "Server not started at" << addr << ":" << port;
        return false;
    }
    qDebug() << "Server started at" << addr << ":" << port;
    return true;
}

void MyServer::doStopServer()
{
    this->close();
    for(auto client: _clients)
    {
        client->_sok->disconnectFromHost();
    }
    qDebug() << "Server stopped";
}

void MyServer::doSendToAllUserJoin(QString name)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out << (quint16)0 << MyClient::comUserJoin << name;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    for (int i = 0; i < _clients.length(); ++i)
        if (_clients.at(i)->getName() != name && _clients.at(i)->getAutched())
            _clients.at(i)->_sok->write(block);
    addLogToGui(" " + name + " connected");
    addUser(name);
}

void MyServer::doSendToAllUserLeft(QString name)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0 << MyClient::comUserLeft << name;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    for (int i = 0; i < _clients.length(); ++i)
        if (_clients.at(i)->getName() != name && _clients.at(i)->getAutched())
            _clients.at(i)->_sok->write(block);
    addLogToGui(" " + name + " disconnected");
    removeUser(name);
}

void MyServer::doSendToAllMessage(QString message, QString fromUsername)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0 << MyClient::comMessageToAll << fromUsername << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    for (int i = 0; i < _clients.length(); ++i)
        if (_clients.at(i)->getAutched())
            _clients.at(i)->_sok->write(block);
    addLogToGui(QString(block)+ " " + fromUsername + " " + message);
}

void MyServer::doSendToAllServerMessage(QString message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0 << MyClient::comPublicServerMessage << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    for (int i = 0; i < _clients.length(); ++i)
        if (_clients.at(i)->getAutched())
            _clients.at(i)->_sok->write(block);
    addLogToGui(QString(block) + " server: " + message);
}

void MyServer::doSendServerMessageToUsers(QString message, const QStringList &users)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0 << MyClient::comPrivateServerMessage << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));
    for (int j = 0; j < _clients.length(); ++j)
        if (users.contains(_clients.at(j)->getName()))
            _clients.at(j)->_sok->write(block);
    addLogToGui(QString(block));
}

void MyServer::doSendMessageToUsers(QString message, const QStringList &users, QString fromUsername)
{
    QByteArray block, blockToSender;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0 << MyClient::comMessageToUsers << fromUsername << message;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    QDataStream outToSender(&blockToSender, QIODevice::WriteOnly);
    outToSender << (quint16)0 << MyClient::comMessageToUsers << users.join(",") << message;
    outToSender.device()->seek(0);
    outToSender << (quint16)(blockToSender.size() - sizeof(quint16));
    for (int j = 0; j < _clients.length(); ++j)
        if (users.contains(_clients.at(j)->getName()))
            _clients.at(j)->_sok->write(block);
        else if (_clients.at(j)->getName() == fromUsername)
            _clients.at(j)->_sok->write(blockToSender);
    addLogToGui("PRIVATE MESSAGE [" + fromUsername + "] to [" + users.join(",") + "]: " + message, Qt::blue);
}

QStringList MyServer::getUsersOnline() const
{
    QStringList l;
    foreach (MyClient * c, _clients)
        if (c->getAutched())
            l << c->getName();
    return l;
}

bool MyServer::isNameValid(QString name) const
{
    if (name.length() > 20 || name.length() < 5)
        return false;
    QRegExp r("[A-Za-z0-9_]+");
    return r.exactMatch(name);
}

bool MyServer::isNameUsed(QString name) const
{
    for (int i = 0; i < _clients.length(); ++i)
        if (_clients.at(i)->getName() == name)
            return true;
    return false;
}

void MyServer::incomingConnection(qintptr handle)
{
    MyClient *client = new MyClient(handle, this, this);
    connect(client, SIGNAL(removeUser(MyClient*)), this, SLOT(onRemoveUser(MyClient*)));
    _clients.append(client);
}

void MyServer::onRemoveUser(MyClient *client)
{
    removeUser(client->getName());
    _clients.removeAt(_clients.indexOf(client));  
}

void MyServer::onAddUserToGui(QString name)
{
    addUser(name);
}

void MyServer::onMessageFromGui(QString message, const QStringList &users)
{
    if (users.isEmpty())
        doSendToAllServerMessage(message);
    else
        doSendServerMessageToUsers(message, users);
}
