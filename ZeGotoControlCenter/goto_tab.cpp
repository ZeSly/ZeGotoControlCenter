#include "zegotocontrolcenter.h"
#include <QMessageBox>

void ZeGotoControlCenter::on_comboBox_Catalog_currentIndexChanged()
{
	if (ui.comboBox_Catalog->currentIndex() == 0)
	{
		QStringList header = Stars.first();
		int name = header.indexOf("Name");
		int constellation = header.indexOf("Constellation");
		int letter = header.indexOf("Bayer");

		bool first = true;
		for each (QStringList star in Stars)
		{
			if (!first)
			{
				QString label = QString("%1 - %2 %3").arg(star[name]).arg(star[letter]).arg(star[constellation]);
				ui.comboBox_Object->addItem(label);
			}
			first = false;
		}
	}
}

void ZeGotoControlCenter::on_comboBox_Object_currentIndexChanged()
{
	QStringList header = Stars.first();
	int ra_idx = header.indexOf("RAJ2000");
	int dec_idx = header.indexOf("DEJ2000");

	QStringList star = Stars[ui.comboBox_Object->currentIndex() + 1];
	QString ra = star[ra_idx].remove(' ');
	QString dec = star[dec_idx].remove(' ');
	ui.lineEdit_GotoRA->setText(ra);
	ui.lineEdit_GotoDec->setText(dec);
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
