#include "zegotocontrolcenter.h"


void ZeGotoControlCenter::on_comboBox_TrackingRate_currentIndexChanged(int index)
{
	static int last_index = 0;

	if (index != last_index)
	{
		last_index = index;

		switch (index)
		{
		case 0:
			on_pushButton_TrackingSideral_clicked();
			break;

		case 1:
			on_pushButton_TrackingLunar_clicked();
			break;

		case 2:
			on_pushButton_TrackingSolar_clicked();
			break;

		case 3:
			on_pushButton_TrackingCustom_clicked();
			ui.lineEdit_CustomTracking->setFocus();
			break;

		default:
			break;
		}
	}
}

void ZeGotoControlCenter::on_pushButton_SetSlewRate_clicked()
{
	char cmd[64];
	int slew_rate = ui.spinBox_SlewRate->value();

	sprintf(cmd, ":Rs%i#", slew_rate);
	if (link != NULL)
	{
		link->CommandBlind(cmd);
		link->Command(":rS#");	// SetMaxRate
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
		link->Command(":rC#");	// SetCenteringRate
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
		link->Command(":rG#");	// SetGuidingRate
	}
}

void ZeGotoControlCenter::on_doubleSpinBox_GuideRate_editingFinished()
{
	on_pushButton_SetGuideRate_clicked();
}
