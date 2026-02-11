/*---------------------------------------------------------*\
| KasaSmartSettingsEntry.cpp                                |
|                                                           |
|   User interface for OpenRGB Kasa Smart settings entry    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "KasaSmartSettingsEntry.h"
#include "ui_KasaSmartSettingsEntry.h"

KasaSmartSettingsEntry::KasaSmartSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::KasaSmartSettingsEntry)
{
    ui->setupUi(this);
}

KasaSmartSettingsEntry::~KasaSmartSettingsEntry()
{
    delete ui;
}

void KasaSmartSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void KasaSmartSettingsEntry::loadFromSettings(const KasaSmartDevice& data)
{
    ui->IPEdit->setText(QString::fromStdString(data.ip));
    ui->NameEdit->setText(QString::fromStdString(data.name));
}

json KasaSmartSettingsEntry::saveSettings()
{
    return KasaSmartDevice
    {
        .ip = ui->IPEdit->text().toStdString(),
        .name = ui->NameEdit->text().toStdString(),
    };
}

void KasaSmartSettingsEntry::setName(QString name)
{
    ui->NameEdit->setText(name);
}

bool KasaSmartSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnKasaSmartSettingsEntry(const json& data)
{
    KasaSmartSettingsEntry* entry = new KasaSmartSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

REGISTER_MANUAL_DEVICE_TYPE("Kasa Smart", KASA_SMART_CFG_KEY, SpawnKasaSmartSettingsEntry);
