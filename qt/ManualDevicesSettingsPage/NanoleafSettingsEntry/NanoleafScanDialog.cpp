/*---------------------------------------------------------*\
| NanoleafScanDialog.cpp                               |
|                                                           |
|   User interface for  Nanoleaf scan & pairing page |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "NanoleafScanDialog.h"
#include "ui_NanoleafScanDialog.h"
#include "NanoleafController.h"

#include "NanoleafNewDeviceDialog.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "LogManager.h"

using json = nlohmann::json;

NanoleafScanDialog::NanoleafScanDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NanoleafScanDialog)
{
    ui->setupUi(this);
    on_NanoleafDeviceList_itemSelectionChanged(); // Refresh button state
}

NanoleafScanDialog::~NanoleafScanDialog()
{
    delete ui;
}

void NanoleafScanDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void NanoleafScanDialog::on_AddNanoleafDeviceButton_clicked()
{
    /*-----------------------------------------------------*\
    | Open a popup to manually add a device by setting ip   |
    | and port                                              |
    \*-----------------------------------------------------*/
    NanoleafNewDeviceDialog dialog;
    NanoleafDevice device = dialog.show();
    if(!device.ip.empty())
    {
        LOG_TRACE("[%s] Add %s:%d", "Nanoleaf", device.ip.c_str(), device.port);
        std::string location = device.ip+":"+std::to_string(device.port);

        if(entries.find(location) == entries.end())
        {
            NanoleafSettingsEntry* entry = new NanoleafSettingsEntry(QString::fromUtf8(device.ip.c_str()), device.port);

            entries[location] = entry;

            QListWidgetItem* item = new QListWidgetItem;

            item->setSizeHint(entry->sizeHint());

            ui->NanoleafDeviceList->addItem(item);
            ui->NanoleafDeviceList->setItemWidget(item, entry);
            ui->NanoleafDeviceList->show();

            std::optional<NanoleafSettings> cfg_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettings<NanoleafSettings>(NANOLEAF_CFG_KEY);
            NanoleafSettings settings = cfg_settings.value_or(NanoleafSettings {});
            NanoleafDeviceSettings new_device {
                .ip = device.ip,
                .port = device.port,
                .auth_token = ""
            };

            settings.devices.insert({location, new_device});
            ResourceManager::get()->GetSettingsManager()->SetControllerSettingsAndSave(NANOLEAF_CFG_KEY, settings);
        }
    }
}

void NanoleafScanDialog::on_RemoveNanoleafDeviceButton_clicked()
{
    /*-------------------------------------------------*\
    | Remove the selected device                        |
    \*-------------------------------------------------*/
    int cur_row = ui->NanoleafDeviceList->currentRow();

    if(cur_row < 0)
    {
        return;
    }

    QListWidgetItem* item = ui->NanoleafDeviceList->item(cur_row);
    NanoleafSettingsEntry* entry = (NanoleafSettingsEntry*) ui->NanoleafDeviceList->itemWidget(item);

    ui->NanoleafDeviceList->removeItemWidget(item);
    delete item;

    std::string location = entry->getLocation();
    delete entries[location];
    entries.erase(location);

    std::optional<NanoleafSettings> cfg_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettings<NanoleafSettings>(NANOLEAF_CFG_KEY);
    NanoleafSettings settings = cfg_settings.value_or(NanoleafSettings {});
    settings.devices.erase(location);
    ResourceManager::get()->GetSettingsManager()->SetControllerSettingsAndSave(NANOLEAF_CFG_KEY, settings);
}

void NanoleafScanDialog::on_ScanForNanoleafDevicesButton_clicked()
{
    /*-----------------------------------------------------*\
    | Create a worker thread for the mDNS query and hookup  |
    | callbacks for when it finds devices                   |
    \*-----------------------------------------------------*/
    NanoleafScanningThread *scanThread = new NanoleafScanningThread;

    connect(scanThread, SIGNAL(DeviceFound(QString, int)),
                        SLOT(on_DeviceFound(QString, int)));

    connect(scanThread, SIGNAL(finished()),
            scanThread, SLOT(deleteLater()));

    scanThread->start();
}

void NanoleafScanDialog::on_DeviceFound(QString address, int port)
{
    std::string location = address.toStdString()+":"+std::to_string(port);

    if(entries.find(location) == entries.end())
    {
        NanoleafSettingsEntry* entry = new NanoleafSettingsEntry(address, port);

        entries[location] = entry;

        QListWidgetItem* item = new QListWidgetItem;

        item->setSizeHint(entry->sizeHint());

        ui->NanoleafDeviceList->addItem(item);
        ui->NanoleafDeviceList->setItemWidget(item, entry);
        ui->NanoleafDeviceList->show();
    }
}

void NanoleafScanDialog::on_NanoleafDeviceList_itemSelectionChanged()
{
    int cur_row = ui->NanoleafDeviceList->currentRow();

    bool anySelected = (cur_row >= 0);
    ui->RemoveNanoleafDeviceButton->setEnabled(anySelected);
}

