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
	if (h)
	{
		ret = QString("%1:%2:%3").arg(deg, 2, 'f', 0, '0').arg(min, 2, 'f', 0, '0').arg(sec, 2, 'f', 0, '0');
		//ret = sprintf(s, "%02.0f:%02.0f:%02.0f", deg, min, sec);
	}
	else
	{
		ret = QString::fromLatin1("%1° %2' %3''").arg(deg, 0, 'f', 0, '0').arg(min, 0, 'f', 0, '0').arg(sec, 2, 'f', 2, '0');
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

	if (ui.comboBox_Catalog->currentIndex() == 1)
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
}

void ZeGotoControlCenter::on_comboBox_Object_currentIndexChanged()
{
	QStringList header = Stars.first();
	int ra_idx = header.indexOf("RAJ2000");
	int dec_idx = header.indexOf("DEJ2000");

	QStringList star = Stars[ui.comboBox_Object->currentData().toInt()];

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

	SkyPosition sp;
	sp.SetLocation(Longitude, Latitude, Elevation);
	sp.SetEquatorialCoord(_ra, _dec);
	ui.lineEdit_GotoAltitude->setText(QString("%1").arg(_Dec2DMS(sp.GetAltitude(), false)));
	ui.lineEdit_GotoAzimuth->setText(QString("%1").arg(_Dec2DMS(sp.GetAzimuth(), false)));
}

void ZeGotoControlCenter::on_pushButton_Goto_clicked()
{
	QString sRA = ui.lineEdit_GotoRA->text();
	QString sDec = ui.lineEdit_GotoDec->text();

	if(QMessageBox::question(this, tr("Goto to equatoriale coordinates"), QString("The telescope will got to\n%1 %2\nAre you sure ?").arg(sRA).arg(sDec)) == QMessageBox::Yes)
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
		QByteArray cmd_RA = QString(":Sr%1").arg(sRA.replace('s','#')).toLocal8Bit();
		QByteArray cmd_Dec = QString(":Sd%1").arg(sDec.replace('s', '#')).toLocal8Bit();
		char *cmd = cmd_RA.data();
		link->Command(cmd);
		cmd = cmd_Dec.data();
		link->Command(cmd);
		link->Command(":CM#");
	}
}
