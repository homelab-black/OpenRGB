/*---------------------------------------------------------*\
| YeelightSettingsEntry.h                            |
|                                                           |
|   User interface for Yeelight settings entry              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "BaseManualDeviceEntry.h"
#include "YeelightController.h"

namespace Ui
{
    class YeelightSettingsEntry;
}

class YeelightSettingsEntry : public BaseManualDeviceEntry
{
    Q_OBJECT

public:
    explicit YeelightSettingsEntry(QWidget *parent = nullptr);
    ~YeelightSettingsEntry();
    void loadFromSettings(const YeelightDevice& data);
    json saveSettings() override;
    bool isDataValid() override;

private:
    Ui::YeelightSettingsEntry *ui;

private slots:
    void changeEvent(QEvent *event) override;
    void on_HostIPChooserButton_clicked();
};
