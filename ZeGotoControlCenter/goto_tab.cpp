#include "zegotocontrolcenter.h"
#include <QMessageBox>
#include <QFile>
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
	}
	else
	{
		ret = QString::fromLatin1("%1° %2' %3''").arg(deg, 0, 'f', 0, '0').arg(min, 2, 'f', 0, '0').arg(sec, 2, 'f', 2, '0');
	}
	return ret;
}

void ZeGotoControlCenter::LoadCat(QString filename, QVector<QStringList> &cat)
{
	QFile file(filename);
	if (file.open((QIODevice::ReadOnly | QIODevice::Text)))
	{
		while (!file.atEnd())
		{
			QString line = file.readLine();
			QStringList fields = line.trimmed().split('\t');
			cat << fields;
		}
		file.close();
	}
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
		ui.comboBox_Object->setEditable(false);
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
		ui.comboBox_Object->setEditable(false);
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
				if (alt >= 0)
				{
					QString label;
					if (messier.size() <= name)
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
		ui.comboBox_Object->setEditable(false);
	}
	break;
	case 3:
	{
		ui.comboBox_Object->setEditable(true);
		ui.comboBox_Object->setStyleSheet("QComboBox:editable { background: #500; }");

		int ira = ngc_ic_header.indexOf("RAJ2000");
		int idec = ngc_ic_header.indexOf("DEJ2000");

		QMapIterator<uint, QStringList> ngc(NewGeneralCatalog);
		while (ngc.hasNext() && ui.comboBox_Object->count() < 20)
		{
			ngc.next();
			QStringList o = ngc.value();
			double ra = o[ira].toDouble() * 24.0 / 360.0;
			double dec = o[idec].toDouble();
			sp.SetEquatorialCoord(ra, dec);
			double alt = sp.GetAltitude();
			if (alt >= 0)
			{
				ui.comboBox_Object->addItem(QString("NGC %1").arg(ngc.key()), QVariant(ngc.key()));
			}
		}
	}
	break;
	case 4:
	{
		ui.comboBox_Object->setEditable(true);
		ui.comboBox_Object->setStyleSheet("QComboBox:editable { background: #500; }");

		int ira = ngc_ic_header.indexOf("RAJ2000");
		int idec = ngc_ic_header.indexOf("DEJ2000");

		QMapIterator<uint, QStringList> ngc(IndexCatalog);
		while (ngc.hasNext() && ui.comboBox_Object->count() < 20)
		{
			ngc.next();
			QStringList o = ngc.value();
			double ra = o[ira].toDouble() * 24.0 / 360.0;
			double dec = o[idec].toDouble();
			sp.SetEquatorialCoord(ra, dec);
			double alt = sp.GetAltitude();
			if (alt >= 0)
			{
				ui.comboBox_Object->addItem(QString("IC %1").arg(ngc.key()), QVariant(ngc.key()));
			}
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

		case 3:
		{
			int ra_idx = ngc_ic_header.indexOf("RAJ2000");
			int dec_idx = ngc_ic_header.indexOf("DEJ2000");
			QStringList ngc = NewGeneralCatalog[(uint)idx];

			double ra = ngc[ra_idx].toDouble() * 24.0 / 360.0;
			double dec = ngc[dec_idx].toDouble();
			ui.lineEdit_GotoRA->setText(_Dec2DMS(ra, true));
			ui.lineEdit_GotoDec->setText(_Dec2DMS(dec, false));

			sp.SetEquatorialCoord(ra, dec);
			ui.lineEdit_GotoAltitude->setText(QString("%1").arg(_Dec2DMS(sp.GetAltitude(), false)));
			ui.lineEdit_GotoAzimuth->setText(QString("%1").arg(_Dec2DMS(sp.GetAzimuth(), false)));
		}
		break;
		case 4:
		{
			int ra_idx = ngc_ic_header.indexOf("RAJ2000");
			int dec_idx = ngc_ic_header.indexOf("DEJ2000");
			QStringList ic = IndexCatalog[(uint)idx];

			double ra = ic[ra_idx].toDouble() * 24.0 / 360.0;
			double dec = ic[dec_idx].toDouble();
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

void ZeGotoControlCenter::on_comboBox_Object_editTextChanged(const QString & text)
{
	SkyPosition sp;

	if (ui.comboBox_Catalog->currentIndex() > 0)
	{
		sp.SetLocation(Longitude, Latitude, Elevation);
	}

	int ira = ngc_ic_header.indexOf("RAJ2000");
	int idec = ngc_ic_header.indexOf("DEJ2000");

	QString t = text.toUpper();
	if (ui.comboBox_Object->findText(t) < 0 && unique.tryAcquire())
	{
		if (ui.comboBox_Catalog->currentIndex() == 3)
		{
			t.remove(QRegExp("NGC ?"));
			int num_ngc = t.toInt();
			int first_ngc;
			for (first_ngc = num_ngc - 10; NewGeneralCatalog.contains(first_ngc) == false; first_ngc++)
				;
			ui.comboBox_Object->clear();
			int select = 0;
			for (int ngc = first_ngc ; ui.comboBox_Object->count() < 20 && NewGeneralCatalog.contains(ngc) == true; ngc++)
			{
				double ra = NewGeneralCatalog[ngc][ira].toDouble() * 24.0 / 360.0;
				double dec = NewGeneralCatalog[ngc][idec].toDouble();
				sp.SetEquatorialCoord(ra, dec);
				double alt = sp.GetAltitude();
				if (alt >= 0)
				{
					ui.comboBox_Object->addItem(QString("NGC %1").arg(ngc), QVariant(ngc));
				}
				if (ngc == num_ngc) select = ngc;
			}
			ui.comboBox_Object->setCurrentIndex(select - first_ngc);
		}
		else if (ui.comboBox_Catalog->currentIndex() == 4)
		{
			t.remove(QRegExp("IC ?"));
			int num_ic = t.toInt();
			int first_ic;
			for (first_ic = num_ic - 10; IndexCatalog.contains(first_ic) == false; first_ic++)
				;
			ui.comboBox_Object->clear();
			int select = 0;
			for (int ic = first_ic; ui.comboBox_Object->count() < 20 && IndexCatalog.contains(ic) == true; ic++)
			{
				double ra = IndexCatalog[ic][ira].toDouble() * 24.0 / 360.0;
				double dec = IndexCatalog[ic][idec].toDouble();
				sp.SetEquatorialCoord(ra, dec);
				double alt = sp.GetAltitude();
				if (alt >= 0)
				{
					ui.comboBox_Object->addItem(QString("IC %1").arg(ic), QVariant(ic));
				}
				if (ic == num_ic) select = ic;
			}
			ui.comboBox_Object->setCurrentIndex(select - first_ic);
		}

		unique.release();
	}
}

void ZeGotoControlCenter::on_pushButton_Goto_clicked()
{
	QString sRA = ui.lineEdit_GotoRA->text();
	QString sDec = ui.lineEdit_GotoDec->text();

	if (QMessageBox::question(this, tr("Goto to equatoriale coordinates"), QString("The telescope will got to\n%1 %2\nAre you sure ?").arg(sRA).arg(sDec)) == QMessageBox::Yes)
	{
		QByteArray cmd_RA = QString(":Sr%1").arg(sRA.replace('s', '#').remove(' ')).toLocal8Bit();
		QByteArray cmd_Dec = QString(":Sd%1").arg(sDec.replace("''", "#").remove(' ')).toLocal8Bit();
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
		QByteArray cmd_RA = QString(":Sr%1").arg(sRA.replace('s', '#').remove(' ')).toLocal8Bit();
		QByteArray cmd_Dec = QString(":Sd%1").arg(sDec.replace("''", "#").remove(' ')).toLocal8Bit();
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
