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

	link = NULL;
	ASCOMServer = NULL;

	ui.comboBox_ConnectionType->addItem("TCP/IP", "IP");
	foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
	{
		QString portFullName = serialPortInfo.description() + " (" + serialPortInfo.portName() + ")";
		ui.comboBox_ConnectionType->addItem(portFullName, serialPortInfo.portName());
	}

	connect(&TelescopePositionTimer, SIGNAL(timeout()), SLOT(on_TelescopePositionTime()));
	connect(QApplication::instance(), SIGNAL(showUp()), SLOT(on_SingleInstance()));

	setConnectedWidgetEnabled(false);
	ui.pushButton_SetParkPosition->setEnabled(false);
	ui.lineEdit_ParkPositionAlt->setEnabled(false);
	ui.lineEdit_ParkPositionAz->setEnabled(false);

	ui.label_OGCCVersion->setText(GetMyVersion());

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
	link->CommandBlind(":U#");       // Switch to high precision
	link->CommandString(":GVN#");    // Get Telescope Firmware Number
	link->CommandString(":GVD#");    // Get Telescope Firmware Date
	link->CommandString(":GVT#");    // Get Telescope Firmware Time
	link->CommandString(":rM#");     // Get Max Speed
	link->CommandString(":rS#");     // Get Current Max Speed
	link->CommandString(":GpH#");    // Get Home Data

	ui.pushButton_Connect->setText(tr("Disconnect"));
	ui.pushButton_Connect->setIcon(QIcon(":/Images/Resources/network-connect.png"));
	setConnectedWidgetEnabled(true);
	ui.groupBox_Tracking->setTitle(tr("Tracking: sideral"));

	on_TelescopePositionTime();
	TelescopePositionTimer.start(1000);
}

void ZeGotoControlCenter::showError(QString msg)
{
	QMessageBox::critical(this, tr("Communication error"), msg);
	TelescopePositionTimer.stop();
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
		LocalSideralTime = ParseResponse(response);
		DisplayCoord(response, ui.label_LSTValue, false);
	}
	else if (strcmp(":GR#", command) == 0)
	{
		RightAscension = ParseResponse(response);
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
		UniversalTime = ParseResponse(response);
		DisplayCoord(response, ui.label_UTCValue, false);
	}
	else if (strcmp(":rM#", command) == 0)
	{
		int max_speed = atoi(response);
		ui.verticalSlider_Speed->setMaximum(max_speed);
		ui.verticalSlider_Speed->setPageStep(max_speed / 10);
	}
	else if (strcmp(":rS#", command) == 0)
	{
		int current_speed = atoi(response);
		ui.verticalSlider_Speed->setValue(current_speed);
		ui.lineEdit_Speed->setText(QString("%1").arg(current_speed));
	}
	else if (strcmp(":pS#", command) == 0)
	{
		ui.label_PierSideValue->setText(QString(response).remove('#'));
	}
	else if (strcmp(":GpH#", command) == 0)
	{
		setComboParkPosition(response);
	}

	if (ui.tabWidget->currentWidget() == ui.tabMeridian)
	{
		int flip_in_secs = RightAscension.secsTo(LocalSideralTime);
		QTime flip_in = QTime::fromMSecsSinceStartOfDay(flip_in_secs * 1000);

		ui.label_FlipInValue->setText(flip_in.toString("h 'h' mm 'm' ss 's'"));
	}
}

void ZeGotoControlCenter::DisplayCoord(const char *s, QLabel *label, bool deg)
{
	QString r(s);

	r = r.remove('#').replace("*", ":").replace("'", ":");
	QStringList l = r.split(':');
	int hour = l[0].toInt();
	int minute = l[1].toInt();
	int second = l[2].toInt();

	if (deg)
	{
		label->setText(QString::fromLatin1("%1%2° %3'' %4'").
			arg(hour >= 0 ? "+" : "").
			arg(hour).
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

QTime ZeGotoControlCenter::ParseResponse(const char * response)
{
	QString r(response);

	r = r.remove('#').replace("*", ":").replace("'", ":");
	QStringList l = r.split(':');
	int hour = l[0].toInt();
	int minute = l[1].toInt();
	int second = l[2].toInt();

	return QTime(hour, minute, second);
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
