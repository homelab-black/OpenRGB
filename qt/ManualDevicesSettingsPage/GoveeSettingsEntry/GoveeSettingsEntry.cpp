/*---------------------------------------------------------*\
| GoveeSettingsEntry.cpp                                    |
|                                                           |
|   User interface for OpenRGB Govee settings entry         |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      15 May 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "GoveeSettingsEntry.h"
#include "ui_GoveeSettingsEntry.h"

GoveeSettingsEntry::GoveeSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::GoveeSettingsEntry)
{
    ui->setupUi(this);
}

GoveeSettingsEntry::~GoveeSettingsEntry()
{
    delete ui;
}

void GoveeSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void GoveeSettingsEntry::loadFromSettings(const GoveeDevice& data)
{
    ui->IPEdit->setText(QString::fromStdString(data.ip));
}

json GoveeSettingsEntry::saveSettings()
{
    return GoveeDevice
    {
        .ip = ui->IPEdit->text().toStdString(),
    };
}

bool GoveeSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnGoveeSettingsEntry(const json& data)
{
    GoveeSettingsEntry* entry = new GoveeSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

REGISTER_MANUAL_DEVICE_TYPE("Govee", "GoveeDevices", SpawnGoveeSettingsEntry);
