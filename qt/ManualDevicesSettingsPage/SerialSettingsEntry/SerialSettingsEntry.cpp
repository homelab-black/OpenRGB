/*---------------------------------------------------------*\
| SerialSettingsEntry.cpp                                   |
|                                                           |
|   User interface entry for serial device configuration    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "SerialSettingsEntry.h"
#include "ui_SerialSettingsEntry.h"

#include "serial_port.h"
#include <QStandardItemModel>

SerialSettingsEntry::SerialSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::SerialSettingsEntry)
{
    ui->setupUi(this);

    ui->ProtocolComboBox->addItem("Keyboard Visualizer");
    ui->ProtocolComboBox->addItem("Adalight");
    ui->ProtocolComboBox->addItem("TPM2");
    ui->ProtocolComboBox->addItem("Basic I2C");

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

SerialSettingsEntry::~SerialSettingsEntry()
{
    delete ui;
}

void SerialSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void SerialSettingsEntry::on_ProtocolComboBox_currentIndexChanged(int index)
{
    if(index == 3)
    {
        ui->BaudLabel->setText("Address:");
    }
    else
    {
        ui->BaudLabel->setText("Baud:");
    }
}

void SerialSettingsEntry::loadFromSettings(const LEDStripDevice& data)
{
    ui->NameEdit->setText(QString::fromStdString(data.name));
    ui->PortComboBox->setCurrentText(QString::fromStdString(data.port));
    ui->BaudEdit->setText(QString::number((int)data.baud));
    ui->NumLEDsEdit->setText(QString::number((int)data.num_leds));
    switch(data.protocol)
    {
        case LedProtocol::KEYBOARD_VISUALIZER:
            ui->ProtocolComboBox->setCurrentIndex(0);
            break;
        case LedProtocol::ADALIGHT:
            ui->ProtocolComboBox->setCurrentIndex(1);
            break;
        case LedProtocol::TPM2:
            ui->ProtocolComboBox->setCurrentIndex(2);
            break;
        case LedProtocol::BASIC_I2C:
            ui->ProtocolComboBox->setCurrentIndex(3);
            break;
    }
}

json SerialSettingsEntry::saveSettings()
{
    LEDStripDevice device;
    /*-------------------------------------------------*\
    | Required parameters                               |
    \*-------------------------------------------------*/
    device.name            = ui->NameEdit->text().toStdString();
    device.port            = ui->PortComboBox->currentText().toStdString();
    device.num_leds        = ui->NumLEDsEdit->text().toUInt();
    device.baud            = ui->BaudEdit->text().toUInt();

    switch(ui->ProtocolComboBox->currentIndex())
    {
        case 0:
            device.protocol = LedProtocol::KEYBOARD_VISUALIZER;
            break;
        case 1:
            device.protocol = LedProtocol::ADALIGHT;
            break;
        case 2:
            device.protocol = LedProtocol::TPM2;
            break;
        case 3:
            device.protocol = LedProtocol::BASIC_I2C;
            break;
    }
    return device;
}

bool SerialSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnSerialSettingsEntry(const json& data)
{
    SerialSettingsEntry* entry = new SerialSettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

static const char* SerialDeviceName = QT_TRANSLATE_NOOP("ManualDevice", "Serial Device");

REGISTER_MANUAL_DEVICE_TYPE(SerialDeviceName, LEDSTRIP_CFG_KEY, SpawnSerialSettingsEntry);
