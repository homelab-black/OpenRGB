/*---------------------------------------------------------*\
| OpenRGBYeelightSettingsEntry.cpp                          |
|                                                           |
|   User interface for Yeelight settings entry              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <QInputDialog>
#include "OpenRGBYeelightSettingsEntry.h"
#include "ui_OpenRGBYeelightSettingsEntry.h"

using namespace Ui;

OpenRGBYeelightSettingsEntry::OpenRGBYeelightSettingsEntry(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OpenRGBYeelightSettingsEntryUi)
{
    ui->setupUi(this);
}

OpenRGBYeelightSettingsEntry::~OpenRGBYeelightSettingsEntry()
{
    delete ui;
}

void OpenRGBYeelightSettingsEntry::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

