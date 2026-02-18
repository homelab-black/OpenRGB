/*---------------------------------------------------------*\
| YeelightSettingsEntry.cpp                                 |
|                                                           |
|   User interface for Yeelight settings entry              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QInputDialog>
#include "YeelightSettingsEntry.h"
#include "ui_YeelightSettingsEntry.h"
#include "net_port.h"

YeelightSettingsEntry::YeelightSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::YeelightSettingsEntry)
{
    ui->setupUi(this);
}

YeelightSettingsEntry::~YeelightSettingsEntry()
{
    delete ui;
}

void YeelightSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void YeelightSettingsEntry::on_HostIPChooserButton_clicked()
{
    char hostname[256];
    gethostname(hostname, 256);

    char **in_addrs = gethostbyname(hostname)->h_addr_list;

    QStringList in_addr_list;

    while (*in_addrs != NULL)
    {
        in_addr_list << inet_ntoa(*((struct in_addr*) *in_addrs));
        in_addrs++;
    }

    QInputDialog inp;

    inp.setOptions(QInputDialog::UseListViewForComboBoxItems);
    inp.setComboBoxItems(in_addr_list);
    inp.setWindowTitle(tr("Choose an IP..."));
    inp.setLabelText(tr("Choose the correct IP for the host"));

    if(!inp.exec())
    {
        return;
    }

    ui->HostIPEdit->setText(inp.textValue());
}

void YeelightSettingsEntry::loadFromSettings(const YeelightDevice& data)
{
    ui->IPEdit->setText(QString::fromStdString(data.ip));
    ui->HostIPEdit->setText(QString::fromStdString(data.host_ip));
    ui->MusicModeCheckBox->setChecked(data.music_mode);
}

json YeelightSettingsEntry::saveSettings()
{
    /*-------------------------------------------------*\
    | Required parameters                               |
    \*-------------------------------------------------*/
    return YeelightDevice
    {
        .ip          = ui->IPEdit->text().toStdString(),
        .host_ip     = ui->HostIPEdit->text().toStdString(),
        .music_mode  = ui->MusicModeCheckBox->isChecked(),
    };

}

bool YeelightSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnYeelightSettingsEntry(const json& data)
{
    YeelightSettingsEntry* entry = new YeelightSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

REGISTER_MANUAL_DEVICE_TYPE("Yeelight", YEELIGHT_CFG_KEY, SpawnYeelightSettingsEntry);
