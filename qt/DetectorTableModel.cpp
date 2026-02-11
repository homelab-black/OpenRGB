/*---------------------------------------------------------*\
| DetectorTableModel.cpp                                    |
|                                                           |
|   Table model for detector list                           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectorTableModel.h"
#include "SettingsManager.h"

DetectorTableModel::DetectorTableModel(QObject* parent) : QAbstractTableModel(parent)
{
    detectors.clear();

    /*-----------------------------------------------------*\
    | Read the detector list from the settings manager      |
    \*-----------------------------------------------------*/
    SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
    const DetectorSettings& detector_settings = settings.access().detectors;

    /*---------------------------------------------------------*\
    | Using foreach loop since we're iterating over a map here  |
    \*---------------------------------------------------------*/
    for (const auto& [detector_str, enabled] : detector_settings.detectors)
    {
        DetectorTableValue new_entry {
            .key = detector_str,
            .value = enabled,
        };
        detectors.push_back(new_entry);
    }
    settings.Release();

    /*-----------------------------------------------------*\
    | If settings contains the detectors list, fill in rows |
    \*-----------------------------------------------------*/
    beginInsertRows(QModelIndex(), 0, (int)detectors.size());
    endInsertRows();
}

int DetectorTableModel::columnCount(const QModelIndex&) const
{
    /*-----------------------------------------------------*\
    | The table has two columns - detector name and enable  |
    \*-----------------------------------------------------*/
    return(2);
}

int DetectorTableModel::rowCount(const QModelIndex&) const
{
    /*-----------------------------------------------------*\
    | The number of rows is equal to the number of detectors|
    \*-----------------------------------------------------*/
    return((int)detectors.size());
}

QVariant DetectorTableModel::data(const QModelIndex& index, int role) const
{
    switch(role)
    {
        /*-----------------------------------------------------*\
        | Column 0 is the detector name, 1 is the enable flag   |
        \*-----------------------------------------------------*/
        case Qt::DisplayRole:
            switch(index.column())
            {
                case 0:
                    return(detectors[index.row()].key.c_str());
                case 1:
                    return(detectors[index.row()].value);
            }
            return(QVariant());

        case Qt::CheckStateRole:
            switch(index.column())
            {
                case 1:
                    return(2 * detectors[index.row()].value);
            }
            return(QVariant());
    }
    return(QVariant());
}

bool DetectorTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    /*-----------------------------------------------------*\
    | Update detector value for column 1                    |
    \*-----------------------------------------------------*/
    if(index.column() == 1 && role == Qt::CheckStateRole)
    {
        detectors[index.row()].value = value.toBool();
        emit dataChanged(index, index);
    }
    return(false);
}

QVariant DetectorTableModel::headerData(int index, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole)
    {
        switch(orientation)
        {
            case Qt::Vertical:
                return(index + 1);

            case Qt::Horizontal:
                switch(index)
                {
                    case 0:
                        return(tr("Name"));
                    case 1:
                        return(tr("Enabled"));
                }
        }
    }
    return(QVariant());
}

Qt::ItemFlags DetectorTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags fl = Qt::ItemIsEnabled;

    if(index.column() == 1)
    {
        fl |= Qt::ItemIsUserCheckable;
    }

    return(fl);
}

void DetectorTableModel::applySettings()
{
    /*-----------------------------------------------------*\
    | Read the detector list from the settings manager      |
    \*-----------------------------------------------------*/
    SettingsAccessGuard settings = ResourceManager::get()->GetSettingsManager()->GetSettings();
    DetectorSettings& detector_settings = settings.access_mut().detectors;

    /*-----------------------------------------------------*\
    | Loop through all detectors in the list and update the |
    | value in the settings                                 |
    \*-----------------------------------------------------*/
    for(unsigned int detector_idx = 0; detector_idx < detectors.size(); detector_idx++)
    {
        const DetectorTableValue& pair = detectors[detector_idx];
        detector_settings.detectors[pair.key] = pair.value;
    }

    /*-----------------------------------------------------*\
    | Set and save the settings                             |
    \*-----------------------------------------------------*/
    settings.Save();
}

void DetectorTableModel::toggleAll(const bool state, QSortFilterProxyModel* detectorSortModel)
{
    for(unsigned int detector_idx = 0; detector_idx < detectors.size(); detector_idx++)
    {
        if(detectorSortModel->mapFromSource(index(detector_idx, 0)).isValid())
        {
            setData(index(detector_idx,1), state, Qt::CheckStateRole);
        }
    }
}
