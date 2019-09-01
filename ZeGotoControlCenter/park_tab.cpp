#include "zegotocontrolcenter.h"

void ZeGotoControlCenter::on_pushButton_Park_clicked()
{
    link->CommandString(":hP#");
}

void ZeGotoControlCenter::on_pushButton_Unpark_clicked()
{
    link->CommandString(":hW#");
}

void ZeGotoControlCenter::on_comboBox_ParkPositions_currentIndexChanged()
{
    int park_index = ui.comboBox_ParkPositions->currentIndex();
    bool enable = (park_index == 3);

    ui.pushButton_SetParkPosition->setEnabled(enable);
    ui.lineEdit_ParkPositionAlt->setEnabled(enable);
    ui.lineEdit_ParkPositionAz->setEnabled(enable);

    if (park_index < 3)
    {
        char cmd[8];

        sprintf(cmd, ":hS%i#", park_index + 1);
        link->CommandBlind(cmd);
    }
}

void ZeGotoControlCenter::setComboParkPosition(const char *response)
{
    if (strcmp(response, "Park1#") == 0)
    {
        ui.comboBox_ParkPositions->setCurrentIndex(0);
    }
    else if (strcmp(response, "Park2#") == 0)
    {
        ui.comboBox_ParkPositions->setCurrentIndex(1);
    }
    else if (strcmp(response, "Park3#") == 0)
    {
        ui.comboBox_ParkPositions->setCurrentIndex(2);
    }
    else if (strncmp(response, "Az", 2) == 0)
    {
        ui.comboBox_ParkPositions->setCurrentIndex(3);
    }
}