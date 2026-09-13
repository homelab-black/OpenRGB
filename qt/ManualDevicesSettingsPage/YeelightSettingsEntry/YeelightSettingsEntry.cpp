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
#include "YeelightController.h"

YeelightSettingsEntry::YeelightSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::YeelightSettingsEntry)
{
    ui->setupUi(this);
    on_ProtocolComboBox_currentIndexChanged(ui->ProtocolComboBox->currentIndex());
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
    if(gethostname(hostname, sizeof(hostname)) != 0)
    {
        return;
    }

    struct hostent* host_entry = gethostbyname(hostname);
    if(host_entry == nullptr)
    {
        return;
    }

    char **in_addrs = host_entry->h_addr_list;

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

void YeelightSettingsEntry::loadFromSettings(const json& data)
{
    if(data.contains("ip"))
    {
        ui->IPEdit->setText(QString::fromStdString(data["ip"]));
    }

    if(data.contains("host_ip"))
    {
        ui->HostIPEdit->setText(QString::fromStdString(data["host_ip"]));
    }

    ui->ProtocolComboBox->setCurrentIndex(YeelightController::GetProtocolFromSettings(data));
}

void YeelightSettingsEntry::on_ProtocolComboBox_currentIndexChanged(int index)
{
    bool music_mode = (index == YEELIGHT_PROTOCOL_MUSIC);
    ui->HostIPEdit->setEnabled(music_mode);
    ui->HostIPChooserButton->setEnabled(music_mode);
    ui->label->setEnabled(music_mode);
    ui->label_2->setEnabled(music_mode);
}

json YeelightSettingsEntry::saveSettings()
{
    json result;
    /*-------------------------------------------------*\
    | Required parameters                               |
    \*-------------------------------------------------*/
    result["ip"]          = ui->IPEdit->text().toStdString();
    result["host_ip"]     = ui->HostIPEdit->text().toStdString();
    result["protocol"]    = ui->ProtocolComboBox->currentText().toStdString();

    return result;
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

REGISTER_MANUAL_DEVICE_TYPE("Yeelight", "YeelightDevices", SpawnYeelightSettingsEntry);
