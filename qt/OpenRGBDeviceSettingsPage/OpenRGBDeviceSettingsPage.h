/*---------------------------------------------------------*\
| OpenRGBDeviceSettingsPage.h                               |
|                                                           |
|   User interface for OpenRGB device settings page         |
|                                                           |
|   Adam Honse <calcprogrammer1@gmail.com>      15 Apr 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <QDialog>
#include "RGBController.h"

namespace Ui
{
    class OpenRGBDeviceSettingsPage;
}

class OpenRGBDeviceSettingsPage : public QDialog
{
    Q_OBJECT

public:
    explicit OpenRGBDeviceSettingsPage(RGBController *dev, QWidget *parent = nullptr);
    ~OpenRGBDeviceSettingsPage();

    void OnSettingChanged(std::string key, nlohmann::json settings);

private:
    /*-----------------------------------------------------*\
    | UI Pointer                                            |
    \*-----------------------------------------------------*/
    Ui::OpenRGBDeviceSettingsPage *ui;

    /*-----------------------------------------------------*\
    | Device pointer                                        |
    \*-----------------------------------------------------*/
    RGBController *device;
};
