/*---------------------------------------------------------*\
| KasaSmartSettingsEntry.h                                  |
|                                                           |
|   User interface for OpenRGB Kasa Smart settings entry    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "BaseManualDeviceEntry.h"
#include "KasaSmartController.h"

namespace Ui
{
    class KasaSmartSettingsEntry;
}

class KasaSmartSettingsEntry : public BaseManualDeviceEntry
{
    Q_OBJECT

public:
    explicit KasaSmartSettingsEntry(QWidget *parent = nullptr);
    ~KasaSmartSettingsEntry();
    void loadFromSettings(const KasaSmartDevice& data);
    void setName(QString name);

    json saveSettings() override;
    bool isDataValid() override;

private:
    Ui::KasaSmartSettingsEntry *ui;

private slots:
    void changeEvent(QEvent *event) override;
};
