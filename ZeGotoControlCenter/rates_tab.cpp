#include "zegotocontrolcenter.h"

void ZeGotoControlCenter::on_pushButton_SetSlewRate_clicked()
{
	char cmd[64];
	int slew_rate = ui.spinBox_SlewRate->value();

	sprintf(cmd, ":Rs%i#", slew_rate);
	if (link != NULL)
	{
		link->CommandBlind(cmd);
	}
}

void ZeGotoControlCenter::on_spinBox_SlewRate_editingFinished()
{
	on_pushButton_SetSlewRate_clicked();
}

void ZeGotoControlCenter::on_pushButton_SetCenteringRate_clicked()
{
	char cmd[64];
	int centering_rate = ui.spinBox_CenteringRate->value();

	sprintf(cmd, ":RC%i#", centering_rate);
	if (link != NULL)
	{
		link->CommandBlind(cmd);
	}
}

void ZeGotoControlCenter::on_spinBox_CenteringRate_editingFinished()
{
	on_pushButton_SetCenteringRate_clicked();
}

void ZeGotoControlCenter::on_pushButton_SetGuideRate_clicked()
{
	char cmd[64];
	int guide_rate = (int)(ui.doubleSpinBox_GuideRate->value() * 10.0);

	sprintf(cmd, ":RG%i#", guide_rate);
	if (link != NULL)
	{
		link->CommandBlind(cmd);
	}
}

void ZeGotoControlCenter::on_doubleSpinBox_GuideRate_editingFinished()
{
	on_pushButton_SetGuideRate_clicked();
}
