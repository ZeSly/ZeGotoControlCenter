#ifndef ZEGOTOCONTROLCENTER_H
#define ZEGOTOCONTROLCENTER_H

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <QTcpServer>

#include "ui_zegotocontrolcenter.h"
#include "link.h"

#define ASCOM_SERVER_PORT 5085      // my EM200 serial number ;)

class ZeGotoControlCenter : public QMainWindow
{
	Q_OBJECT

public:
	ZeGotoControlCenter(QWidget *parent = 0);
	~ZeGotoControlCenter();

	private slots:
	void linkConnected();
	void showError(QString msg);
	void linkResponse(const char *command, const char *response);

	void on_TelescopePositionTime();

	void on_comboBox_ConnectionType_currentIndexChanged(const QString &arg1);
	void on_pushButton_Connect_clicked();
	void on_pushButton_Expand_clicked();
	void on_pushButton_Stop_clicked();
	void on_pushButton_North_pressed();
	void on_pushButton_North_released();
	void on_pushButton_West_pressed();
	void on_pushButton_West_released();
	void on_pushButton_East_pressed();
	void on_pushButton_East_released();
	void on_pushButton_South_pressed();
	void on_pushButton_South_released();
	void on_verticalSlider_Speed_valueChanged(int value);
	void on_verticalSlider_Speed_sliderReleased();
	void on_pushButton_TrackingStop_clicked();
	void on_pushButton_TrackingSideral_clicked();
	void on_pushButton_TrackingLunar_clicked();
	void on_pushButton_TrackingSolar_clicked();
	void on_pushButton_TrackingCustom_clicked();

	// park_tab.cpp
	void on_pushButton_Park_clicked();
	void on_pushButton_Unpark_clicked();
	void on_comboBox_ParkPositions_currentIndexChanged();

	void ASCOMConnect();
	void ASCOMDisconnect();
	void ASCOMReadCommand();

	void on_SingleInstance();

private:
	Ui::ZeGotoControlCenterClass ui;
	Link *link;
	QTimer TelescopePositionTimer;

	void DisplayCoord(const char *s, QLabel *label, bool deg);

	// TCP/IP server for ASCOM driver
	QTcpServer *ASCOMServer;
	QTcpSocket *ASCOMConnection;
	QByteArray ASCOMLastCommand;
	bool ASCOMConnected;
	void startASCOMServer();
	void stopASCOMServer();
	void ASCOMResponse(const char *command, const char *response);

	void setConnectedWidgetEnabled(bool);

	void setComboParkPosition(const char *response);

	void closeEvent(QCloseEvent *event);
};

#endif // ZEGOTOCONTROLCENTER_H
