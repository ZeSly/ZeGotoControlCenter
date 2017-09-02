#include "zegotocontrolcenter.h"

void ZeGotoControlCenter::on_comboBox_Catalog_currentIndexChanged()
{
	if (ui.comboBox_Catalog->currentIndex() == 0)
	{
		QStringList header = Stars.first();
		int name = header.indexOf("Name");

		bool first = true;
		for each (QStringList star in Stars)
		{
			if (!first)
			{
				ui.comboBox_Object->addItem(star[name]);
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
