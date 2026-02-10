/*---------------------------------------------------------*\
| DMXSettingsEntry.cpp                                      |
|                                                           |
|   User interface for OpenRGB DMX settings entry           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DMXSettingsEntry.h"
#include "ui_DMXSettingsEntry.h"

#include "serial_port.h"
#include <QStandardItemModel>

DMXSettingsEntry::DMXSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::DMXSettingsEntry)
{
    ui->setupUi(this);

    std::vector<std::string> serialPorts = serial_port::getSerialPorts();
    for(size_t i = 0; i < serialPorts.size(); ++i)
    {
        ui->PortComboBox->addItem(QString::fromStdString(serialPorts[i]));
    }
    if(serialPorts.empty())
    {
        /*---------------------------------------------------*\
        | When no ports were found, add an unselectable entry |
        | denoting this fact istead                           |
        \*---------------------------------------------------*/
        QStandardItemModel* comboBoxModel = qobject_cast<QStandardItemModel *>(ui->PortComboBox->model());
        if(comboBoxModel != nullptr)
        {
            ui->PortComboBox->addItem(tr("No serial ports found"));
            QStandardItem *item = comboBoxModel->item(0);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }
    ui->PortComboBox->clearEditText();
}

DMXSettingsEntry::~DMXSettingsEntry()
{
    delete ui;
}

void DMXSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void DMXSettingsEntry::loadFromSettings(const DMXDevice& data)
{
    ui->NameEdit->setText(QString::fromStdString(data.name));
    ui->PortComboBox->setCurrentText(QString::fromStdString(data.port));
    ui->RedEdit->setText(QString::number((int)data.red_channel));
    ui->GreenEdit->setText(QString::number((int)data.green_channel));
    ui->BlueEdit->setText(QString::number((int)data.blue_channel));
    ui->BrightnessEdit->setText(QString::number((int)data.brightness_channel));
    ui->KeepaliveTimeEdit->setText(QString::number((int)data.keepalive_time));
}

json DMXSettingsEntry::saveSettings()
{
    /*-------------------------------------------------*\
    | Required parameters                               |
    \*-------------------------------------------------*/
    DMXDevice device
    {
        .name           = ui->NameEdit->text().toStdString(),
        .port           = ui->PortComboBox->currentText().toStdString(),
        .red_channel    = ui->RedEdit->text().toUInt(),
        .green_channel  = ui->GreenEdit->text().toUInt(),
        .blue_channel   = ui->BlueEdit->text().toUInt(),
    };

    /*-------------------------------------------------*\
    | Optional parameters                               |
    \*-------------------------------------------------*/
    if(ui->BrightnessEdit->text() != "")
    {
        device.brightness_channel = ui->BrightnessEdit->text().toUInt();
    }

    if(ui->KeepaliveTimeEdit->text() != "")
    {
        device.keepalive_time = ui->KeepaliveTimeEdit->text().toUInt();
    }

    return device;
}

bool DMXSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnDMXEntry(const json& data)
{
    DMXSettingsEntry* entry = new DMXSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

REGISTER_MANUAL_DEVICE_TYPE("DMX", "DMXDevices", SpawnDMXEntry);
