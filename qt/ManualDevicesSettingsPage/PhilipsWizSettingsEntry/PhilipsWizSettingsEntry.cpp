/*---------------------------------------------------------*\
| PhilipsWizSettingsEntry.cpp                               |
|                                                           |
|   User interface for OpenRGB Philips Wiz settings entry   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "PhilipsWizSettingsEntry.h"
#include "ui_PhilipsWizSettingsEntry.h"

PhilipsWizSettingsEntry::PhilipsWizSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::PhilipsWizSettingsEntry)
{
    ui->setupUi(this);

    ui->WhiteStrategyComboBox->addItem(tr("Average"));
    ui->WhiteStrategyComboBox->addItem(tr("Minimum"));
}

PhilipsWizSettingsEntry::~PhilipsWizSettingsEntry()
{
    delete ui;
}

void PhilipsWizSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void PhilipsWizSettingsEntry::loadFromSettings(const PhilipsWizDevice& data)
{
    ui->IPEdit->setText(QString::fromStdString(data.ip));
    ui->UseCoolWhiteCheckBox->setChecked(data.use_cool_white);
    ui->UseWarmWhiteCheckBox->setChecked(data.use_warm_white);
    ui->WhiteStrategyComboBox->setCurrentText(QString::fromStdString(data.selected_white_strategy));
}

json PhilipsWizSettingsEntry::saveSettings()
{
    return PhilipsWizDevice
    {
        .ip               = ui->IPEdit->text().toStdString(),
        .use_cool_white   = ui->UseCoolWhiteCheckBox->isChecked(),
        .use_warm_white   = ui->UseWarmWhiteCheckBox->isChecked(),
        .selected_white_strategy   = ui->WhiteStrategyComboBox->currentText().toStdString(),
    };
}

bool PhilipsWizSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnPhilipsWizSettingsEntry(const json& data)
{
    PhilipsWizSettingsEntry* entry = new PhilipsWizSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

REGISTER_MANUAL_DEVICE_TYPE("Philips Wiz", PHILIPZWIZ_CFG_KEY, SpawnPhilipsWizSettingsEntry);
