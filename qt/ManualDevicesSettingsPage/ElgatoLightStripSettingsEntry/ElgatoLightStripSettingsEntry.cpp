/*---------------------------------------------------------*\
| ElgatoLightStripSettingsEntry.cpp                         |
|                                                           |
|   User interface for OpenRGB Elgato Light Strips entry    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ElgatoLightStripSettingsEntry.h"
#include "ui_ElgatoLightStripSettingsEntry.h"

ElgatoLightStripSettingsEntry::ElgatoLightStripSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::ElgatoLightStripSettingsEntry)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

ElgatoLightStripSettingsEntry::~ElgatoLightStripSettingsEntry()
{
    delete ui;
}

void ElgatoLightStripSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void ElgatoLightStripSettingsEntry::loadFromSettings(const ElgatoDeviceSettings& data)
{
    ui->IPEdit->setText(QString::fromStdString(data.ip));
}

json ElgatoLightStripSettingsEntry::saveSettings()
{
    return ElgatoDeviceSettings
    {
        .ip = ui->IPEdit->text().toStdString(),
    };
}

bool ElgatoLightStripSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnElgatoLightStripEntry(const json& data)
{
    ElgatoLightStripSettingsEntry* entry = new ElgatoLightStripSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

REGISTER_MANUAL_DEVICE_TYPE("Elgato Light Strip", ELGATO_LIGHTSTRIP_CFG_KEY, SpawnElgatoLightStripEntry);

