#include "zegotocontrolcenter.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include "link.h"

#include <Windows.h>

char *GetMyVersion()
{
	static char szResult[256] = { 0 };
	TCHAR szFullPath[256];
	TCHAR szGetName[256];
	DWORD dwVerInfoSize; // Size of version information block
	DWORD dwVerHnd = 0; // An 'ignored' parameter, always '0'
	UINT uVersionLen;
	BOOL bRetCode;
	VS_FIXEDFILEINFO *FixedFileInfo;

	GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
	dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if (dwVerInfoSize)
	{
		LPSTR lpstrVffInfo;
		HANDLE hMem;
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo = (LPSTR)GlobalLock(hMem);

		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);

		// Get a codepage from base_file_info_sctructure
		lstrcpy(szGetName, L"\\");
		uVersionLen = 0;

		bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
			szGetName,
			(void **)&FixedFileInfo,
			(UINT *)& uVersionLen);

		if (bRetCode && uVersionLen)
		{
			sprintf(szResult, "%u.%u.%u.%u",
				FixedFileInfo->dwFileVersionMS >> 16 & 0xFFFF,
				FixedFileInfo->dwFileVersionMS & 0xFFFF,
				FixedFileInfo->dwFileVersionLS >> 16 & 0xFFFF,
				FixedFileInfo->dwFileVersionLS & 0xFFFF);
		}
		else
		{
			szResult[0] = '\0';
		}

		GlobalFree(hMem);
	}

	return szResult;

}

ZeGotoControlCenter::ZeGotoControlCenter(QWidget *parent)
	: QMainWindow(parent, Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	ui.setupUi(this);
	this->setFixedSize(this->size());
	ui.statusBar->setSizeGripEnabled(false);
	systrayIcon.setIcon(QIcon(":/Images/Resources/em200_32.png"));

	link = NULL;
	ASCOMServer = NULL;

	ui.comboBox_ConnectionType->addItem("TCP/IP", "IP");
	foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
	{
		QString portFullName = serialPortInfo.description() + " (" + serialPortInfo.portName() + ")";
		ui.comboBox_ConnectionType->addItem(portFullName, serialPortInfo.portName());
	}

	connect(&TelescopePositionTimer, SIGNAL(timeout()), SLOT(on_TelescopePositionTime()));
	connect(&PierFlipAlertTimer, SIGNAL(timeout()), SLOT(on_PierFlipAlertTimer()));
	connect(QApplication::instance(), SIGNAL(showUp()), SLOT(on_SingleInstance()));

	setConnectedWidgetEnabled(false);
	ui.pushButton_SetParkPosition->setEnabled(false);
	ui.lineEdit_ParkPositionAlt->setEnabled(false);
	ui.lineEdit_ParkPositionAz->setEnabled(false);

	ui.label_OGCCVersion->setText(GetMyVersion());

	ui.lineEdit_IPAddress->setText(settings.value("IPAddress").toString());
	ui.lineEdit_IPPort->setText(settings.value("IPPort").toString());
	QVariant ConnectionType = settings.value("ConnectionType");
	int s = ui.comboBox_ConnectionType->findData(ConnectionType);
	ui.comboBox_ConnectionType->setCurrentIndex(s);
	ui.checkBox_SyncDateTime->setChecked(settings.value("SyncDateTimeWithSystem").toBool());

	ui.comboBox_TrackingRate->addItem(tr("Sideral"));
	ui.comboBox_TrackingRate->addItem(tr("Lunar"));
	ui.comboBox_TrackingRate->addItem(tr("Solar"));
	ui.comboBox_TrackingRate->addItem(tr("Custom"));

	bool alert = settings.value("PierFlipAlert").toBool();
	ui.checkBox_PierFlipAlert->setChecked(alert);
	ui.timeEdit_PierFlipAlert->setEnabled(alert);
	ui.label_PierFlipAlert->setEnabled(alert);

	QTime t = settings.value("PierFlipTime").toTime();
	ui.timeEdit_PierFlipAlert->setTime(t);

	switch (settings.value("PierFlipMode").toInt())
	{
	default :
	case 1 :
		ui.radioButton_PierFlipAutomatic->setChecked(true);
		break;
	case 2 :
		ui.radioButton_PierFlipManual->setChecked(true);
		break;
	case 3 :
		ui.radioButton_PierFlipPictureFolder->setChecked(true);
		break;
	}
	ui.lineEdit_PierFlipPictureFolder->setText(settings.value("PierFlipPictureFolder").toString());
	ui.comboBox_ManualSideOfPier->addItem(tr("East"));
	ui.comboBox_ManualSideOfPier->addItem(tr("West"));

	ui.pushButton_GPS_OnOff->setDown(true);

	startASCOMServer();
}

ZeGotoControlCenter::~ZeGotoControlCenter()
{
	TelescopePositionTimer.stop();
	if (link != NULL) delete link;
}

void ZeGotoControlCenter::closeEvent(QCloseEvent *event)
{
	if (ASCOMConnected)
	{
		if (QMessageBox::question(this, tr("ASCOM still connected"), "ASCOM driver is still connected\nAre you sure ?") == QMessageBox::No)
		{
			event->ignore();
		}
		else
		{
			event->accept();
		}
	}
}

void ZeGotoControlCenter::on_SingleInstance()
{
	raise();
	show();
	activateWindow();
}

void ZeGotoControlCenter::setConnectedWidgetEnabled(bool enable)
{
	ui.groupBox_MoveScope->setEnabled(enable);
	ui.groupBox_Speed->setEnabled(enable);
	ui.groupBox_Tracking->setEnabled(enable);
	ui.groupBox_ParkActions->setEnabled(enable);
	ui.comboBox_ParkPositions->setEnabled(enable);
	ui.pushButton_PierFlipNow->setEnabled(enable && ui.radioButton_PierFlipManual->isChecked());
	ui.comboBox_ManualSideOfPier->setEnabled(enable && ui.radioButton_PierFlipManual->isChecked());
	ui.pushButton_GPS_OnOff->setEnabled(enable);
	ui.pushButton_StopMonitor->setEnabled(enable);
}

void ZeGotoControlCenter::on_comboBox_ConnectionType_currentIndexChanged(const QString &arg1)
{
	if (arg1 == "TCP/IP")
	{
		ui.label_IPAddress->setEnabled(true);
		ui.lineEdit_IPAddress->setEnabled(true);
		ui.label_IPPort->setEnabled(true);
		ui.lineEdit_IPPort->setEnabled(true);
	}
	else
	{
		ui.label_IPAddress->setEnabled(false);
		ui.lineEdit_IPAddress->setEnabled(false);
		ui.label_IPPort->setEnabled(false);
		ui.lineEdit_IPPort->setEnabled(false);
	}
	
	if (ASCOMServer != NULL)
	{
		settings.setValue("ConnectionType", ui.comboBox_ConnectionType->currentData().toString());
	}
}

void ZeGotoControlCenter::on_lineEdit_IPAddress_editingFinished()
{
	settings.setValue("IPAddress", ui.lineEdit_IPAddress->text());
}

void ZeGotoControlCenter::on_lineEdit_IPPort_editingFinished()
{
	settings.setValue("IPPort", ui.lineEdit_IPPort->text());
}

void ZeGotoControlCenter::on_checkBox_SyncDateTime_toggled(bool checked)
{
	settings.setValue("SyncDateTimeWithSystem", checked);
}

void ZeGotoControlCenter::on_pushButton_Connect_clicked()
{
	if (link != NULL)
	{
		TelescopePositionTimer.stop();
		delete link;
		link = NULL;

		setConnectedWidgetEnabled(false);

		ui.pushButton_Connect->setText(tr("Connect"));
		ui.pushButton_Connect->setIcon(QIcon(":/Images/Resources/network-disconnect.png"));
		ui.pushButton_Connect->setDown(false);
	}
	else
	{
		QString portName = ui.comboBox_ConnectionType->currentData().toString();
		if (portName != "IP")
		{
			link = new Link(portName);
		}
		else
		{
			link = new Link(ui.lineEdit_IPAddress->text(), ui.lineEdit_IPPort->text().toInt());
		}

		connect(link, SIGNAL(connected()), this, SLOT(linkConnected()));
		connect(link, SIGNAL(error(QString)), this, SLOT(showError(QString)));
		connect(link, SIGNAL(response(const char *, const char *)), this, SLOT(linkResponse(const char *, const char *)));

		link->Connect();
	}
}

void ZeGotoControlCenter::linkConnected()
{
	Synched = false;

	link->CommandBlind(":U#");       // Switch to high precision
	link->CommandString(":GVN#");    // Get Telescope Firmware Number
	link->CommandString(":GVD#");    // Get Telescope Firmware Date
	link->CommandString(":GVT#");    // Get Telescope Firmware Time
	link->CommandString(":rM#");     // Get Max Speed
	link->CommandString(":rS#");     // Get Current Max Speed
	link->CommandString(":GpH#");    // Get Home Data
	link->CommandString(":GC#");     // Get Current Date
	link->CommandString(":GG#");     // Get UTC Offset Time

	ui.pushButton_Connect->setText(tr("Disconnect"));
	ui.pushButton_Connect->setIcon(QIcon(":/Images/Resources/network-connect.png"));
	ui.pushButton_Connect->setDown(true);
	setConnectedWidgetEnabled(true);
	ui.groupBox_Tracking->setTitle(tr("Tracking: sideral"));
	ui.tabWidget->setCurrentIndex(0);

	if (ui.radioButton_PierFlipAutomatic->isChecked())
	{
		link->Command(":psA#");
	}
	else
	{
		if (PierSide == EAST)
		{
			link->Command(":psE#");
		}
		else
		{
			link->Command(":psW#");
		}
	}

	on_TelescopePositionTime();
	TelescopePositionTimer.start(1000);
}

void ZeGotoControlCenter::showError(QString msg)
{
	QMessageBox::critical(this, tr("Communication error"), msg);
	TelescopePositionTimer.stop();
	on_pushButton_Connect_clicked();
	//delete link;
	//link = NULL;
}

void ZeGotoControlCenter::linkResponse(const char *command, const char *response)
{
	ASCOMResponse(command, response);

	if (strcmp(":GVN#", command) == 0)
	{
		ui.label_FirmwareVersionValue->setText(QString(response).remove('#'));
	}

	else if (strcmp(":GVD#", command) == 0)
	{
		ui.label_BuildDateValue->setText(QString(response).remove('#'));
	}
	else if (strcmp(":GVT#", command) == 0)
	{
		ui.label_BuildDateValue->setText(ui.label_BuildDateValue->text() + " " + QString(response).remove('#'));
	}

	else if (strcmp(":GS#", command) == 0)
	{
		LocalSideralTime = ParseTime(response);
		DisplayCoord(response, ui.label_LSTValue, false);
	}
	else if (strcmp(":GR#", command) == 0)
	{
		RightAscension = ParseTime(response);
		double ra = (double)(LocalSideralTime.msecsSinceStartOfDay() - RightAscension.msecsSinceStartOfDay()) / 3600000.0;

		if (ra < 0) ra += 24.0;
		if (ra > 18.0)
		{
			ui.label_SideOfPierValue->setText(tr("The scope is on WEST side of mount,\nSide of pier is WEST"));
		}
		else if (ra > 12)
		{
			ui.label_SideOfPierValue->setText(tr("The scope is on EAST side of mount,\nSide of pier is WEST"));
		}
		else if (ra > 6)
		{
			ui.label_SideOfPierValue->setText(tr("The scope is on WEST side of mount,\nSide of pier is EAST"));
		}
		else
		{
			ui.label_SideOfPierValue->setText(tr("The scope is on EAST side of mount,\nSide of pier is EAST"));
		}

		DisplayCoord(response, ui.label_RAValue, false);
	}
	else if (strcmp(":GD#", command) == 0)
	{
		DisplayCoord(response, ui.label_DECValue, true);
	}
	else if (strcmp(":GA#", command) == 0)
	{
		DisplayCoord(response, ui.label_ALTValue, true);
	}
	else if (strcmp(":GZ#", command) == 0)
	{
		DisplayCoord(response, ui.label_AZValue, true);
	}
	else if (strcmp(":GL#", command) == 0)
	{
		UniversalTime = ParseTime(response);
		DisplayCoord(response, ui.label_UTCValue, false);
		if (ui.checkBox_SyncDateTime->isChecked() && !Synched)
		{
			SyncDateTimeWithSystem();
		}
	}
	else if (strcmp(":GC#", command) == 0)
	{
		MountDate = ParseDate(response);
	}
	else if (strcmp(":GG#", command) == 0)
	{
		MountUTCOffset = atoi(response);
	}
	else if (strcmp(":rM#", command) == 0)
	{
		int max_speed = atoi(response);
		ui.verticalSlider_Speed->setMaximum(max_speed);
		ui.verticalSlider_Speed->setPageStep(max_speed / 10);
		ui.spinBox_SlewRate->setMaximum(max_speed);
		ui.spinBox_CenteringRate->setMaximum(max_speed);
	}
	else if (strcmp(":rS#", command) == 0)
	{
		int current_speed = atoi(response);
		ui.verticalSlider_Speed->setValue(current_speed);
		ui.lineEdit_Speed->setText(QString("%1").arg(current_speed));
		ui.spinBox_SlewRate->setValue(current_speed);
	}
	else if (strcmp(":rC#", command) == 0)
	{
		int centering_speed = atoi(response);
		ui.spinBox_CenteringRate->setValue(centering_speed);
	}
	else if (strcmp(":rG#", command) == 0)
	{
		double guide_speed = (double)atoi(response) / 10;
		ui.doubleSpinBox_GuideRate->setValue(guide_speed);
	}
	else if (strcmp(":pS#", command) == 0)
	{
		QString side_of_pier = QString(response).remove('#');
		ui.label_PierSideValue->setText(side_of_pier);
		if (side_of_pier == "West")
		{
			PierSide = WEST;
			ui.comboBox_ManualSideOfPier->setCurrentIndex(1);
		}
		else
		{
			PierSide = EAST;
			ui.comboBox_ManualSideOfPier->setCurrentIndex(0);
		}
	}
	else if (strcmp(":GpH#", command) == 0)
	{
		setComboParkPosition(response);
	}
	else if (strcmp(":Gg#", command) == 0)
	{
		DisplayCoord(response, ui.label_GPS_Longitude_Value, true, 2);
	}
	else if (strcmp(":Gt#", command) == 0)
	{
		DisplayCoord(response, ui.label_GPS_Latitude_Value, true, 1);
	}
	else if (strcmp(":Gu#", command) == 0)
	{
		int elevation = atoi(response);
		ui.label_GPS_Elevation_Value->setText(QString("%1 m").arg(elevation));
	}

	else if (strcmp(":RS#", command) == 0)
	{
		int slew_rate = atoi(response);
	}

	int flip_in_secs;
	if (PierSide == EAST)
	{
		flip_in_secs = LocalSideralTime.addSecs(43200).secsTo(RightAscension);
	}
	else
	{
		flip_in_secs = LocalSideralTime.secsTo(RightAscension);
	}
	
	if (flip_in_secs < 0) flip_in_secs += 86400;
	QTime flip_in = QTime::fromMSecsSinceStartOfDay(flip_in_secs * 1000);

	ui.label_FlipInValue->setText(flip_in.toString("h 'h' mm 'm' ss 's'"));

	if (ui.checkBox_PierFlipAlert->isChecked() && !PierFlipAlertTimer.isActive() && flip_in_secs != 0)
	{
		int flip_alert = flip_in.msecsSinceStartOfDay() - ui.timeEdit_PierFlipAlert->time().msecsSinceStartOfDay();
		QTime t = QTime::fromMSecsSinceStartOfDay(flip_alert);
		PierFlipAlertTimer.start(flip_alert);
	}
}

void ZeGotoControlCenter::DisplayCoord(const char *s, QLabel *label, bool deg, int geo)
{
	QString r(s);

	r = r.remove('#').replace("*", ":").replace("'", ":");
	QStringList l = r.split(':');
	int hour = l[0].toInt();
	int minute = l[1].toInt();
	int second = l[2].toInt();

	if (deg)
	{
		QString positive, negative;

		switch (geo)
		{
		case 0:
			positive = "+";
			negative = "-";
			break;
		case 1:
			positive = tr("N ");
			negative = tr("S ");
			break;
		case 2 :
			positive = tr("W ");
			negative = tr("E ");
		default:
			break;
		}

		label->setText(QString::fromLatin1("%1%2° %3'' %4'").
			arg(hour >= 0 ? positive : negative).
			arg(abs(hour)).
			arg(minute, 2, 10, QChar('0')).
			arg(second, 2, 10, QChar('0')));
	}
	else
	{
		label->setText(QString("%1h %2m %3s").
			arg(hour).
			arg(minute, 2, 10, QChar('0')).
			arg(second, 2, 10, QChar('0')));
	}
}

QTime ZeGotoControlCenter::ParseTime(const char * response)
{
	QString r(response);

	r = r.remove('#').replace("*", ":").replace("'", ":");
	QStringList l = r.split(':');
	int hour = l[0].toInt();
	int minute = l[1].toInt();
	int second = l[2].toInt();

	return QTime(hour, minute, second);
}

QDate ZeGotoControlCenter::ParseDate(const char * response)
{
	QString r(response);

	r = r.remove('#');
	QStringList l = r.split('/');
	int month = l[0].toInt();
	int day = l[1].toInt();
	int year = l[2].toInt() + 2000;

	return QDate(year, month, day);
}

void ZeGotoControlCenter::on_TelescopePositionTime()
{
	link->CommandString(":GS#"); // Get the Sidereal Time
	link->CommandString(":GR#"); // Get RA
	link->CommandString(":GD#"); // Get Dec
	link->CommandString(":GA#"); // Get Altitude
	link->CommandString(":GZ#"); // Get Azimuth
	link->CommandString(":GL#"); // Get Local Time
	link->CommandString(":pS#"); // Get Pier Side

}

void ZeGotoControlCenter::on_pushButton_Expand_clicked()
{
	if (ui.tabWidget->isVisible())
	{
		int w = ui.centralWidget->layout()->spacing() / 2;
		w += ui.tabWidget->x();
		int h = this->height();
		ui.tabWidget->setVisible(false);
		this->setFixedSize(w, h);
		ui.pushButton_Expand->setText(">>>");
	}
	else
	{
		int h = this->height();
		ui.tabWidget->setVisible(true);
		this->setFixedSize(800, h);
		ui.pushButton_Expand->setText("<<<");
	}
}

void ZeGotoControlCenter::on_pushButton_StopMonitor_clicked()
{
	if (ui.pushButton_StopMonitor->isChecked())
	{
		TelescopePositionTimer.stop();
		ui.pushButton_StopMonitor->setDown(true);
	}
	else
	{
		TelescopePositionTimer.start(1000);
		ui.pushButton_StopMonitor->setDown(false);
	}
}

void ZeGotoControlCenter::on_pushButton_Stop_clicked()
{
	link->CommandBlind(":Q#");
}

void ZeGotoControlCenter::on_pushButton_North_pressed()
{
	link->CommandBlind(":Mn#");
}

void ZeGotoControlCenter::on_pushButton_North_released()
{
	link->CommandBlind(":Qn#");
}

void ZeGotoControlCenter::on_pushButton_South_pressed()
{
	link->CommandBlind(":Ms#");
}

void ZeGotoControlCenter::on_pushButton_South_released()
{
	link->CommandBlind(":Qs#");
}

void ZeGotoControlCenter::on_pushButton_West_pressed()
{
	link->CommandBlind(":Mw#");
}

void ZeGotoControlCenter::on_pushButton_West_released()
{
	link->CommandBlind(":Qw#");
}

void ZeGotoControlCenter::on_pushButton_East_pressed()
{
	link->CommandBlind(":Me#");
}

void ZeGotoControlCenter::on_pushButton_East_released()
{
	link->CommandBlind(":Qe#");
}

void ZeGotoControlCenter::on_verticalSlider_Speed_valueChanged(int value)
{
	ui.lineEdit_Speed->setText(QString("%1").arg(value));
}

void ZeGotoControlCenter::on_verticalSlider_Speed_sliderReleased()
{
	char cmd[64];

	sprintf(cmd, ":Rs%i#", ui.verticalSlider_Speed->value());
	if (link != NULL)
	{
		link->CommandBlind(cmd);
	}
}

void ZeGotoControlCenter::on_pushButton_TrackingStop_clicked()
{
	ui.groupBox_Tracking->setTitle(tr("Tracking: stopped"));
}

void ZeGotoControlCenter::on_pushButton_TrackingSideral_clicked()
{
	link->CommandBlind(":TQ#");
	ui.groupBox_Tracking->setTitle(tr("Tracking: sideral"));
}

void ZeGotoControlCenter::on_pushButton_TrackingLunar_clicked()
{
	link->CommandBlind(":TL#");
	ui.groupBox_Tracking->setTitle(tr("Tracking: lunar"));
}

void ZeGotoControlCenter::on_pushButton_TrackingSolar_clicked()
{
	link->CommandBlind(":TS#");
	ui.groupBox_Tracking->setTitle(tr("Tracking: solar"));
}

void ZeGotoControlCenter::on_pushButton_TrackingCustom_clicked()
{
	ui.groupBox_Tracking->setTitle(tr("Tracking: custom"));
}


void ZeGotoControlCenter::SyncDateTimeWithSystem()
{
	int utcOffset = QDateTime::currentDateTime().utcOffset() / 3600;
	if (MountUTCOffset != utcOffset)
	{
		char cmd[16];

		sprintf(cmd, ":SG%c%02d.0#", utcOffset > 0 ? '+' : '-', utcOffset);
		link->CommandBool(cmd);
		link->CommandString(":GG#");
		qDebug("Sync UTC");
	}
	else
	{
		QDateTime now = QDateTime::currentDateTimeUtc();
		if (MountDate != now.date())
		{
			QDate today = now.toLocalTime().date();
			char cmd[16];

			sprintf(cmd, ":SC%02d:%02d:%02d#", today.month(), today.day(), today.year() % 100);
			link->CommandBool(cmd);
			qDebug("Sync Date");
		}

		QTime time = now.time();
		int s = UniversalTime.secsTo(time);
		if (abs(s) > 60)
		{
			QTime time = now.toLocalTime().time();
			char cmd[16];

			sprintf(cmd, ":SL%02d:%02d:%02d#", time.hour(), time.minute(), time.second());
			link->CommandBool(cmd);
			qDebug("Sync Time");
		}

		Synched = true;
	}
}

void ZeGotoControlCenter::on_tabWidget_currentChanged(int index)
{
	if (ui.tabWidget->widget(index) == ui.tabLocation)
	{
		if (link != NULL)
		{
			link->Command(":Gg#");	// GetCurrentSiteLongitude
			link->Command(":Gt#");	// GetCurrentSiteLatitude
			link->Command(":Gu#");	// GetCurrentSiteAltitude
		}
	}
	else if (ui.tabWidget->widget(index) == ui.tabRates)
	{
		if (link != NULL)
		{
			link->Command(":rS#");	// SetMaxRate
			link->Command(":rC#");	// SetCenteringRate
			link->Command(":rG#");	// SetGuidingRate
		}
	}
}
