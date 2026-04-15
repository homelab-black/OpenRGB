/*---------------------------------------------------------*\
| OpenRGBDeviceSettingsPage.cpp                             |
|                                                           |
|   User interface for OpenRGB device settings page         |
|                                                           |
|   Adam Honse <calcprogrammer1@gmail.com>      15 Apr 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "OpenRGBDynamicSettingsWidget.h"
#include "OpenRGBDeviceSettingsPage.h"
#include "ui_OpenRGBDeviceSettingsPage.h"

static void Callback(void* this_ptr, std::string key, nlohmann::json settings)
{
    ((OpenRGBDeviceSettingsPage*)this_ptr)->OnSettingChanged(key, settings);
}

OpenRGBDeviceSettingsPage::OpenRGBDeviceSettingsPage(RGBController *dev, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenRGBDeviceSettingsPage)
{
    ui->setupUi(this);

    /*-----------------------------------------------------*\
    | Store device pointer                                  |
    \*-----------------------------------------------------*/
    device = dev;

    /*-----------------------------------------------------*\
    | Initialize settings list                              |
    \*-----------------------------------------------------*/
    nlohmann::json settings_schema  = device->GetDeviceSettingsSchema();
    nlohmann::json settings_value   = device->GetDeviceSettings();

    /*-----------------------------------------------------*\
    | Loop through the schema and create an entry for each  |
    | setting                                               |
    \*-----------------------------------------------------*/
    for(nlohmann::json::iterator json_iterator = settings_schema.begin(); json_iterator != settings_schema.end(); json_iterator++)
    {
        nlohmann::json                  schema_entry    = json_iterator.value();
        OpenRGBDynamicSettingsWidget*   item_widget     = new OpenRGBDynamicSettingsWidget(json_iterator.key(), schema_entry, settings_value);

        item_widget->SetCallback(Callback, this);
        ui->ScrollAreaDeviceConfigurationLayout->addWidget(item_widget);
    }
}

OpenRGBDeviceSettingsPage::~OpenRGBDeviceSettingsPage()
{

}

void OpenRGBDeviceSettingsPage::OnSettingChanged(std::string /*key*/, nlohmann::json settings)
{
    device->SetDeviceSettings(settings);
}
