/*---------------------------------------------------------*\
| E131SettingsEntry.cpp                                     |
|                                                           |
|   User interface for OpenRGB E1.31 settings entry         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "E131SettingsEntry.h"
#include "ui_E131SettingsEntry.h"

E131SettingsEntry::E131SettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::E131SettingsEntry)
{
    ui->setupUi(this);

    ui->TypeComboBox->addItem(tr("Single"));
    ui->TypeComboBox->addItem(tr("Linear"));
    ui->TypeComboBox->addItem(tr("Matrix"));

    ui->MatrixOrderComboBox->addItem(tr("Horizontal Top Left"));
    ui->MatrixOrderComboBox->addItem(tr("Horizontal Top Right"));
    ui->MatrixOrderComboBox->addItem(tr("Horizontal Bottom Left"));
    ui->MatrixOrderComboBox->addItem(tr("Horizontal Bottom Right"));
    ui->MatrixOrderComboBox->addItem(tr("Vertical Top Left"));
    ui->MatrixOrderComboBox->addItem(tr("Vertical Top Right"));
    ui->MatrixOrderComboBox->addItem(tr("Vertical Bottom Left"));
    ui->MatrixOrderComboBox->addItem(tr("Vertical Bottom Right"));

    ui->RGBOrderComboBox->addItem("RGB");
    ui->RGBOrderComboBox->addItem("RBG");
    ui->RGBOrderComboBox->addItem("GRB");
    ui->RGBOrderComboBox->addItem("GBR");
    ui->RGBOrderComboBox->addItem("BRG");
    ui->RGBOrderComboBox->addItem("BGR");

    HideMatrixSettings();
}

E131SettingsEntry::~E131SettingsEntry()
{
    delete ui;
}

void E131SettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void E131SettingsEntry::HideMatrixSettings()
{
    ui->MatrixWidthLabel->setDisabled(true);
    ui->MatrixWidthEdit->setDisabled(true);

    ui->MatrixHeightLabel->setDisabled(true);
    ui->MatrixHeightEdit->setDisabled(true);

    ui->MatrixOrderLabel->setDisabled(true);
    ui->MatrixOrderComboBox->setDisabled(true);
}

void E131SettingsEntry::ShowMatrixSettings()
{
    ui->MatrixWidthLabel->setDisabled(false);
    ui->MatrixWidthEdit->setDisabled(false);

    ui->MatrixHeightLabel->setDisabled(false);
    ui->MatrixHeightEdit->setDisabled(false);

    ui->MatrixOrderLabel->setDisabled(false);
    ui->MatrixOrderComboBox->setDisabled(false);
}

void E131SettingsEntry::on_TypeComboBox_currentIndexChanged(int index)
{
    if(index == 2)
    {
        ShowMatrixSettings();
    }
    else
    {
        HideMatrixSettings();
    }
}

void E131SettingsEntry::loadFromSettings(const E131Device& data)
{
    ui->NameEdit->setText(QString::fromStdString(data.name));
    ui->IPEdit->setText(QString::fromStdString(data.ip));
    ui->StartUniverseEdit->setText(QString::number((int)data.start_universe));
    ui->StartChannelEdit->setText(QString::number((int)data.start_channel));
    ui->NumLEDsEdit->setText(QString::number((int)data.num_leds));

    switch(data.type)
    {
        case ZONE_TYPE_SINGLE:
            ui->TypeComboBox->setCurrentIndex(0);
            break;
        case ZONE_TYPE_LINEAR:
            ui->TypeComboBox->setCurrentIndex(1);
            break;
        case ZONE_TYPE_MATRIX:
            ui->TypeComboBox->setCurrentIndex(2);
            break;
        default:
            ui->TypeComboBox->setCurrentIndex(data.type);
    };

    switch(data.rgb_order)
    {
        case E131RGBOrder::RGB:
            ui->RGBOrderComboBox->setCurrentIndex(0);
            break;
        case E131RGBOrder::RBG:
            ui->RGBOrderComboBox->setCurrentIndex(1);
            break;
        case E131RGBOrder::GRB:
            ui->RGBOrderComboBox->setCurrentIndex(2);
            break;
        case E131RGBOrder::GBR:
            ui->RGBOrderComboBox->setCurrentIndex(3);
            break;
        case E131RGBOrder::BRG:
            ui->RGBOrderComboBox->setCurrentIndex(4);
            break;
        case E131RGBOrder::BGR:
            ui->RGBOrderComboBox->setCurrentIndex(5);
            break;
    };

    ui->MatrixWidthEdit->setText(QString::number((int)data.matrix_width));
    ui->MatrixHeightEdit->setText(QString::number((int)data.matrix_height));

    switch(data.matrix_order)
    {
        case E131MatrixOrder::HORIZONTAL_TOP_LEFT:
            ui->MatrixOrderComboBox->setCurrentIndex(0);
            break;
        case E131MatrixOrder::HORIZONTAL_TOP_RIGHT:
            ui->MatrixOrderComboBox->setCurrentIndex(1);
            break;
        case E131MatrixOrder::HORIZONTAL_BOTTOM_LEFT:
            ui->MatrixOrderComboBox->setCurrentIndex(2);
            break;
        case E131MatrixOrder::HORIZONTAL_BOTTOM_RIGHT:
            ui->MatrixOrderComboBox->setCurrentIndex(3);
            break;
        case E131MatrixOrder::VERTICAL_TOP_LEFT:
            ui->MatrixOrderComboBox->setCurrentIndex(4);
            break;
        case E131MatrixOrder::VERTICAL_TOP_RIGHT:
            ui->MatrixOrderComboBox->setCurrentIndex(5);
            break;
        case E131MatrixOrder::VERTICAL_BOTTOM_LEFT:
            ui->MatrixOrderComboBox->setCurrentIndex(6);
            break;
        case E131MatrixOrder::VERTICAL_BOTTOM_RIGHT:
            ui->MatrixOrderComboBox->setCurrentIndex(7);
            break;
    }

    ui->UniverseSizeEdit->setText(QString::number((int)data.universe_size));
    ui->KeepaliveTimeEdit->setText(QString::number((int)data.keepalive_time));
}

json E131SettingsEntry::saveSettings()
{
    E131Device device;
    /*-------------------------------------------------*\
    | Required parameters                               |
    \*-------------------------------------------------*/
    device.name                = ui->NameEdit->text().toStdString();
    device.start_universe      = ui->StartUniverseEdit->text().toUInt();
    device.start_channel       = ui->StartChannelEdit->text().toUInt();
    device.num_leds            = ui->NumLEDsEdit->text().toUInt();
    device.type                = ui->TypeComboBox->currentIndex();
    device.rgb_order           = ui->RGBOrderComboBox->currentIndex();

    /*-------------------------------------------------*\
    | Optional parameters                               |
    \*-------------------------------------------------*/
    if(ui->IPEdit->text() != "")
    {
        device.ip              = ui->IPEdit->text().toStdString();
    }

    if(device.type == ZONE_TYPE_MATRIX)
    {
        device.matrix_width    = ui->MatrixWidthEdit->text().toUInt();
        device.matrix_height   = ui->MatrixHeightEdit->text().toUInt();
        device.matrix_order    = ui->MatrixOrderComboBox->currentIndex();
    }

    if(ui->UniverseSizeEdit->text() != "")
    {
        device.universe_size   = ui->UniverseSizeEdit->text().toUInt();
    }

    if(ui->KeepaliveTimeEdit->text() != "")
    {
        device.keepalive_time  = ui->KeepaliveTimeEdit->text().toUInt();
    }

    return device;
}

bool E131SettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnE131Entry(const json& data)
{
    E131SettingsEntry* entry = new E131SettingsEntry;
    entry->loadFromSettings(data);
    return entry;
}

static const char* E131DeviceName = QT_TRANSLATE_NOOP("ManualDevice", "E1.31 (including WLED)");

REGISTER_MANUAL_DEVICE_TYPE(E131DeviceName, E131_SETTINGS_KEY, SpawnE131Entry);
