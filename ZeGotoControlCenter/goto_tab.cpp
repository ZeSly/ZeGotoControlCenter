#include "zegotocontrolcenter.h"
#include <QMessageBox>
#include "SkyPosition.h"

QString _Dec2DMS(double d, bool h)
{
	double fract;
	double deg, min, sec;
	QString ret;

	fract = fabs(modf(d, &deg));
	fract = modf(fract * 60.0, &min);
	sec = fract * 60.0;
	if (round(sec * 100.0) == 6000.0)
	{
		sec = 0.0;
		min += 1;
	}
	if (h)
	{
		ret = QString("%1:%2:%3").arg(deg, 2, 'f', 0, '0').arg(min, 2, 'f', 0, '0').arg(sec, 2, 'f', 0, '0');
		//ret = sprintf(s, "%02.0f:%02.0f:%02.0f", deg, min, sec);
	}
	else
	{
		ret = QString::fromLatin1("%1° %2' %3''").arg(deg, 0, 'f', 0, '0').arg(min, 2, 'f', 0, '0').arg(sec, 2, 'f', 2, '0');
		//ret = sprintf(s, "%.0f&deg;%.0f'%.2f''", deg, min, sec);
	}
	return ret;
}

void ZeGotoControlCenter::on_comboBox_Catalog_currentIndexChanged()
{
	SkyPosition sp;

	if (ui.comboBox_Catalog->currentIndex() > 0)
	{
		sp.SetLocation(Longitude, Latitude, Elevation);
	}

	ui.comboBox_Object->clear();

	switch (ui.comboBox_Catalog->currentIndex())
	{
	default:
		break;

	case 1:
	{
		QStringList header = Stars.first();
		int ira = header.indexOf("RAJ2000");
		int idec = header.indexOf("DEJ2000");
		int name = header.indexOf("Name");
		int constellation = header.indexOf("Constellation");
		int letter = header.indexOf("Bayer");

		bool first = true;
		int idx = 0;
		for each (QStringList star in Stars)
		{
			if (!first)
			{
				QStringList fields = star[ira].split(' ');
				double ra = fields[0].toDouble() + fields[1].toDouble() / 60.0 + fields[2].toDouble() / 3600.0;
				fields = star[idec].split(' ');
				double dec = fields[0].toDouble() + fields[1].toDouble() / 60.0 + fields[2].toDouble() / 3600.0;

				sp.SetEquatorialCoord(ra, dec);
				double alt = sp.GetAltitude();
				if (alt >= 0)
				{
					QString label = QString("%1 - %2 %3").arg(star[name]).arg(star[letter]).arg(star[constellation]);
					ui.comboBox_Object->addItem(label, QVariant(idx));
				}
			}
			first = false;
			idx++;
		}
	}
	break;

	case 2:
	{
		QStringList header = Messier.first();
		int ira = header.indexOf("RAB2000");
		int idec = header.indexOf("DEB2000");
		int m = header.indexOf("Messier");
		int ngc = header.indexOf("Name");
		int constellation = header.indexOf("Const");
		int name = header.indexOf("Object");

		bool first = true;
		int idx = 0;
		for each (QStringList messier in Messier)
		{
			if (!first)
			{
				double ra = messier[ira].toDouble() * 24.0 / 360.0;
				double dec = messier[idec].toDouble();
				sp.SetEquatorialCoord(ra, dec);
				double alt = sp.GetAltitude();
				//if (alt >= 0)
				{
					QString label;
					if (messier[name].isEmpty())
					{
						label = QString("%1 (%2)").arg(messier[m]).arg(messier[ngc]);
					}
					else
					{
						label = QString("%1 (%2) - %3").arg(messier[m]).arg(messier[ngc]).arg(messier[name]);
					}
					ui.comboBox_Object->addItem(label, QVariant(idx));
				}
			}
			first = false;
			idx++;
		}
	}
	break;
	}
}

void ZeGotoControlCenter::on_comboBox_Object_currentIndexChanged()
{
	SkyPosition sp;
	sp.SetLocation(Longitude, Latitude, Elevation);

	unsigned int idx = ui.comboBox_Object->currentData().toInt();

	if (idx > 0)
	{
		switch (ui.comboBox_Catalog->currentIndex())
		{
		default:
			break;

		case 1:
		{
			QStringList header = Stars.first();
			int ra_idx = header.indexOf("RAJ2000");
			int dec_idx = header.indexOf("DEJ2000");
			QStringList star = Stars[idx];

			QStringList fields = star[ra_idx].split(' ');
			double _ra = fields[0].toDouble() + fields[1].toDouble() / 60.0 + fields[2].toDouble() / 3600.0;
			fields = star[dec_idx].split(' ');
			double _d = fields[0].toDouble();
			double _dec = fabs(_d) + fields[1].toDouble() / 60.0 + fields[2].toDouble() / 3600.0;
			if (_d < 0) _dec = -_dec;

			QString ra = star[ra_idx].remove(' ');
			QString dec = star[dec_idx].remove(' ');
			ui.lineEdit_GotoRA->setText(ra);
			ui.lineEdit_GotoDec->setText(dec);

			sp.SetEquatorialCoord(_ra, _dec);
			ui.lineEdit_GotoAltitude->setText(QString("%1").arg(_Dec2DMS(sp.GetAltitude(), false)));
			ui.lineEdit_GotoAzimuth->setText(QString("%1").arg(_Dec2DMS(sp.GetAzimuth(), false)));
		}
		break;

		case 2:
		{
			QStringList header = Messier.first();
			int ra_idx = header.indexOf("RAB2000");
			int dec_idx = header.indexOf("DEB2000");
			QStringList messier = Messier[idx];

			double ra = messier[ra_idx].toDouble() * 24.0 / 360.0;
			double dec = messier[dec_idx].toDouble();
			ui.lineEdit_GotoRA->setText(_Dec2DMS(ra, true));
			ui.lineEdit_GotoDec->setText(_Dec2DMS(dec, false));

			sp.SetEquatorialCoord(ra, dec);
			ui.lineEdit_GotoAltitude->setText(QString("%1").arg(_Dec2DMS(sp.GetAltitude(), false)));
			ui.lineEdit_GotoAzimuth->setText(QString("%1").arg(_Dec2DMS(sp.GetAzimuth(), false)));
		}
		break;

		}
	}

	lineEdit_GotoRA_textHasChanged = false;
	lineEdit_GotoDec_textHasChanged = false;
	lineEdit_GotoAltitude_textHasChanged = false;
	lineEdit_GotoAzimuth_textHasChanged = false;
}

void ZeGotoControlCenter::on_pushButton_Goto_clicked()
{
	QString sRA = ui.lineEdit_GotoRA->text();
	QString sDec = ui.lineEdit_GotoDec->text();

	if (QMessageBox::question(this, tr("Goto to equatoriale coordinates"), QString("The telescope will got to\n%1 %2\nAre you sure ?").arg(sRA).arg(sDec)) == QMessageBox::Yes)
	{
		QByteArray cmd_RA = QString(":Sr%1").arg(sRA.replace('s', '#')).toLocal8Bit();
		QByteArray cmd_Dec = QString(":Sd%1").arg(sDec.replace('s', '#')).toLocal8Bit();
		char *cmd = cmd_RA.data();
		link->Command(cmd);
		cmd = cmd_Dec.data();
		link->Command(cmd);
		link->Command(":MS#");
	}
}

void ZeGotoControlCenter::on_pushButton_Sync_clicked()
{
	QString sRA = ui.lineEdit_GotoRA->text();
	QString sDec = ui.lineEdit_GotoDec->text();

	if (QMessageBox::question(this, tr("Sync to equatoriale coordinates"), QString("Confirm the telescope is pointing to\n%1 %2\n").arg(sRA).arg(sDec)) == QMessageBox::Yes)
	{
		QByteArray cmd_RA = QString(":Sr%1").arg(sRA.replace('s', '#')).toLocal8Bit();
		QByteArray cmd_Dec = QString(":Sd%1").arg(sDec.replace('s', '#')).toLocal8Bit();
		char *cmd = cmd_RA.data();
		link->Command(cmd);
		cmd = cmd_Dec.data();
		link->Command(cmd);
		link->Command(":CM#");
	}
}

void ZeGotoControlCenter::GotoEd2AltAz()
{
	double h = 0;
	double m = 0;
	double s = 0;

	QStringList fields = ui.lineEdit_GotoRA->text().split(QRegExp("[^0-9]+"));
	h = fields[0].toDouble();
	if (fields.size() > 1) m = fields[1].toDouble();
	if (fields.size() > 2) s = fields[2].toDouble();
	double ra = h + m / 60.0 + s / 3600.0;

	h = 0;
	m = 0;
	s = 0;
	fields = ui.lineEdit_GotoDec->text().split(QRegExp("[^0-9]+"));
	h = fields[0].toDouble();
	if (fields.size() > 1) m = fields[1].toDouble();
	if (fields.size() > 2) s = fields[2].toDouble();
	double dec = fabs(h) + m / 60.0 + s / 3600.0;
	if (h < 0) dec = -dec;

	SkyPosition sp;
	sp.SetLocation(Longitude, Latitude, Elevation);
	sp.SetEquatorialCoord(ra, dec);
	ui.lineEdit_GotoAltitude->setText(QString("%1").arg(_Dec2DMS(sp.GetAltitude(), false)));
	ui.lineEdit_GotoAzimuth->setText(QString("%1").arg(_Dec2DMS(sp.GetAzimuth(), false)));
}

void ZeGotoControlCenter::GotoAltAz2Ed()
{
	double d = 0;
	double m = 0;
	double s = 0;

	QStringList fields = ui.lineEdit_GotoAzimuth->text().split(QRegExp("[^0-9]+"));
	d = fields[0].toDouble();
	if (fields.size() > 1) m = fields[1].toDouble();
	if (fields.size() > 2) s = fields[2].toDouble();
	double az = d + m / 60.0 + s / 3600.0;

	d = 0;
	m = 0;
	s = 0;
	fields = ui.lineEdit_GotoAltitude->text().split(QRegExp("[^0-9]+"));
	d = fields[0].toDouble();
	if (fields.size() > 1) m = fields[1].toDouble();
	if (fields.size() > 2) s = fields[2].toDouble();
	double alt = fabs(d) + m / 60.0 + s / 3600.0;
	if (d < 0) alt = -alt;

	SkyPosition sp;
	sp.SetLocation(Longitude, Latitude, Elevation);
	sp.SetAzimuthalCoord(alt, az);
	ui.lineEdit_GotoRA->setText(QString("%1").arg(_Dec2DMS(sp.GetRightAscension(), false)));
	ui.lineEdit_GotoDec->setText(QString("%1").arg(_Dec2DMS(sp.GetDeclination(), false)));
}

void ZeGotoControlCenter::on_lineEdit_GotoRA_editingFinished()
{
	if (lineEdit_GotoRA_textHasChanged)
		ui.comboBox_Catalog->setCurrentIndex(0);
	lineEdit_GotoRA_textHasChanged = false;

	GotoEd2AltAz();
}

void ZeGotoControlCenter::on_lineEdit_GotoRA_textChanged(const QString & text)
{
	lineEdit_GotoRA_textHasChanged = true;
}

void ZeGotoControlCenter::on_lineEdit_GotoDec_editingFinished()
{
	if (lineEdit_GotoDec_textHasChanged)
		ui.comboBox_Catalog->setCurrentIndex(0);
	lineEdit_GotoDec_textHasChanged = false;

	GotoEd2AltAz();
}

void ZeGotoControlCenter::on_lineEdit_GotoDec_textChanged(const QString & text)
{
	lineEdit_GotoDec_textHasChanged = true;
}

void ZeGotoControlCenter::on_lineEdit_GotoAltitude_editingFinished()
{
	if (lineEdit_GotoAltitude_textHasChanged)
		ui.comboBox_Catalog->setCurrentIndex(0);
	lineEdit_GotoAltitude_textHasChanged = false;

	GotoAltAz2Ed();
}

void ZeGotoControlCenter::on_lineEdit_GotoAltitude_textChanged(const QString & text)
{
	lineEdit_GotoAltitude_textHasChanged = true;
}

void ZeGotoControlCenter::on_lineEdit_GotoAzimuth_editingFinished()
{
	if (lineEdit_GotoAzimuth_textHasChanged)
		ui.comboBox_Catalog->setCurrentIndex(0);
	lineEdit_GotoAzimuth_textHasChanged = false;

	GotoAltAz2Ed();
}

void ZeGotoControlCenter::on_lineEdit_GotoAzimuth_textChanged(const QString & text)
{
	lineEdit_GotoAzimuth_textHasChanged = true;
}
