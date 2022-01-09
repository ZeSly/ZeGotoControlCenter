//#include <Windows.h>
#include "zegotocontrolcenter.h"
#include <QMessageBox>
#include <QTcpSocket>

void ZeGotoControlCenter::startASCOMServer()
{
    if (ASCOMServer == NULL)
    {
        ASCOMServer = new QTcpServer(this);
        ASCOMConnection = NULL;
        ASCOMConnected = false;

        if (!ASCOMServer->listen(QHostAddress::Any, ASCOM_SERVER_PORT))
        {
            QMessageBox::critical(this, tr("OpenGoto Control Center"), tr("Unable to start the ASCOM server: %1.").arg(ASCOMServer->errorString()));
            return;
        }

        connect(ASCOMServer, SIGNAL(newConnection()), this, SLOT(ASCOMConnect()));
    }
}

void ZeGotoControlCenter::stopASCOMServer()
{
    delete ASCOMServer;
    ASCOMServer = NULL;
}

void ZeGotoControlCenter::ASCOMConnect()
{
    if (!ASCOMConnected)
    {
        if (!link->IsConnected())
        {
            ui.statusBar->showMessage(tr("ASCOM driver connecting..."));
            on_pushButton_Connect_clicked();
        }

        ASCOMConnection = ASCOMServer->nextPendingConnection();
        connect(ASCOMConnection, SIGNAL(disconnected()), this, SLOT(ASCOMDisconnect()));
        connect(ASCOMConnection, SIGNAL(readyRead()), this, SLOT(ASCOMReadCommand()));
        //connect(ASCOMConnection, SIGNAL(error(QAbstractSocket::SocketError)),
        ASCOMConnected = true;

        ui.statusBar->showMessage(tr("ASCOM driver connected."));
    }
}

void ZeGotoControlCenter::ASCOMDisconnect()
{
    ASCOMConnected = false;
    ASCOMConnection->deleteLater();
    ui.statusBar->showMessage(tr("ASCOM driver disconnected."));
}

void ZeGotoControlCenter::ASCOMReadCommand()
{
    QByteArray buffer;

    if (ASCOMConnected)
    {
        buffer.append(ASCOMConnection->readAll());
        while (!ui.groupBox_MoveScope->isEnabled())
            QCoreApplication::processEvents();

        if (buffer.length() == 1 && buffer[0] == (char)6)
        {
            ASCOMConnection->write("P");
        }
        if (buffer.indexOf('#') > 1 && link != NULL)
        {
            QList<QByteArray> cmd_list = buffer.split('#');
            foreach (QByteArray buf, cmd_list)
            {
                if (buf.length() > 1)
                {
                    buf += '#';
                    ASCOMLastCommand = buf;
                    qDebug("ASCOM RX %s", buf.data());
                    link->Command(buf);
                }
            }
        }
    }
}

void ZeGotoControlCenter::ASCOMResponse(const char *command, const char *response)
{
    if (ASCOMConnected)
    {
        if (strcmp(command, ASCOMLastCommand.data()) == 0)
        {
            ASCOMLastCommand.clear();
            qDebug("ASCOM TX %s", response);
            ASCOMConnection->write(response, strlen(response));
        }
    }
}