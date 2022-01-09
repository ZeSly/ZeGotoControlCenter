#ifndef ZEGOTOCONTROLCENTER_H
#define ZEGOTOCONTROLCENTER_H

#include <memory>

#include <QtWidgets/QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QTcpServer>
#include <QTime>
#include <QSystemTrayIcon>
#include <QSemaphore>

#include "ui_zegotocontrolcenter.h"
#include "link.h"

#ifdef linux
#include "linux.h"
#include "build_number.h"
#endif

#define ASCOM_SERVER_PORT 5085      // my EM200 serial number ;)

class ZeGotoControlCenter : public QMainWindow
{
	Q_OBJECT

public:
	explicit ZeGotoControlCenter(QWidget *parent = 0);
	~ZeGotoControlCenter();

private slots:
    void linkConnected();
    void linkDisconnected();
    void showError(QString msg);
	void linkResponse(const char *command, const char *response);

	void on_TelescopePositionTime();
	void on_PierFlipAlertTimer();

	void on_comboBox_ConnectionType_currentIndexChanged(const QString &arg1);
	void on_lineEdit_IPAddress_editingFinished();
	void on_lineEdit_IPPort_editingFinished();
	void on_checkBox_SyncDateTime_toggled(bool checked);
	void on_pushButton_Connect_clicked();
	void on_pushButton_Expand_clicked();
	void on_pushButton_StopMonitor_clicked();
	void on_pushButton_StopMonitor_toggled();
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
	void on_pushButton_PierFlipNow_clicked();
	void on_pushButton_DecreaseReticuleBrightness_clicked();
	void on_pushButton_IncreaseReticuleBrightness_clicked();
	void on_pushButton_Bootloader_clicked();
	void on_tabWidget_currentChanged(int index);

	// park_tab.cpp
	void on_pushButton_Park_clicked();
	void on_pushButton_Unpark_clicked();
	void on_comboBox_ParkPositions_currentIndexChanged();

	// goto_tab.cpp
	void on_comboBox_Catalog_currentIndexChanged();
	void on_comboBox_Object_currentIndexChanged();
	void on_comboBox_Object_editTextChanged(const QString & text);
	void on_pushButton_Goto_clicked();
	void on_pushButton_Sync_clicked();
	void on_lineEdit_GotoRA_editingFinished();
	void on_lineEdit_GotoRA_textChanged(const QString & text);
	void on_lineEdit_GotoDec_editingFinished();
	void on_lineEdit_GotoDec_textChanged(const QString & text);
	void on_lineEdit_GotoAltitude_editingFinished();
	void on_lineEdit_GotoAltitude_textChanged(const QString & text);
	void on_lineEdit_GotoAzimuth_editingFinished();
	void on_lineEdit_GotoAzimuth_textChanged(const QString & text);

	// meridian_tab
	void on_checkBox_PierFlipAlert_clicked();
	void on_timeEdit_PierFlipAlert_timeChanged(const QTime &time);
	void on_radioButton_PierFlipAutomatic_toggled(bool checked);
	void on_radioButton_PierFlipManual_toggled(bool checked);
	void on_radioButton_PierFlipPictureFolder_toggled(bool checked);
	void on_pushButton_PierFlipPictureFolder_clicked();
	void on_lineEdit_PierFlipPictureFolder_editingFinished();
	void on_comboBox_ManualSideOfPier_currentIndexChanged(int index);

	// rates_tap.cpp
	void on_comboBox_TrackingRate_currentIndexChanged(int index);
	void on_pushButton_SetSlewRate_clicked();
	void on_spinBox_SlewRate_editingFinished();
	void on_pushButton_SetCenteringRate_clicked();
	void on_spinBox_CenteringRate_editingFinished();
	void on_pushButton_SetGuideRate_clicked();
	void on_doubleSpinBox_GuideRate_editingFinished();

	// location_tab.cpp
	void on_pushButton_GPS_OnOff_clicked();
	void on_pushButton_RefreshSat_clicked();

	void ASCOMConnect();
	void ASCOMDisconnect();
	void ASCOMReadCommand();

	void on_SingleInstance();

private:
	Ui::ZeGotoControlCenterClass ui;
	QSettings settings;
	QSystemTrayIcon systrayIcon;
	QTimer TelescopePositionTimer;
	QTimer PierFlipAlertTimer;
	QSemaphore unique;

    std::unique_ptr<Link> link;
    bool TryReconnect;

	void DisplayCoord(const char *s, QLabel *label, bool deg, int geo = 0, double *value = NULL);

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

	QTime LocalSideralTime;
	QTime UniversalTime;
	QTime RightAscension;
	QDate MountDate;
	int MountUTCOffset;

	QTime ParseTime(const char * response);
	QDate ParseDate(const char * response);
	void SyncDateTimeWithSystem();
	bool Synched;

	enum T_PierSide
	{
		EAST,
		WEST
	};
	T_PierSide PierSide;

	void DecodeGPSFrame(const char *frame);
	void SetPositionFixIndicator(int p);

	QVector<QStringList> Stars;
	QVector<QStringList> Messier;
	QMap<uint, QStringList> NewGeneralCatalog;
	QMap<uint, QStringList> IndexCatalog;
	QStringList ngc_ic_header;

	void LoadCat(QString filename, QVector<QStringList> &cat);

	double Latitude;
	double Longitude;
	double Elevation;

	bool lineEdit_GotoRA_textHasChanged;
	bool lineEdit_GotoDec_textHasChanged;
	bool lineEdit_GotoAltitude_textHasChanged;
	bool lineEdit_GotoAzimuth_textHasChanged;

	void GotoEd2AltAz();
	void GotoAltAz2Ed();
};

#endif // ZEGOTOCONTROLCENTER_H
