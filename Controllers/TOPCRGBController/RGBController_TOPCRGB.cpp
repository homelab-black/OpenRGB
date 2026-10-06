#include "RGBController_TOPCRGB.h"
#include "LogManager.h"

const char* TOPC_RGB_CONTROLLER_NAME = "RGBController_TOPCRGB";

/**------------------------------------------------------------------*\
    @name TOPC RGB Device
    @category LEDStrip
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectTOPCRGBControllers
    @comment The TOPC RGB controller drives a single addressable
        3-pin RGB header on the motherboard.  The firmware is
        driven exactly like the vendor reference implementation in
        backend/: the same colour buffer is mirrored to four
        feature reports (0x10..0x13), each followed by an apply
        report.  The device only supports direct per-LED control
        (plain 8-bit RGB, no brightness channel), so all dynamic
        effects are rendered host side by OpenRGB.  The LED count
        is fixed at 60, matching the reference code; there is no
        user supplied size.
\*-------------------------------------------------------------------*/

RGBController_TOPCRGB::RGBController_TOPCRGB(TOPCRGBController* controller_ptr)
{
    controller  = controller_ptr;

    name        = "TOPC RGB Controller";
    vendor      = "TOPC";
    type        = DEVICE_TYPE_LEDSTRIP;
    description = "TOPC RGB Controller Device";
    location    = controller->GetDeviceLocation();
    serial      = "";

    LOG_DEBUG("[%s] Constructing controller '%s' at %s",
              TOPC_RGB_CONTROLLER_NAME, name.c_str(), location.c_str());

    /*-----------------------------------------------------*\
    | Define the Direct mode.  The device only performs      |
    | direct per-LED control, so this is the only mode the   |
    | hardware itself provides.  MODE_FLAG_AUTOMATIC_SAVE is |
    | used because colours are pushed to the device          |
    | immediately and the hardware has no persistent storage |
    | (without it the UI shows "Saving Not Supported").      |
    \*-----------------------------------------------------*/
    mode Direct;
    Direct.name                   = "Direct";
    Direct.value                  = 0;
    Direct.flags                  = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Direct.color_mode             = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();

    active_mode = 0;

    LOG_DEBUG("[%s] Created %u zone(s) and %u LED(s)",
              TOPC_RGB_CONTROLLER_NAME, (unsigned int)zones.size(), (unsigned int)leds.size());
}

RGBController_TOPCRGB::~RGBController_TOPCRGB()
{
    LOG_DEBUG("[%s] Destructing controller '%s'", TOPC_RGB_CONTROLLER_NAME, name.c_str());

    Shutdown();

    delete controller;
}

void RGBController_TOPCRGB::SetupZones()
{
    LOG_DEBUG("[%s] SetupZones called", TOPC_RGB_CONTROLLER_NAME);

    /*-----------------------------------------------------*\
    | Clear any existing color/LED configuration             |
    \*-----------------------------------------------------*/
    leds.clear();
    colors.clear();

    /*-----------------------------------------------------*\
    | A single fixed-size addressable zone.  The size is     |
    | not user configurable; it always matches the reference |
    | implementation's 60 LEDs.                              |
    \*-----------------------------------------------------*/
    zones.resize(1);

    zones[0].name                   = "Addressable RGB Header";
    zones[0].type                   = ZONE_TYPE_LINEAR;
    zones[0].leds_min               = TOPC_RGB_LED_COUNT;
    zones[0].leds_max               = TOPC_RGB_LED_COUNT;
    zones[0].leds_count             = TOPC_RGB_LED_COUNT;

    for(unsigned int led_idx = 0; led_idx < zones[0].leds_count; led_idx++)
    {
        led new_led;
        new_led.name            = zones[0].name;
        new_led.name.append(", LED ");
        new_led.name.append(std::to_string(led_idx + 1));
        new_led.value           = 0;

        leds.push_back(new_led);
    }

    LOG_DEBUG("[%s] SetupZones done: 1 zone, %u LEDs", TOPC_RGB_CONTROLLER_NAME, (unsigned int)leds.size());

    SetupColors();
}

void RGBController_TOPCRGB::DeviceUpdateLEDs()
{
    LOG_DEBUG("[%s] DeviceUpdateLEDs (active_mode=%d)", TOPC_RGB_CONTROLLER_NAME, active_mode);

    DeviceUpdateZoneLEDs(0);
}

void RGBController_TOPCRGB::DeviceUpdateZoneLEDs(int zone)
{
    /*-----------------------------------------------------*\
    | Only Direct mode drives the hardware from the per-LED  |
    | color buffer.                                          |
    \*-----------------------------------------------------*/
    if((active_mode < 0) || ((unsigned int)active_mode >= modes.size()))
    {
        LOG_WARNING("[%s] DeviceUpdateZoneLEDs: invalid active_mode %d",
                    TOPC_RGB_CONTROLLER_NAME, active_mode);
        return;
    }

    if(modes[active_mode].color_mode != MODE_COLORS_PER_LED)
    {
        LOG_WARNING("[%s] DeviceUpdateZoneLEDs: active mode '%s' color_mode %d is not per-LED",
                    TOPC_RGB_CONTROLLER_NAME, modes[active_mode].name.c_str(), (int)modes[active_mode].color_mode);
        return;
    }

    if((zone < 0) || ((std::size_t)zone >= zones.size()))
    {
        LOG_ERROR("[%s] DeviceUpdateZoneLEDs: invalid zone %d", TOPC_RGB_CONTROLLER_NAME, zone);
        return;
    }

    LOG_DEBUG("[%s] DeviceUpdateZoneLEDs(%d): pushing %u colour(s) to hardware",
              TOPC_RGB_CONTROLLER_NAME, zone, zones[zone].leds_count);

    controller->SetLEDs(zones[zone].colors, zones[zone].leds_count);
}

void RGBController_TOPCRGB::DeviceUpdateSingleLED(int led)
{
    if((unsigned int)led >= leds.size())
    {
        LOG_ERROR("[%s] DeviceUpdateSingleLED: invalid LED %d (have %u)",
                  TOPC_RGB_CONTROLLER_NAME, led, (unsigned int)leds.size());
        return;
    }

    LOG_DEBUG("[%s] DeviceUpdateSingleLED(%d): mapped to channel %u",
              TOPC_RGB_CONTROLLER_NAME, led, (unsigned int)leds[led].value);

    /*-----------------------------------------------------*\
    | Route through the public UpdateZoneLEDs() so the base  |
    | class locking / signalling is honoured, exactly like   |
    | the Cooler Master ARGB controller does.                |
    \*-----------------------------------------------------*/
    UpdateZoneLEDs((int)leds[led].value);
}

void RGBController_TOPCRGB::DeviceUpdateMode()
{
    LOG_DEBUG("[%s] DeviceUpdateMode", TOPC_RGB_CONTROLLER_NAME);

    DeviceUpdateLEDs();
}
