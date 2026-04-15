/*---------------------------------------------------------*\
| OpenRGBSettingsPage.cpp                                   |
|                                                           |
|   User interface for OpenRGB settings page                |
|                                                           |
|   Adam Honse <calcprogrammer1@gmail.com>      15 Apr 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QGroupBox>
#include <QVBoxLayout>
#include "OpenRGBDynamicSettingsWidget.h"
#include "OpenRGBSettingsPage.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "ui_OpenRGBSettingsPage.h"

static void Callback(void* this_ptr, std::string key, nlohmann::json settings)
{
    ((OpenRGBSettingsPage*)this_ptr)->OnSettingChanged(key, settings);
}

OpenRGBSettingsPage::OpenRGBSettingsPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenRGBSettingsPage)
{
    ui->setupUi(this);

    /*-----------------------------------------------------*\
    | Initialize settings list                              |
    \*-----------------------------------------------------*/
    nlohmann::json settings_schema  = ResourceManager::get()->GetSettingsManager()->GetSettingsSchema("");

    /*-----------------------------------------------------*\
    | Loop through the schema and create an entry for each  |
    | setting                                               |
    \*-----------------------------------------------------*/
    for(nlohmann::json::iterator key_iterator = settings_schema.begin(); key_iterator != settings_schema.end(); key_iterator++)
    {
        nlohmann::json                  setting_value   = ResourceManager::get()->GetSettingsManager()->GetSettings(key_iterator.key());
        OpenRGBDynamicSettingsWidget*   item_widget     = new OpenRGBDynamicSettingsWidget(key_iterator.key(), key_iterator.value(), setting_value);

        ui->ScrollAreaSettingsLayout->addWidget(item_widget);
    }
}

OpenRGBSettingsPage::~OpenRGBSettingsPage()
{

}

void OpenRGBSettingsPage::OnSettingChanged(std::string /*key*/, nlohmann::json settings)
{

}
