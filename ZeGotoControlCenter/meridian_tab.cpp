#include "zegotocontrolcenter.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

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
	QString Message = QString(tr("Pier flip will occur in %1 minutes")).arg(ui.timeEdit_PierFlipAlert->time().toString("mm:ss"));
	systrayIcon.show();
	systrayIcon.showMessage("ZeGoto Control Center", Message, QSystemTrayIcon::Warning);
}

void ZeGotoControlCenter::on_radioButton_PierFlipAutomatic_toggled(bool checked)
{
	if (checked)
	{
		settings.setValue("PierFlipMode", 1);
		if (link != NULL)
		{
			link->Command(":psA#");
		}
	}
}

void ZeGotoControlCenter::on_radioButton_PierFlipManual_toggled(bool checked)
{
	if (checked)
	{
		settings.setValue("PierFlipMode", 2);
		if (link != NULL)
		{
			if (PierSide == EAST)
			{
				link->Command(":psE#");
			}
			else
			{
				link->Command(":psW#");
			}
		}
	}

	if (link != NULL)
	{
		ui.pushButton_PierFlipNow->setEnabled(checked);
		ui.comboBox_ManualSideOfPier->setEnabled(checked);
	}
}

void ZeGotoControlCenter::on_radioButton_PierFlipPictureFolder_toggled(bool checked)
{
	if (checked)
	{
		settings.setValue("PierFlipMode", 3);
		if (link != NULL)
		{
			if (PierSide == EAST)
			{
				link->Command(":psE#");
			}
			else
			{
				link->Command(":psW#");
			}
		}
	}

	ui.lineEdit_PierFlipPictureFolder->setEnabled(checked);
	ui.pushButton_PierFlipPictureFolder->setEnabled(checked);
}

void ZeGotoControlCenter::on_pushButton_PierFlipPictureFolder_clicked()
{
	QString dir = ui.lineEdit_PierFlipPictureFolder->text();

	if (dir.isEmpty())
	{
		dir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
	}
		
	dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dir, QFileDialog::ShowDirsOnly);
	ui.lineEdit_PierFlipPictureFolder->setText(dir);
	settings.setValue("PierFlipPictureFolder", dir);
}

void ZeGotoControlCenter::on_lineEdit_PierFlipPictureFolder_editingFinished()
{
	settings.setValue("PierFlipPictureFolder", ui.lineEdit_PierFlipPictureFolder->text());
}

void ZeGotoControlCenter::on_comboBox_ManualSideOfPier_currentIndexChanged(int index)
{
	if (link != NULL && !ui.radioButton_PierFlipAutomatic->isChecked())
	{
		if (index == 0)
		{
			link->Command(":psE#");
		}
		else
		{
			link->Command(":psW#");
		}
	}
}
