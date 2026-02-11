/*---------------------------------------------------------*\
| QMKVialRGBSettingsEntry.cpp                               |
|                                                           |
|   User interface entry for VialRGB QMK configuration      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "QMKVialRGBSettingsEntry.h"
#include "ui_QMKVialRGBSettingsEntry.h"

QMKVialRGBSettingsEntry::QMKVialRGBSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::QMKVialRGBSettingsEntry)
{
    ui->setupUi(this);
}

QMKVialRGBSettingsEntry::~QMKVialRGBSettingsEntry()
{
    delete ui;
}

void QMKVialRGBSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void QMKVialRGBSettingsEntry::loadFromSettings(const QMKOpenDeviceSettings& data)
{
    ui->NameEdit->setText(QString::fromStdString(data.name));
    ui->USBVIDEdit->setText(QString::fromStdString(data.usb_vid));
    ui->USBPIDEdit->setText(QString::fromStdString(data.usb_pid));
}

json QMKVialRGBSettingsEntry::saveSettings()
{
    return QMKOpenDeviceSettings
    {
        .usb_pid  = ui->USBPIDEdit->text().toStdString(),
        .usb_vid  = ui->USBVIDEdit->text().toStdString(),
        .name     = ui->NameEdit->text().toStdString(),
    };
}

bool QMKVialRGBSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnQMKVialRGBSettingsEntry(const json& data)
{
    QMKVialRGBSettingsEntry* entry = new QMKVialRGBSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

static const char* QMKDeviceName = QT_TRANSLATE_NOOP("ManualDevice", "QMK (VialRGB Protocol)");

REGISTER_MANUAL_DEVICE_TYPE(QMKDeviceName, QMK_VIAL_CFG_KEY, SpawnQMKVialRGBSettingsEntry);
