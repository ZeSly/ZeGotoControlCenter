#include "link.h"

const int Link::timeout = 10000;

Link::T_TypeCommand Link::TypesCommand[] =
{
    { ":GR", LINK_TYPE_STRING },
    { ":Gr", LINK_TYPE_STRING },
    { ":GD", LINK_TYPE_STRING },
    { ":Gd", LINK_TYPE_STRING },
    { ":GZ", LINK_TYPE_STRING },
    { ":GA", LINK_TYPE_STRING },
    { ":GS", LINK_TYPE_STRING },
    { ":GL", LINK_TYPE_STRING },
    { ":GG", LINK_TYPE_STRING },
    { ":GC", LINK_TYPE_STRING },
    { ":Gc", LINK_TYPE_STRING },
    { ":Gg", LINK_TYPE_STRING },
    { ":Gt", LINK_TYPE_STRING },
    { ":Gu", LINK_TYPE_STRING },
    { ":CM", LINK_TYPE_STRING },
    { ":GVD", LINK_TYPE_STRING },
    { ":GVN", LINK_TYPE_STRING },
    { ":GVP", LINK_TYPE_STRING },
    { ":GVT", LINK_TYPE_STRING },
    { ":Q", LINK_TYPE_BLIND },
    { ":Me", LINK_TYPE_BLIND },
    { ":Mn", LINK_TYPE_BLIND },
    { ":Ms", LINK_TYPE_BLIND },
    { ":Mw", LINK_TYPE_BLIND },
    { ":Mge", LINK_TYPE_BLIND },
    { ":Mgn", LINK_TYPE_BLIND },
    { ":Mgs", LINK_TYPE_BLIND },
    { ":Mgw", LINK_TYPE_BLIND },
    { ":MS", LINK_TYPE_STRING },
    { ":RC", LINK_TYPE_BLIND },
    { ":RG", LINK_TYPE_BLIND },
    { ":RM", LINK_TYPE_BLIND },
    { ":RS", LINK_TYPE_BLIND },
    { ":Rs", LINK_TYPE_BLIND },
    { ":Sr", LINK_TYPE_BOOL },
    { ":Sd", LINK_TYPE_BOOL },
    { ":Sa", LINK_TYPE_BOOL },
    { ":Sz", LINK_TYPE_BOOL },
    { ":SC", LINK_TYPE_BOOL },
    { ":SL", LINK_TYPE_BOOL },
    { ":SG", LINK_TYPE_BOOL },
    { ":Sg", LINK_TYPE_BOOL },
    { ":St", LINK_TYPE_BOOL },
    { ":Su", LINK_TYPE_BOOL },
    { ":hS", LINK_TYPE_BLIND },
    { ":hP", LINK_TYPE_STRING },
    { ":hW", LINK_TYPE_STRING },
    { ":PO", LINK_TYPE_BLIND },
    { ":pS", LINK_TYPE_STRING },
    { ":ps", LINK_TYPE_BLIND },
    { ":pF", LINK_TYPE_BLIND },
    { ":rG", LINK_TYPE_STRING },
    { ":rC", LINK_TYPE_STRING },
    { ":rS", LINK_TYPE_STRING },
    { ":rM", LINK_TYPE_STRING },
    { ":g+", LINK_TYPE_BLIND },
    { ":g-", LINK_TYPE_BLIND },
    { ":gps", LINK_TYPE_GPS },
    { ":B+", LINK_TYPE_BLIND },
    { ":B-", LINK_TYPE_BLIND },
    { ":ZGR", LINK_TYPE_STRING },
    { ":ZGD", LINK_TYPE_STRING },
    { ":ZGr", LINK_TYPE_STRING },
    { ":ZGd", LINK_TYPE_STRING },
    { ":U", LINK_TYPE_BLIND },
    { ":P", LINK_TYPE_STRING },
    { ":V", LINK_TYPE_STRING },
    { ":TL", LINK_TYPE_BLIND },
    { ":TQ", LINK_TYPE_BLIND },
    { ":TS", LINK_TYPE_BLIND },
    { ":TH", LINK_TYPE_BLIND },
    { ":GIP", LINK_TYPE_STRING },
    { ":SIP", LINK_TYPE_STRING },

    { "END", LINK_TYPE_BLIND}
};

Link::Link(QObject *parent) : QObject(parent), link(nullptr)
{

}

Link::~Link(void)
{
    Disconnect();
}

void Link::Open(QString portName)
{
    QSerialPort *serial = new QSerialPort(portName, this);
    serial->setBaudRate(QSerialPort::Baud115200);
    link.reset(serial);
    connect(link.get(), SIGNAL(readyRead()), SLOT(Receive()));
    m_ErrorConnection = connect(link.get(), SIGNAL(error(QSerialPort::SerialPortError)), SLOT(handleSerialPortError(QSerialPort::SerialPortError)));
    connect(&timer, SIGNAL(timeout()), SLOT(handleSerialPortTimeout()));
}

void Link::Open(QString Address, qint16 Port)
{
    AddressIP = Address;
    PortIP = Port;

    link.reset(new QTcpSocket(this));
    connect(link.get(), SIGNAL(connected()), this, SLOT(handleConnect()));
    connect(link.get(), SIGNAL(readyRead()), this, SLOT(Receive()));
    m_ErrorConnection = connect(link.get(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(handleSocketError(QAbstractSocket::SocketError)));
    connect(&timer, SIGNAL(timeout()), SLOT(handleSocketTimeout()));
}

void Link::Connect()
{
    QSerialPort *serial = dynamic_cast<QSerialPort*>(link.get());
    if (serial != nullptr)
    {
        if (serial->open(QIODevice::ReadWrite))
        {
            serial->clear();
            qDebug() << "Connected";
            emit connected();
        }
    }
    else
    {
        QTcpSocket *socket = dynamic_cast<QTcpSocket*>(link.get());
        if (socket != nullptr)
        {
            socket->abort();
            socket->connectToHost(AddressIP, PortIP);
        }
    }
}

void Link::Disconnect()
{
    commandStack.clear();
    if (link)
    {
        QSerialPort *serial = dynamic_cast<QSerialPort*>(link.get());
        QSerialPort::SerialPortError error = serial != nullptr ? serial->error() : QSerialPort::SerialPortError::NoError;
        if (error != QSerialPort::SerialPortError::ResourceError && link->isOpen())
        {
            link->waitForBytesWritten(100);
            link->close();
            qDebug() << "Disconnected";
        }
        else
        {
            disconnect(m_ErrorConnection);
            link.reset();
            emit disconnected();
        }
    }
}

bool Link::IsConnected()
{
    return link && link->isOpen();
}

void Link::handleConnect()
{
    emit connected();
}

void Link::Command(const char *command)
{
    QString Cmd(command);

    for (int i = 0; TypesCommand[i].Command.compare("END") != 0; i++)
    {
        if (Cmd.startsWith(TypesCommand[i].Command))
        {
            switch (TypesCommand[i].Type)
            {
            case LINK_TYPE_BLIND:  CommandBlind(command); break;
            case LINK_TYPE_BOOL:   CommandBool(command); break;
            case LINK_TYPE_STRING: CommandString(command); break;
            case LINK_TYPE_GPS: CommandGPS(command); break;
            }
            break;
        }
    }
}

void Link::CommandGPS(const char *command)
{
    T_LastCommand Cmd;

    Cmd.Command = QByteArray(command);
    Cmd.Type = LINK_TYPE_GPS;

    Send(&Cmd);
}

void Link::CommandString(const char *command)
{
    T_LastCommand Cmd;

    Cmd.Command = QByteArray(command);
    Cmd.Type = LINK_TYPE_STRING;

    Send(&Cmd);
}

void Link::CommandBool(const char *command)
{
    T_LastCommand Cmd;

    Cmd.Command = QByteArray(command);
    Cmd.Type = LINK_TYPE_BOOL;

    Send(&Cmd);
}

void Link::CommandBlind(const char *command)
{
    T_LastCommand Cmd;

    Cmd.Command = QByteArray(command);
    Cmd.Type = LINK_TYPE_BLIND;

    Send(&Cmd);
}

bool Link::IsCommandToSend()
{
    return !commandStack.empty();
}

void Link::Send(T_LastCommand *cmd)
{
    bool sent = false;

    if (link)
    {
        if (!commandStack.isEmpty() && commandStack.first().Type == LINK_TYPE_GPS)
        {
            commandStack.removeFirst();
        }

        if (commandStack.isEmpty())
        {
            qDebug() << "TX: " << cmd->Command;
            link->write(cmd->Command);
            if (cmd->Type != LINK_TYPE_BLIND)
            {
                timer.start(timeout);
            }
            sent = true;
        }

        if (!sent || cmd->Type != LINK_TYPE_BLIND)
        {
            commandStack.append(*cmd);
        }
    }
}

void Link::Receive()
{
    bool rxComplete = false;
    readData.append(link->readAll());

    T_LastCommand last = commandStack.first();

    if (last.Type == LINK_TYPE_STRING)
    {
        if (readData.endsWith('#'))
        {
            rxComplete = true;
        }
    }
    else if (last.Type == LINK_TYPE_BOOL)
    {
        if (readData.size() == 1 && (readData[0] == '0' || readData[0] == '1'))
        {
            rxComplete = true;
        }
    }
    else if (last.Type == LINK_TYPE_GPS)
    {
        if (readData.size() > 2 && readData.at(readData.size() - 3) == '*')
        {
            int s = readData.size();
            qDebug() << "len=" << s << "RX: " << readData.data();
            strncpy(resp, readData.data(), sizeof resp);
            readData.clear();
            if (timer.isActive()) timer.stop();
            emit response(last.Command.data(), resp);
        }
    }


    if (rxComplete)
    {
        strncpy(resp, readData.data(), sizeof resp);
        qDebug() << "RX: " << resp;
        readData.clear();
        timer.stop();

        if (last.Type != LINK_TYPE_BLIND)
        {
            emit response(last.Command.data(), resp);
        }
        commandStack.removeFirst();

        if (!commandStack.isEmpty())
        {
            T_LastCommand Cmd;
            do
            {
                Cmd = commandStack.first();
                const char *command = Cmd.Command.data();
                qDebug() << "tx: " << command;
                //serial->clear();
                link->write(command);
                if (Cmd.Type != LINK_TYPE_BLIND)
                {
                    timer.start(timeout);
                }
                else
                {
                    commandStack.removeFirst();
                }
            } while (!commandStack.isEmpty() && Cmd.Type == LINK_TYPE_BLIND);
        }
        else
        {
            emit nothing_to_send();
        }
    }
    else if (!timer.isActive())
    {
        timer.start(timeout / 2);
    }
}

void Link::handleSerialPortTimeout()
{
    QSerialPort *serial = dynamic_cast<QSerialPort*>(link.get());
    if (serial != nullptr)
    {
        emit error(tr("Reception timeout on %1").arg(serial->portName()));
    }
}

void Link::handleSocketTimeout()
{
    emit error(tr("Reception timeout on %1:%2").arg(AddressIP).arg(PortIP));
}

void Link::handleSerialPortError(QSerialPort::SerialPortError err)
{
    if (err == 0) return;

    QSerialPort *serial = dynamic_cast<QSerialPort*>(link.get());

    qDebug() << serial->error() << " " << serial->errorString();
    switch (err)
    {
    case QSerialPort::OpenError:
        emit error(tr("Failed to open %1, error: %2").arg(serial->portName()).arg(serial->errorString()));
        break;

    case QSerialPort::ReadError:
        emit error(tr("I/O error reading %1, error: %2").arg(serial->portName()).arg(serial->errorString()));
        break;

    case QSerialPort::WriteError:
        emit error(tr("I/O error writing %1, error: %2").arg(serial->portName()).arg(serial->errorString()));
        break;

    case QSerialPort::NoError:
        break;

    default:
        emit error(tr("I/O error %1, error: %2").arg(serial->portName()).arg(serial->errorString()));
        break;
    }

}

void Link::handleSocketError(QAbstractSocket::SocketError err)
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(link.get());

    qDebug() << socket->error() << " " << socket->errorString();
    //switch (err)
    //{
    //case QSerialPort::OpenError:
    //    emit error(tr("Failed to open %1:%2, error: %3").arg(Address).arg(Port).arg(socket->errorString()));
    //    break;

    //case QSerialPort::ReadError :
    //    emit error(tr("I/O error reading %1:%2, error: %3").arg(Address).arg(Port).arg(socket->errorString()));
    //    break;

    //case QSerialPort::WriteError:
    //    emit error(tr("I/O error writing %1:%2, error: %3").arg(Address).arg(Port).arg(socket->errorString()));
    //    break;

    //case QSerialPort::NoError:
    //    break;

    //default:
    emit error(tr("Failed to connect to %1:%2, error %3 %4").arg(AddressIP).arg(PortIP).arg(socket->error()).arg(socket->errorString()));
    //    break;
    //}
}
