#pragma once

#include <atomic>
#include <thread>
#include "RGBController.h"
#include "TOPCRGBController.h"

/*---------------------------------------------------------*\
| Software effect modes.  The device has no hardware effect  |
| engine, so every dynamic mode is rendered host side by a   |
| background thread and pushed to the hardware as raw per-LED |
| RGB frames, exactly like the vendor reference code in      |
| backend/effects/.  The values are arbitrary (only used     |
| internally to select the renderer).                        |
\*---------------------------------------------------------*/
enum
{
    TOPC_RGB_MODE_DIRECT        = 0,    /* User supplied per-LED colors  */
    TOPC_RGB_MODE_STATIC        = 1,    /* Single fixed color            */
    TOPC_RGB_MODE_RAINBOW_WAVE  = 2,    /* Moving rainbow (flowing)      */
    TOPC_RGB_MODE_BREATHING     = 3,    /* Breathing / fade              */
    TOPC_RGB_MODE_METEOR        = 4,    /* Meteor / comet wipe           */
    TOPC_RGB_MODE_BLINK         = 5,    /* Blink on/off                  */
};

class RGBController_TOPCRGB : public RGBController
{
public:
    RGBController_TOPCRGB(TOPCRGBController* controller_ptr);
    ~RGBController_TOPCRGB();

    void        SetupZones();
    void        DeviceConfigureZone(int zone_idx);

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);
    void        DeviceUpdateMode();

private:
    TOPCRGBController*  controller;

    /*-----------------------------------------------------*\
    | Software effect renderer                              |
    \*-----------------------------------------------------*/
    std::thread*        effect_thread;
    std::atomic<bool>   effect_thread_run;
    std::atomic<bool>   effect_thread_active;

    std::vector<RGBColor>   effect_colors;      /* Scratch per-LED frame buffer  */
    unsigned int            effect_step;        /* Animation frame counter       */

    void        EffectThreadFunction();
    void        RenderStatic();
    void        RenderRainbowWave();
    void        RenderBreathing();
    void        RenderMeteor();
    void        RenderBlink();

    static void HSVToRGB(float h, float s, float v, unsigned char* r, unsigned char* g, unsigned char* b);
};
