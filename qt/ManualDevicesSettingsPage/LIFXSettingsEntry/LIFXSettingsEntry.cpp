/*---------------------------------------------------------*\
| LIFXSettingsEntry.cpp                                     |
|                                                           |
|   User interface for OpenRGB LIFX settings entry          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "LIFXSettingsEntry.h"
#include "ui_LIFXSettingsEntry.h"

LIFXSettingsEntry::LIFXSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::LIFXSettingsEntry)
{
    ui->setupUi(this);

    connect(ui->MultizoneCheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_MultizoneCheckBox_stateChanged(int)));
}

LIFXSettingsEntry::~LIFXSettingsEntry()
{
    delete ui;
}

void LIFXSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void LIFXSettingsEntry::loadFromSettings(const LIFXDevice& data)
{
    ui->IPEdit->setText(QString::fromStdString(data.ip));
    ui->NameEdit->setText(QString::fromStdString(data.name));
    ui->MultizoneCheckBox->setCheckState(data.multizone ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->ExtendedMultizoneCheckBox->setCheckState(data.extended_multizone ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}

json LIFXSettingsEntry::saveSettings()
{
    return LIFXDevice
    {
        .ip                 = ui->IPEdit->text().toStdString(),
        .name               = ui->NameEdit->text().toStdString(),
        .multizone          = ui->MultizoneCheckBox->checkState() == Qt::Checked,
        .extended_multizone = ui->ExtendedMultizoneCheckBox->checkState() == Qt::Checked,
    };
}

void LIFXSettingsEntry::setName(QString name)
{
    ui->NameEdit->setText(name);
}

void LIFXSettingsEntry::on_MultizoneCheckBox_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
    {
        ui->ExtendedMultizoneCheckBox->setEnabled(true);
    }
    else
    {
        ui->ExtendedMultizoneCheckBox->setEnabled(false);
        ui->ExtendedMultizoneCheckBox->setCheckState(Qt::Unchecked);
    }
}

bool LIFXSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnLIFXSettingsEntry(const json& data)
{
    LIFXSettingsEntry* entry = new LIFXSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

REGISTER_MANUAL_DEVICE_TYPE("LIFX", LIFX_CFG_KEY, SpawnLIFXSettingsEntry);
