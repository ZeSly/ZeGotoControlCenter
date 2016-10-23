#include "zegotocontrolcenter.h"
#include <QMessageBox>

void ZeGotoControlCenter::on_pushButton_PierFlipNow_clicked()
{
	if (QMessageBox::question(this, tr("Manul Pier Flip"), tr("Are you sure ?")) == QMessageBox::Yes)
	{
		link->CommandBlind(":pF#");
	}
}

void ZeGotoControlCenter::on_checkBox_PierFlipAlert_clicked()
{
	bool alert = ui.checkBox_PierFlipAlert->isChecked();
	ui.timeEdit_PierFlipAlert->setEnabled(alert);
	ui.label_PierFlipAlert->setEnabled(alert);

	settings.setValue("PierFlipAlert", alert);

	if (!alert)
	{
		PierFlipAlertTimer.stop();
	}
}

void ZeGotoControlCenter::on_timeEdit_PierFlipAlert_timeChanged(const QTime &time)
{
	settings.setValue("PierFlipTime", time.toString());
}

void ZeGotoControlCenter::on_PierFlipAlertTimer()
{
	QString Message = QString("Pier flip will occur in %1 minutes").arg(ui.timeEdit_PierFlipAlert->time().toString("mm:ss"));
	systrayIcon.show();
	systrayIcon.showMessage("ZeGoto Control Center", Message, QSystemTrayIcon::Warning);
}