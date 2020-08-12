#include "zegotocontrolcenter.h"
#include "firmwareupdate.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMap>
#include <QThread>
#include <QFileDialog>
#include "link.h"
#include <limits>


#ifdef WIN32
#include <Windows.h>

char *GetMyVersion()
{
	static char szResult[256] = { 0 };
	TCHAR szFullPath[256];
	DWORD dwVerInfoSize; // Size of version information block
	DWORD dwVerHnd = 0; // An 'ignored' parameter, always '0'
	UINT uVersionLen;
	VS_FIXEDFILEINFO *FixedFileInfo;

	GetModuleFileName(NULL, szFullPath, sizeof(szFullPath));
	dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if (dwVerInfoSize)
	{
	    TCHAR szGetName[256];
	    BOOL bRetCode;
		LPSTR lpstrVffInfo;
		HANDLE hMem;
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo = (LPSTR)GlobalLock(hMem);

		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);

		// Get a codepage from base_file_info_sctructure
		lstrcpy(szGetName, "\\");
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
#undef max
#endif

ZeGotoControlCenter::ZeGotoControlCenter(QWidget *parent)
    : QMainWindow(parent, Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    Latitude(std::numeric_limits<double>::max()),
    Longitude(std::numeric_limits<double>::max()),
    Elevation(std::numeric_limits<double>::max()),
    lineEdit_GotoAltitude_textHasChanged(false),
    lineEdit_GotoAzimuth_textHasChanged(false),
    lineEdit_GotoDec_textHasChanged(false),
    lineEdit_GotoRA_textHasChanged(false),
    TryReconnect(false),
	unique(1)
{

	ui.setupUi(this);
	this->setFixedSize(this->size());
	ui.statusBar->setSizeGripEnabled(false);
	systrayIcon.setIcon(QIcon(":/Images/Resources/em200_32.png"));

    link.reset(new Link(this));
	ASCOMServer = NULL;

	ui.comboBox_ConnectionType->addItem("TCP/IP", "IP");
	foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
	{
		QString portFullName = serialPortInfo.description() + " (" + serialPortInfo.portName() + ")";
		ui.comboBox_ConnectionType->addItem(portFullName, serialPortInfo.portName());
	}

	TelescopePositionTimer.setInterval(5000);
	connect(&TelescopePositionTimer, SIGNAL(timeout()), SLOT(on_TelescopePositionTime()));
	connect(&PierFlipAlertTimer, SIGNAL(timeout()), SLOT(on_PierFlipAlertTimer()));
	connect(QApplication::instance(), SIGNAL(showUp()), SLOT(on_SingleInstance()));

	setConnectedWidgetEnabled(false);
	ui.pushButton_SetParkPosition->setEnabled(false);
	ui.lineEdit_ParkPositionAlt->setEnabled(false);
	ui.lineEdit_ParkPositionAz->setEnabled(false);

#ifdef WIN32
	ui.label_OGCCVersion->setText(GetMyVersion());
#else
	ui.label_OGCCVersion->setText(QString("%1.%2").arg(VERSION).arg(BUILD_NUMBER));
#endif

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

	ui.pushButton_GPS_OnOff->setChecked(true);
	ui.pushButton_RefreshSat->setChecked(false);

	LoadCat("stars.csv", Stars);
	LoadCat("messiers.csv", Messier);

	QFile ngc_ic("ngc_ic.csv");
	if (ngc_ic.open((QIODevice::ReadOnly | QIODevice::Text)))
	{
		bool first_line = true;
		while (!ngc_ic.atEnd())
		{
			QString line = ngc_ic.readLine();
			QStringList fields = line.trimmed().split(',');
			if (first_line)
			{
				ngc_ic_header = fields;
				first_line = false;
			}
			else
			{
				uint n = fields[1].toUInt();
				if (fields[0] == "N" && !NewGeneralCatalog.contains(n))
					NewGeneralCatalog[n] = fields;
				if (fields[0] == "I" && !IndexCatalog.contains(n))
					IndexCatalog[n] = fields;
			}
		}
		ngc_ic.close();
	}

	/*QFile messier_file("messier.csv");
	if (messier_file.open((QIODevice::ReadOnly | QIODevice::Text)))
	{
		int idx_Object = 0;
		int idx_Name = 0;
		int idx_ra = 0;
		int idx_dec = 0;
		int ngc_ic_ra = ngc_ic_header.indexOf("RAJ2000");
		int ngc_ic_dec = ngc_ic_header.indexOf("DEJ2000");
		bool first_line = true;
		QStringList prec;
		while (!messier_file.atEnd())
		{
			QString line = messier_file.readLine();
			QStringList fields = line.trimmed().split(',');
			if (first_line)
			{
				first_line = false;
				idx_Object = fields.indexOf("Object");
				idx_Name = fields.indexOf("Name");
				idx_ra = fields.indexOf("RAB2000");
				idx_dec= fields.indexOf("DEB2000");
			}
			else
			{
				if (fields[idx_Name].at(0) == 'I')
				{
					uint n = fields[idx_Name].remove(0, 1).toUInt();
					fields[idx_Name] = "IC " + fields[idx_Name];
					fields[idx_ra] = IndexCatalog[n][ngc_ic_ra];
					fields[idx_dec] = IndexCatalog[n][ngc_ic_dec];
				}
				else
				{
					uint n = fields[idx_Name].toUInt();
					fields[idx_Name] = "NGC " + fields[idx_Name];
					fields[idx_ra] = NewGeneralCatalog[n][ngc_ic_ra];
					fields[idx_dec] = NewGeneralCatalog[n][ngc_ic_dec];
				}

				if (fields[idx_Object] == fields[0]) fields[idx_Object] = "";

				if (!prec.isEmpty())
				{
					if (prec[0] == fields[0])
					{
						if (prec[idx_Object] != fields[idx_Object])
						{
							if (!prec[idx_Object].isEmpty()) 
								Messier << prec;
							else
								Messier << fields;
							fields.clear();
						}
					}
					else
					{
						Messier << prec;
					}
				}
			}
			prec = fields;
		}
		Messier << prec;
		messier_file.close();
	}

	QFile wr_messiers("messiers.csv");
	if (wr_messiers.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream out(&wr_messiers);
		for each (QStringList m in Messier)
		{
			for each (QString f in m)
			{
				out << f << "\t";
			}
			out << "\n";
		}
		wr_messiers.close();
	}*/

	ui.comboBox_Catalog->addItem(tr(""));
	ui.comboBox_Catalog->addItem(tr("Stars"));
	ui.comboBox_Catalog->addItem(tr("Messier"));
	ui.comboBox_Catalog->addItem(tr("NGC - New General Catalog"));
	ui.comboBox_Catalog->addItem(tr("IC - Index Catalog"));

	startASCOMServer();
}

ZeGotoControlCenter::~ZeGotoControlCenter()
{
	TelescopePositionTimer.stop();
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
	ui.pushButton_RefreshSat->setEnabled(enable);
	ui.pushButton_StopMonitor->setEnabled(enable);
	ui.comboBox_Catalog->setEnabled(enable);
	ui.comboBox_Object->setEnabled(enable);
	ui.pushButton_Goto->setEnabled(enable);
	ui.pushButton_Sync->setEnabled(enable);
	ui.groupBox_ReticuleBrightness->setEnabled(enable);
}

void ZeGotoControlCenter::on_comboBox_ConnectionType_currentIndexChanged(const QString &arg1)
{
	if (arg1 == "TCP/IP")
	{
		ui.label_IPAddress->setEnabled(true);
		ui.lineEdit_IPAddress->setEnabled(true);
		ui.label_IPPort->setEnabled(true);
		ui.lineEdit_IPPort->setEnabled(true);
		ui.pushButton_Bootloader->setVisible(false);
	}
	else
	{
		ui.label_IPAddress->setEnabled(false);
		ui.lineEdit_IPAddress->setEnabled(false);
		ui.label_IPPort->setEnabled(false);
		ui.lineEdit_IPPort->setEnabled(false);
		ui.pushButton_Bootloader->setVisible(true);
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
    if (!TryReconnect)
    {
        ui.statusBar->clearMessage();
    }

	if (link->IsConnected())
	{
		TelescopePositionTimer.stop();
		link->Disconnect();

		setConnectedWidgetEnabled(false);

		ui.pushButton_Connect->setText(tr("Connect"));
		ui.pushButton_Connect->setIcon(QIcon(":/Images/Resources/network-disconnect.png"));
		ui.pushButton_Connect->setChecked(false);

        disconnect(link.get(), SIGNAL(connected()), this, SLOT(linkConnected()));
        disconnect(link.get(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
        disconnect(link.get(), SIGNAL(response(const char *, const char *)), this, SLOT(linkResponse(const char *, const char *)));
    }
	else
	{
		QString portName = ui.comboBox_ConnectionType->currentData().toString();
		if (portName != "IP")
		{
			TelescopePositionTimer.setInterval(5000);
			link->Open(portName);
		}
		else
		{
			TelescopePositionTimer.setInterval(1000);
			link->Open(ui.lineEdit_IPAddress->text(), ui.lineEdit_IPPort->text().toInt());
		}

		connect(link.get(), SIGNAL(connected()), this, SLOT(linkConnected()));
		connect(link.get(), SIGNAL(error(QString)), this, SLOT(showError(QString)));
		connect(link.get(), SIGNAL(response(const char *, const char *)), this, SLOT(linkResponse(const char *, const char *)));

		link->Connect();
	}
}

void ZeGotoControlCenter::linkConnected()
{
	Synched = false;
    if (TryReconnect)
    {
        ui.statusBar->showMessage(tr("Reconnected"));
        TryReconnect = false;
    }

	link->CommandBlind(":U#");       // Switch to high precision
	link->CommandString(":GVN#");    // Get Telescope Firmware Number
	link->CommandString(":GVD#");    // Get Telescope Firmware Date
	link->CommandString(":GVT#");    // Get Telescope Firmware Time
	link->CommandString(":rM#");     // Get Max Speed
	link->CommandString(":rS#");     // Get Current Max Speed
	link->CommandString(":GpH#");    // Get Home Data
	link->CommandString(":GC#");     // Get Current Date
	link->CommandString(":GG#");     // Get UTC Offset Time
	link->Command(":Gg#");	// GetCurrentSiteLongitude
	link->Command(":Gt#");	// GetCurrentSiteLatitude
	link->Command(":Gu#");	// GetCurrentSiteAltitude

	ui.pushButton_Connect->setText(tr("Disconnect"));
	ui.pushButton_Connect->setIcon(QIcon(":/Images/Resources/network-connect.png"));
	ui.pushButton_Connect->setChecked(true);
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
	TelescopePositionTimer.start();
}

void ZeGotoControlCenter::showError(QString msg)
{
	TelescopePositionTimer.stop();
    link->Disconnect();
    //if (!TryReconnect)
    {
        //QMessageBox *msgBox = new QMessageBox(this);
        //msgBox->setIcon(QMessageBox::Warning);
        //msgBox->setWindowTitle(tr("Communication error"));
        //msgBox->setText(msg);
        //msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        //msgBox->setModal(false);
        //msgBox->show();

        ui.statusBar->showMessage(msg);
    }
    qDebug() << __FUNCTION__ << ": " << msg;
    TryReconnect = true;
    link->Connect();
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
		if (side_of_pier.startsWith("West"))
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
		DisplayCoord(response, ui.label_GPS_Longitude_Value, true, 2, &Longitude);
	}
	else if (strcmp(":Gt#", command) == 0)
	{
		DisplayCoord(response, ui.label_GPS_Latitude_Value, true, 1, &Latitude);
	}
	else if (strcmp(":Gu#", command) == 0)
	{
		Elevation = atof(response);
		ui.label_GPS_Elevation_Value->setText(QString("%1 m").arg(Elevation));
	}

	else if (strcmp(":gps#", command) == 0)
	{
		DecodeGPSFrame(response);
	}

	else if (strcmp(":RS#", command) == 0)
	{
		int slew_rate = atoi(response);
	}

	else if (strcmp(":GPV#", command) == 0)
	{
		double voltage = strtod(response, NULL);
		ui.label_MainVoltageValue->setText(QString("%1 V").arg(voltage, 0, 'f', 1));
	}

    else if (strcmp(":MS#", command) == 0)
    {
        if (*response != '0')
        {
            QString msg(response + 1);
            QMessageBox::critical(this, "Goto rejected", msg.remove("#"));
        }
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

void ZeGotoControlCenter::DisplayCoord(const char *s, QLabel *label, bool deg, int geo, double *value)
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

	if (value != NULL)
	{
		*value = (double)hour + (double)minute / 60.0 + (double)second / 3600.0;
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
	link->CommandString(":GPV#");	// Get Power Voltage
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
		ui.pushButton_StopMonitor->setChecked(true);
	}
	else
	{
		TelescopePositionTimer.start();
		ui.pushButton_StopMonitor->setChecked(false);
	}
}

void ZeGotoControlCenter::on_pushButton_StopMonitor_toggled()
{
	qDebug("on_pushButton_StopMonitor_toggled  !!");
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
    if (link != NULL)
    {
        link->CommandBlind(":TH#");
    }
    ui.groupBox_Tracking->setTitle(tr("Tracking: stopped"));
}

void ZeGotoControlCenter::on_pushButton_TrackingSideral_clicked()
{
	if (link != NULL)
	{
		link->CommandBlind(":TQ#");
	}
	ui.groupBox_Tracking->setTitle(tr("Tracking: sideral"));
}

void ZeGotoControlCenter::on_pushButton_TrackingLunar_clicked()
{
	if (link != NULL)
	{
		link->CommandBlind(":TL#");
	}
	ui.groupBox_Tracking->setTitle(tr("Tracking: lunar"));
}

void ZeGotoControlCenter::on_pushButton_TrackingSolar_clicked()
{
	if (link != NULL)
	{
		link->CommandBlind(":TS#");
	}
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

void ZeGotoControlCenter::on_pushButton_DecreaseReticuleBrightness_clicked()
{
	link->CommandBlind(":B-#");
}

void ZeGotoControlCenter::on_pushButton_IncreaseReticuleBrightness_clicked()
{
	link->CommandBlind(":B+#");
}

void ZeGotoControlCenter::on_pushButton_Bootloader_clicked()
{
    if (link != NULL)
    {
        QString newFileName = QFileDialog::getOpenFileName(this, "Open Hex File", "", "Hex Files (*.hex *.ehx)");

        if (!newFileName.isEmpty())
        {
            char cmd[] = { 4, 0 };

            TelescopePositionTimer.stop();

            if (link->IsCommandToSend())
            {
                QEventLoop loop;
                connect(link.get(), SIGNAL(nothing_to_send()), &loop, SLOT(quit()));
                loop.exec();
            }

            link->CommandBlind(cmd);
            on_pushButton_Connect_clicked();


            FirmwareUpdate firmwareUpdate(newFileName, this);
            firmwareUpdate.exec();
        }
    }
}

void ZeGotoControlCenter::on_tabWidget_currentChanged(int index)
{
	if (link != NULL)
	{
		//if (ui.tabWidget->widget(index) != ui.tabLocation)
		//{
		//	if (!TelescopePositionTimer.isActive()) TelescopePositionTimer.start(1000);
		//}

		//if (ui.tabWidget->widget(index) == ui.tabLocation ||
		//	ui.tabWidget->widget(index) == ui.tabGoto)
		//{
		//	link->Command(":Gg#");	// GetCurrentSiteLongitude
		//	link->Command(":Gt#");	// GetCurrentSiteLatitude
		//	link->Command(":Gu#");	// GetCurrentSiteAltitude
		//}
		//else 
		if (ui.tabWidget->widget(index) == ui.tabRates)
		{
			link->Command(":rS#");	// SetMaxRate
			link->Command(":rC#");	// SetCenteringRate
			link->Command(":rG#");	// SetGuidingRate
		}
	}
}
