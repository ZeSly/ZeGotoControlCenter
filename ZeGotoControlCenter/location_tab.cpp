#include "zegotocontrolcenter.h"

void ZeGotoControlCenter::on_pushButton_GPS_OnOff_clicked()
{
	if (ui.pushButton_GPS_OnOff->isChecked())
	{
		// Set GPS ON
		link->Command(":g+#");
		ui.pushButton_GPS_OnOff->setText("GPS OFF");
		ui.pushButton_GPS_OnOff->setDown(true);
	}
	else
	{
		// set GPS OFF
		link->Command(":g-#");
		ui.pushButton_GPS_OnOff->setText("GPS ON");
		ui.pushButton_GPS_OnOff->setDown(false);
	}
}
