/*---------------------------------------------------------*\
| QMKORGBSettingsEntry.cpp                                  |
|                                                           |
|   User interface entry for OpenRGB QMK configuration      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "QMKORGBSettingsEntry.h"
#include "ui_QMKORGBSettingsEntry.h"

QMKORGBSettingsEntry::QMKORGBSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::QMKORGBSettingsEntry)
{
    ui->setupUi(this);
}

QMKORGBSettingsEntry::~QMKORGBSettingsEntry()
{
    delete ui;
}

void QMKORGBSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void QMKORGBSettingsEntry::loadFromSettings(const QMKOpenDeviceSettings& data)
{
    ui->NameEdit->setText(QString::fromStdString(data.name));
    ui->USBVIDEdit->setText(QString::fromStdString(data.usb_vid));
    ui->USBPIDEdit->setText(QString::fromStdString(data.usb_pid));
}

json QMKORGBSettingsEntry::saveSettings()
{
    return QMKOpenDeviceSettings
    {
        .usb_pid  = ui->USBPIDEdit->text().toStdString(),
        .usb_vid  = ui->USBVIDEdit->text().toStdString(),
        .name     = ui->NameEdit->text().toStdString(),
    };
}

bool QMKORGBSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnQMKORGBSettingsEntry(const json& data)
{
    QMKORGBSettingsEntry* entry = new QMKORGBSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

static const char* QMKDeviceName = QT_TRANSLATE_NOOP("ManualDevice", "QMK (OpenRGB Protocol)");

REGISTER_MANUAL_DEVICE_TYPE(QMKDeviceName, QMK_OPEN_SETTINGS_KEY, SpawnQMKORGBSettingsEntry);
