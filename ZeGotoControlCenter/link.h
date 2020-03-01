#ifndef LINK_H
#define LINK_H

#include <memory>

#include <QtSerialPort/QSerialPort>
#include <QTcpSocket>
#include <QTimer>
#include <QList>

enum T_LinkTypeCommand
{
	LINK_TYPE_STRING,
	LINK_TYPE_BOOL,
	LINK_TYPE_BLIND,
	LINK_TYPE_GPS
};

class Link : public QObject
{
    Q_OBJECT

public:
    Link(QObject *parent = 0);
    ~Link();

    void Open(QString portName);
    void Open(QString Address, qint16 Port);

    void Connect();
    void Disconnect();

    bool IsConnected();

    void Command(const char *command);
	void CommandGPS(const char *command);
    void CommandString(const char *command);
    void CommandBool(const char *command);
    void CommandBlind(const char *command);

	bool IsCommandToSend();

Q_SIGNALS:
    void connected();
    void error(QString msg);
    void response(const char *command, const char *resp);
	void nothing_to_send();

private slots:
    void handleConnect();
    void Receive();
    void handleSocketTimeout();
    void handleSerialPortTimeout();
    void handleSerialPortError(QSerialPort::SerialPortError error);
    void handleSocketError(QAbstractSocket::SocketError err);

private:
    struct T_TypeCommand
    {
        QString Command;
        T_LinkTypeCommand Type;
    };

    static T_TypeCommand TypesCommand[];

    struct T_LastCommand
    {
        T_LinkTypeCommand Type;
        QByteArray Command;
    };

    QList<T_LastCommand> commandStack;
    QByteArray readData;
    char resp[128];
    QTimer timer;

    std::shared_ptr<QIODevice> link;
    QString AddressIP;
    qint16 PortIP;

    void Send(T_LastCommand *command);
};

#endif // SERIALLINK_H
