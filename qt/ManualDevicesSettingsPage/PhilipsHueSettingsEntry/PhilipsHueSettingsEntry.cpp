/*---------------------------------------------------------*\
| PhilipsHueSettingsEntry.cpp                               |
|                                                           |
|   User interface for OpenRGB Philips Hue settings entry   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "PhilipsHueSettingsEntry.h"
#include "ui_PhilipsHueSettingsEntry.h"

PhilipsHueSettingsEntry::PhilipsHueSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::PhilipsHueSettingsEntry)
{
    ui->setupUi(this);
}

PhilipsHueSettingsEntry::~PhilipsHueSettingsEntry()
{
    delete ui;
}

void PhilipsHueSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void PhilipsHueSettingsEntry::on_UnpairButton_clicked()
{
    ui->UsernameValue->setText("");
    ui->ClientKeyValue->setText("");
}

void PhilipsHueSettingsEntry::loadFromSettings(const BridgeSettings& data)
{
    ui->IPEdit->setText(QString::fromStdString(data.ip));
    ui->MACEdit->setText(QString::fromStdString(data.mac));
    ui->EntertainmentCheckBox->setChecked(data.entertainment);
    ui->AutoConnectCheckBox->setChecked(data.autoconnect);
    ui->UsernameValue->setText(QString::fromStdString(data.username));
    ui->ClientKeyValue->setText(QString::fromStdString(data.clientkey));
}

json PhilipsHueSettingsEntry::saveSettings()
{
    BridgeSettings settings
    {
        .ip               = ui->IPEdit->text().toStdString(),
        .mac              = ui->MACEdit->text().toStdString(),
        .username         = ui->UsernameValue->text().toStdString(),
        .clientkey        = ui->ClientKeyValue->text().toStdString(),
        .autoconnect      = ui->AutoConnectCheckBox->isChecked(),
        .entertainment    = ui->EntertainmentCheckBox->isChecked(),
    };
    json j = settings;
    return j;
}

bool PhilipsHueSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnPhilipsHueSettingsEntry(const json& data)
{
    PhilipsHueSettingsEntry* entry = new PhilipsHueSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

REGISTER_MANUAL_DEVICE_TYPE("Philips Hue", "PhilipsHueDevices", SpawnPhilipsHueSettingsEntry);
