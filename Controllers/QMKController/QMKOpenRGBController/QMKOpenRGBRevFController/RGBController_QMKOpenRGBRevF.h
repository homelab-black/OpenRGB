/*---------------------------------------------------------*\
| RGBController_QMKOpenRGBRevF.h                            |
|                                                           |
|   Driver for OpenRGB QMK Keyboard Protocol Revision F     |
|                                                           |
|   reklrekl                                    06 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include "RGBController.h"
#include "QMKOpenRGBRevFController.h"

#define NO_LED 0xFFFFFFFF

typedef std::vector<std::vector<unsigned int>> VectorMatrix;

class RGBController_QMKOpenRGBRevF : public RGBController
{
public:
    RGBController_QMKOpenRGBRevF(QMKOpenRGBRevFController* controller_ptr);
    ~RGBController_QMKOpenRGBRevF();

    void                                    SetupZones();
    void                                    ResizeZone(int zone, int new_size);

    void                                    DeviceUpdateLEDs();
    void                                    UpdateZoneLEDs(int zone);
    void                                    UpdateSingleLED(int led);

    void                                    DeviceUpdateMode();
    void                                    DeviceSaveMode();

private:
    QMKOpenRGBRevFController*   controller;
    std::vector<unsigned int>   flat_matrix_map;
    std::vector<unsigned int>   flat_underglow_map;

    void                                    InitializeMode
                                            (
                                            std::string name,
                                            unsigned int qmk_id,
                                            unsigned int flags,
                                            bool save
                                            );

    unsigned int                            CalculateDivisor
                                            (
                                            std::vector<point_t> led_points,
                                            std::set<int> rows,
                                            std::set<int> columns
                                            );

    void                                    PlaceLEDsInMaps
                                            (
                                                std::set<int>               unique_rows,
                                                std::set<int>               unique_cols,
                                                unsigned int                divisor,
                                                std::vector<point_t>        led_points,
                                                std::vector<unsigned int>   led_flags,
                                                VectorMatrix&               matrix_map_xl,
                                                VectorMatrix&               underglow_map_xl
                                            );

    VectorMatrix                            MakeEmptyMatrixMap
                                            (
                                                std::size_t height,
                                                std::size_t width
                                            );

    void                                    CleanMatrixMaps
                                            (
                                            VectorMatrix&   matrix_map,
                                            VectorMatrix&   underglow_map,
                                            unsigned int    height,
                                            bool            has_underglow
                                            );

    std::vector<unsigned int>               FlattenMatrixMap
                                            (
                                            VectorMatrix matrix_map
                                            );
};
