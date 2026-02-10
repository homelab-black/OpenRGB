/*---------------------------------------------------------*\
| NanoleafSettingsEntry.cpp                                 |
|                                                           |
|   User interface for OpenRGB Nanoleaf settings entry      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "NanoleafSettingsEntry.h"
#include "ui_NanoleafSettingsEntry.h"

#include "NanoleafScanDialog.h"
#include "ResourceManager.h"
#include "SettingsManager.h"

NanoleafSettingsEntry::NanoleafSettingsEntry(QWidget *parent) :
    BaseManualDeviceEntry(parent),
    ui(new Ui::NanoleafSettingsEntry),
    paired(false)
{
    ui->setupUi(this);
}

NanoleafSettingsEntry::NanoleafSettingsEntry(QString a_address, int a_port) :
    NanoleafSettingsEntry(nullptr)
{
    address = a_address;
    port = a_port;
    const std::string location = getLocation();

    ui->IPValue->setText(address);
    ui->PortValue->setText(QString::fromStdString(std::to_string(a_port)));

    std::optional<NanoleafSettings> cfg_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettings<NanoleafSettings>(NANOLEAF_CFG_KEY);
    NanoleafSettings settings = cfg_settings.value_or(NanoleafSettings {});

    if(settings.devices.count(location) &&
        settings.devices.at(location).auth_token.size())
    {
        paired = true;
        auth_token = settings.devices.at(location).auth_token;
        ui->AuthKeyValue->setText(QString::fromStdString(auth_token));
        ui->PairButton->hide();
    }
    else
    {
        ui->UnpairButton->hide();
    }
}

NanoleafSettingsEntry::~NanoleafSettingsEntry()
{
    delete ui;
}

void NanoleafSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void NanoleafSettingsEntry::on_PairButton_clicked()
{
    try
    {
        auth_token = NanoleafController::Pair(address.toStdString(), port);

        // Save auth token.
        std::string location = getLocation();
        std::optional<NanoleafSettings> cfg_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettings<NanoleafSettings>(NANOLEAF_CFG_KEY);
        NanoleafSettings settings = cfg_settings.value_or(NanoleafSettings {});
        NanoleafDeviceSettings new_device
        {
            .ip = address.toStdString(),
            .port = (unsigned int) port,
            .auth_token = auth_token
        };
        settings.devices.insert({location, new_device});
        ResourceManager::get()->GetSettingsManager()->SetControllerSettingsAndSave(NANOLEAF_CFG_KEY, settings);

        // Update UI.
        paired = true;
        ui->AuthKeyValue->setText(QString::fromStdString(auth_token));
        ui->PairButton->hide();
        ui->UnpairButton->show();
    }
    catch(const std::exception& /*e*/)
    {
        paired = false;
        ui->AuthKeyValue->setText("PAIRING FAILED");
    }
}

void NanoleafSettingsEntry::on_UnpairButton_clicked()
{
    NanoleafController::Unpair(address.toStdString(), port, auth_token);

    std::string location = getLocation();
    SettingsManager* sm = ResourceManager::get()->GetSettingsManager();
    std::optional<NanoleafSettings> cfg_settings = sm->GetControllerSettings<NanoleafSettings>(NANOLEAF_CFG_KEY);
    NanoleafSettings settings = cfg_settings.value_or(NanoleafSettings {});
    settings.devices.erase(location);
    sm->SetControllerSettingsAndSave(NANOLEAF_CFG_KEY, settings);

    paired = false;
    ui->AuthKeyValue->setText("");
    ui->PairButton->show();
    ui->UnpairButton->hide();
}

void NanoleafSettingsEntry::loadFromSettings(const NanoleafSettings& data)
{
    std::string location = getLocation();
    if(!data.devices.count(location))
    {
        return;
    }
    const NanoleafDeviceSettings& device = data.devices.at(location);
    address = QString::fromStdString(device.ip);
    port = device.port;

    ui->IPValue->setText(address);
    ui->PortValue->setText(QString::fromStdString(std::to_string(port)));

    if(device.auth_token.size())
    {
        auth_token = device.auth_token;
        ui->AuthKeyValue->setText(QString::fromStdString(auth_token));
        ui->PairButton->hide();
    }
    else
    {
        auth_token.clear();
        ui->UnpairButton->hide();
    }
}

json NanoleafSettingsEntry::saveSettings()
{
    NanoleafDeviceSettings device_settings
    {
        .ip = address.toStdString(),
        .port = (unsigned int) port,
        .auth_token = auth_token,
    };
    json j = device_settings;
    return j;
}

std::string NanoleafSettingsEntry::getLocation()
{
    return (address.toStdString() + ":" + std::to_string(port));
}

bool NanoleafSettingsEntry::isDataValid()
{
    // stub
    return true;
}

static BaseManualDeviceEntry* SpawnNanoleafSettingsEntry(const json& data)
{
    if(data.empty())
    {
        // A special case: we open a new scanning dialog instead of returning a new entry
        // The caller should be able to handle this
        NanoleafScanDialog scanPage;
        scanPage.exec();
        return nullptr;
    }
    else
    {
        NanoleafSettingsEntry* entry = new NanoleafSettingsEntry;
        NanoleafSettings settings = data.get<NanoleafSettings>();
        entry->loadFromSettings(settings);
        return entry;
    }
}

REGISTER_MANUAL_DEVICE_TYPE("Nanoleaf", NANOLEAF_CFG_KEY, SpawnNanoleafSettingsEntry);
