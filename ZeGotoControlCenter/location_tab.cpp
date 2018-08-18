#include "zegotocontrolcenter.h"

void ZeGotoControlCenter::on_pushButton_GPS_OnOff_clicked()
{
	if (ui.pushButton_GPS_OnOff->isChecked())
	{
		// Set GPS ON
		link->Command(":g+#");
		ui.pushButton_GPS_OnOff->setText("GPS OFF");
		ui.pushButton_GPS_OnOff->setChecked(true);
	}
	else
	{
		// set GPS OFF
		link->Command(":g-#");
		ui.pushButton_GPS_OnOff->setText("GPS ON");
		ui.pushButton_GPS_OnOff->setChecked(false);
	}
}

void ZeGotoControlCenter::on_pushButton_RefreshSat_clicked()
{
	if (ui.pushButton_RefreshSat->isChecked())
	{
		TelescopePositionTimer.stop();
		link->Command(":gps#");
		ui.pushButton_RefreshSat->setChecked(true);
	}
	else
	{
		ui.pushButton_RefreshSat->setChecked(false);
		if (!ui.pushButton_StopMonitor->isChecked())
		{
			TelescopePositionTimer.start();
		}
		else
		{
			on_TelescopePositionTime();
		}
	}
}

void ZeGotoControlCenter::SetPositionFixIndicator(int PositionFixIndicator)
{
	QString txt;

	switch (PositionFixIndicator)
	{
	case 0:
		txt = "0, position fix unavailable";
		break;
	case 1:
		txt = "1, valid position fix, SPS mode";
		break;
	case 2:
		txt = "2, valid position fix, differential GPS mode";
		break;
	case 3:
		txt = "3, GPS PPS Mode, fix valid";
		break;
	case 4:
		txt = "4, Real Time Kinematic. System used in RTK mode with fixed integers";
		break;
	case 5:
		txt = "5, Float RTK. Satellite system used in RTK mode. Floating integers";
		break;
	case 6:
		txt = "6, Estimated(dead reckoning) Mode";
		break;
	case 7:
		txt = "7, Manual Input Mode";
		break;
	case 8:
		txt = "8, Simulator Mode";
		break;
	default:
		txt = "No GPS";
		break;
	}

	ui.label_GPS_FixIndicator_Value->setText(txt);
	ui.label_GPS_FixIndicator_Value->setToolTip(txt);
}

void ZeGotoControlCenter::DecodeGPSFrame(const char *f)
{
	QString frame(f);
	QStringList fields = frame.split(',');
	static QString Latitude, Longitude, Altitude;
	static int SatellitesInView = 0;
	static int SatellitesInUse = 0;

	if (fields[0] == "$GPGGA")
	{
		SetPositionFixIndicator(fields[6].toInt());
		Latitude = fields[3] + " " + fields[2];
		Longitude = fields[5] + " " + fields[4];
		Altitude = fields[9] + " " + fields[10];
		ui.label_GPS_UTC_Time_Value->setText(fields[1]);
		ui.label_GPS_Satellites_Value->setText(fields[7]);
		SatellitesInUse = fields[7].toInt();
	}
	if (fields[0] == "$GPGLL")
	{
		Latitude = fields[2] + " " + fields[1];
		Longitude = fields[4] + " " + fields[3];
		ui.label_GPS_UTC_Time_Value->setText(fields[5]);
	}
	if (fields[0] == "$GPRMC")
	{
		ui.label_GPS_UTC_Time_Value->setText(fields[1]);
		Latitude = fields[4] + " " + fields[3];
		Longitude = fields[6] + " " + fields[5];
	}
	else if (fields[0] == "$GPGSV")
	{
		int i = 2;
		int MessageNumber = fields[i++].toInt();
		int o = (MessageNumber - 1) * 4;
		SatellitesInView = fields[i++].toInt();
		ui.frame_GPSSatellites->SetSatellitesInView(SatellitesInView);
		for (int j = 0; j < 4 && j + o < SatellitesInView; j++)
		{
			Satellite sat;
			sat.Id = fields[i++];
			sat.Elevation = fields[i++].toInt();
			sat.Azimuth = fields[i++].toInt();
			sat.SNR = fields[i++].toInt();
			ui.frame_GPSSatellites->AddSatellites(sat);
		}
	}
	ui.frame_GPSSatellites->text = QString("%1 %2 %3\nIn view: %4 In use: %5").arg(Latitude).arg(Longitude).arg(Altitude).arg(SatellitesInView).arg(SatellitesInUse);
}
