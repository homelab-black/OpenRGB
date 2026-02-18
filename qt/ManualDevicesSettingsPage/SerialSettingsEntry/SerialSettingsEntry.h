/*---------------------------------------------------------*\
| SerialSettingsEntry.h                                     |
|                                                           |
|   User interface entry for serial device configuration    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "BaseManualDeviceEntry.h"
#include "LEDStripController.h"

namespace Ui
{
    class SerialSettingsEntry;
}

class SerialSettingsEntry : public BaseManualDeviceEntry
{
    Q_OBJECT

private slots:
    void changeEvent(QEvent *event) override;

    void on_ProtocolComboBox_currentIndexChanged(int index);

public:
    explicit SerialSettingsEntry(QWidget *parent = nullptr);
    ~SerialSettingsEntry();
    void loadFromSettings(const LEDStripDevice& data);
    json saveSettings() override;
    bool isDataValid() override;

private:
    Ui::SerialSettingsEntry *ui;
};
