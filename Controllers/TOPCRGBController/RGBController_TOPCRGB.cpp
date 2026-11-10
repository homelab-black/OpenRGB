#include <cmath>
#include "RGBController_TOPCRGB.h"
#include "LogManager.h"

using namespace std::chrono_literals;

const char* TOPC_RGB_CONTROLLER_NAME = "RGBController_TOPCRGB";

/*---------------------------------------------------------*\
| Speed / brightness ranges used by the software effects.    |
\*---------------------------------------------------------*/
#define TOPC_RGB_SPEED_SLOWEST              1
#define TOPC_RGB_SPEED_DEFAULT              5
#define TOPC_RGB_SPEED_FASTEST              10
#define TOPC_RGB_BRIGHTNESS_MIN             0
#define TOPC_RGB_BRIGHTNESS_MAX             100

/*---------------------------------------------------------*\
| Framerate of the software effect renderer (~30 fps).       |
\*---------------------------------------------------------*/
#define TOPC_RGB_EFFECT_PERIOD_MS           33

/**------------------------------------------------------------------*\
    @name TOPC RGB Device
    @category LEDStrip
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectTOPCRGBControllers
    @comment The TOPC RGB controller drives a single addressable
        3-pin RGB header on the motherboard.  The firmware is
        driven exactly like the vendor reference implementation in
        backend/: the same colour buffer is mirrored to four
        feature reports (0x10..0x13), each followed by an apply
        report.  The device only supports direct per-LED control
        (plain 8-bit RGB, no brightness channel), so every dynamic
        effect is rendered host side by a background thread and
        pushed as raw per-LED frames, mirroring backend/effects/.
        The number of connected LEDs is user configurable in the UI
        (resizable zone, 1..60, default TOPC_RGB_LED_COUNT).
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
    | The device has no hardware effect engine, so every     |
    | mode is rendered host side by EffectThreadFunction().  |
    | MODE_FLAG_AUTOMATIC_SAVE is used because colours are   |
    | pushed to the device immediately and the hardware has  |
    | no persistent storage (without it the UI shows         |
    | "Saving Not Supported").                               |
    \*-----------------------------------------------------*/
    mode Direct;
    Direct.name                   = "Direct";
    Direct.value                  = TOPC_RGB_MODE_DIRECT;
    Direct.flags                  = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Direct.color_mode             = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode Static;
    Static.name                   = "Static";
    Static.value                  = TOPC_RGB_MODE_STATIC;
    Static.flags                  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Static.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min             = 1;
    Static.colors_max             = 1;
    Static.colors.resize(1);
    Static.brightness_min         = TOPC_RGB_BRIGHTNESS_MIN;
    Static.brightness_max         = TOPC_RGB_BRIGHTNESS_MAX;
    Static.brightness             = TOPC_RGB_BRIGHTNESS_MAX;
    modes.push_back(Static);

    mode RainbowWave;
    RainbowWave.name              = "Rainbow Wave";
    RainbowWave.value             = TOPC_RGB_MODE_RAINBOW_WAVE;
    RainbowWave.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    RainbowWave.color_mode        = MODE_COLORS_NONE;
    RainbowWave.speed_min         = TOPC_RGB_SPEED_SLOWEST;
    RainbowWave.speed_max         = TOPC_RGB_SPEED_FASTEST;
    RainbowWave.speed             = TOPC_RGB_SPEED_DEFAULT;
    RainbowWave.direction         = MODE_DIRECTION_RIGHT;
    RainbowWave.brightness_min    = TOPC_RGB_BRIGHTNESS_MIN;
    RainbowWave.brightness_max    = TOPC_RGB_BRIGHTNESS_MAX;
    RainbowWave.brightness        = TOPC_RGB_BRIGHTNESS_MAX;
    modes.push_back(RainbowWave);

    mode Breathing;
    Breathing.name                = "Breathing";
    Breathing.value               = TOPC_RGB_MODE_BREATHING;
    Breathing.flags               = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing.color_mode          = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min          = 1;
    Breathing.colors_max          = 2;
    Breathing.colors.resize(1);
    Breathing.colors[0]           = ToRGBColor(255, 0, 0);
    Breathing.speed_min           = TOPC_RGB_SPEED_SLOWEST;
    Breathing.speed_max           = TOPC_RGB_SPEED_FASTEST;
    Breathing.speed               = TOPC_RGB_SPEED_DEFAULT;
    Breathing.brightness_min      = TOPC_RGB_BRIGHTNESS_MIN;
    Breathing.brightness_max      = TOPC_RGB_BRIGHTNESS_MAX;
    Breathing.brightness          = TOPC_RGB_BRIGHTNESS_MAX;
    modes.push_back(Breathing);

    mode Meteor;
    Meteor.name                   = "Meteor";
    Meteor.value                  = TOPC_RGB_MODE_METEOR;
    Meteor.flags                  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Meteor.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Meteor.colors_min             = 1;
    Meteor.colors_max             = 2;
    Meteor.colors.resize(1);
    Meteor.colors[0]              = ToRGBColor(255, 0, 0);
    Meteor.speed_min              = TOPC_RGB_SPEED_SLOWEST;
    Meteor.speed_max              = TOPC_RGB_SPEED_FASTEST;
    Meteor.speed                  = TOPC_RGB_SPEED_DEFAULT;
    Meteor.brightness_min         = TOPC_RGB_BRIGHTNESS_MIN;
    Meteor.brightness_max         = TOPC_RGB_BRIGHTNESS_MAX;
    Meteor.brightness             = TOPC_RGB_BRIGHTNESS_MAX;
    modes.push_back(Meteor);

    mode Blink;
    Blink.name                    = "Blink";
    Blink.value                   = TOPC_RGB_MODE_BLINK;
    Blink.flags                   = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Blink.color_mode              = MODE_COLORS_MODE_SPECIFIC;
    Blink.colors_min              = 1;
    Blink.colors_max              = 2;
    Blink.colors.resize(1);
    Blink.colors[0]               = ToRGBColor(255, 0, 0);
    Blink.speed_min               = TOPC_RGB_SPEED_SLOWEST;
    Blink.speed_max               = TOPC_RGB_SPEED_FASTEST;
    Blink.speed                   = TOPC_RGB_SPEED_DEFAULT;
    Blink.brightness_min          = TOPC_RGB_BRIGHTNESS_MIN;
    Blink.brightness_max          = TOPC_RGB_BRIGHTNESS_MAX;
    Blink.brightness              = TOPC_RGB_BRIGHTNESS_MAX;
    modes.push_back(Blink);

    SetupZones();

    active_mode = 0;

    /*-----------------------------------------------------*\
    | Start the software effect renderer                     |
    \*-----------------------------------------------------*/
    effect_step          = 0;
    effect_thread_active = false;
    effect_thread_run    = true;
    effect_thread        = new std::thread(&RGBController_TOPCRGB::EffectThreadFunction, this);

    LOG_DEBUG("[%s] Created %u zone(s), %u LED(s), %u mode(s)",
              TOPC_RGB_CONTROLLER_NAME, (unsigned int)zones.size(), (unsigned int)leds.size(), (unsigned int)modes.size());
}

RGBController_TOPCRGB::~RGBController_TOPCRGB()
{
    LOG_DEBUG("[%s] Destructing controller '%s'", TOPC_RGB_CONTROLLER_NAME, name.c_str());

    Shutdown();

    /*-----------------------------------------------------*\
    | Stop the software effect renderer before tearing down  |
    \*-----------------------------------------------------*/
    effect_thread_run = false;

    if(effect_thread != nullptr)
    {
        if(effect_thread->joinable())
        {
            effect_thread->join();
        }

        delete effect_thread;
        effect_thread = nullptr;
    }

    delete controller;
}

void RGBController_TOPCRGB::SetupZones()
{
    LOG_DEBUG("[%s] SetupZones called", TOPC_RGB_CONTROLLER_NAME);

    /*-----------------------------------------------------*\
    | Only set the LED count on the first run.  On later     |
    | calls (triggered by DeviceConfigureZone when the user  |
    | resizes the zone) the persisted count is honoured.     |
    \*-----------------------------------------------------*/
    bool first_run = false;

    if(zones.size() == 0)
    {
        first_run = true;
    }

    /*-----------------------------------------------------*\
    | Clear any existing color/LED configuration             |
    \*-----------------------------------------------------*/
    leds.clear();
    colors.clear();

    /*-----------------------------------------------------*\
    | A single linear zone.  It is user resizable through    |
    | the "Edit Zone" UI (the button becomes "Edit Zone"     |
    | while this zone is selected), between 1 and            |
    | TOPC_RGB_LEDS_PER_REPORT (60) LEDs, matching the       |
    | largest count the firmware report can carry.  The      |
    | default is TOPC_RGB_LED_COUNT (60).                    |
    \*-----------------------------------------------------*/
    if(first_run)
    {
        zone new_zone;

        new_zone.flags = ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE
                       | ZONE_FLAG_MANUALLY_CONFIGURABLE_NAME
                       | ZONE_FLAG_MANUALLY_CONFIGURABLE_TYPE;

        zones.push_back(new_zone);
    }

    if(!(zones[0].flags & ZONE_FLAG_MANUALLY_CONFIGURED_NAME))
    {
        zones[0].name               = "Addressable RGB Header";
    }

    zones[0].leds_min               = 1;
    zones[0].leds_max               = TOPC_RGB_LEDS_PER_REPORT;

    if(!(zones[0].flags & ZONE_FLAG_MANUALLY_CONFIGURED_SIZE))
    {
        zones[0].leds_count         = TOPC_RGB_LED_COUNT;
    }

    if(!(zones[0].flags & ZONE_FLAG_MANUALLY_CONFIGURED_TYPE))
    {
        zones[0].type               = ZONE_TYPE_LINEAR;
    }

    /*-----------------------------------------------------*\
    | Clamp a stored size that is out of range               |
    \*-----------------------------------------------------*/
    if(zones[0].leds_count < zones[0].leds_min)
    {
        zones[0].leds_count         = zones[0].leds_min;
    }

    if(zones[0].leds_count > zones[0].leds_max)
    {
        zones[0].leds_count         = zones[0].leds_max;
    }

    for(unsigned int led_idx = 0; led_idx < zones[0].leds_count; led_idx++)
    {
        led new_led;
        new_led.name            = zones[0].name;
        new_led.name.append(", LED ");
        new_led.name.append(std::to_string(led_idx + 1));
        new_led.value           = 0;

        leds.push_back(new_led);
    }

    LOG_DEBUG("[%s] SetupZones done: 1 zone, %u LEDs (min %u, max %u)",
              TOPC_RGB_CONTROLLER_NAME,
              (unsigned int)leds.size(),
              zones[0].leds_min,
              zones[0].leds_max);

    SetupColors();
}

/*---------------------------------------------------------*\
| Called by the UI when the user changes the zone size.  Re- |
| run SetupZones() so the leds/colors arrays are rebuilt for |
| the new count.  The new count is already stored in the     |
| zone (and flagged as manually configured) by the UI, so    |
| SetupZones() will honour it.                               |
\*---------------------------------------------------------*/
void RGBController_TOPCRGB::DeviceConfigureZone(int zone_idx)
{
    LOG_DEBUG("[%s] DeviceConfigureZone(%d)", TOPC_RGB_CONTROLLER_NAME, zone_idx);

    if((zone_idx >= 0) && ((std::size_t)zone_idx < zones.size()))
    {
        LOG_DEBUG("[%s]   new leds_count %u",
                  TOPC_RGB_CONTROLLER_NAME, (unsigned int)zones[zone_idx].leds_count);
    }

    SetupZones();
}

void RGBController_TOPCRGB::DeviceUpdateLEDs()
{
    LOG_DEBUG("[%s] DeviceUpdateLEDs (active_mode=%d)", TOPC_RGB_CONTROLLER_NAME, active_mode);

    /*-----------------------------------------------------*\
    | In Direct mode the UI supplies per-LED colors, so push |
    | them straight away.  Effect modes are rendered on the  |
    | background thread and must not be overridden here.     |
    \*-----------------------------------------------------*/
    if((active_mode >= 0) && ((unsigned int)active_mode < modes.size())
    && (modes[active_mode].value == TOPC_RGB_MODE_DIRECT))
    {
        controller->SetLEDs(zones[0].colors, zones[0].leds_count);
    }
}

void RGBController_TOPCRGB::DeviceUpdateZoneLEDs(int zone)
{
    (void)zone;

    DeviceUpdateLEDs();
}

void RGBController_TOPCRGB::DeviceUpdateSingleLED(int led)
{
    (void)led;

    DeviceUpdateLEDs();
}

void RGBController_TOPCRGB::DeviceUpdateMode()
{
    LOG_DEBUG("[%s] DeviceUpdateMode (active_mode=%d)", TOPC_RGB_CONTROLLER_NAME, active_mode);

    effect_step = 0;

    /*-----------------------------------------------------*\
    | Enable the software renderer for effect modes and      |
    | disable it for Direct (the UI drives the LEDs there).  |
    \*-----------------------------------------------------*/
    effect_thread_active = (modes[active_mode].value != TOPC_RGB_MODE_DIRECT);

    DeviceUpdateLEDs();
}

/*---------------------------------------------------------*\
| Software effect renderer                                   |
|                                                            |
| Renders the active mode into effect_colors and pushes the  |
| frame to the device.  This mirrors the vendor reference    |
| code in backend/effects/, which performs all animation     |
| host side because the hardware has no effect engine.       |
\*---------------------------------------------------------*/
void RGBController_TOPCRGB::EffectThreadFunction()
{
    while(effect_thread_run.load())
    {
        if(effect_thread_active.load())
        {
            if((active_mode >= 0) && ((unsigned int)active_mode < modes.size()))
            {
                switch(modes[active_mode].value)
                {
                    case TOPC_RGB_MODE_STATIC:
                        RenderStatic();
                        break;

                    case TOPC_RGB_MODE_RAINBOW_WAVE:
                        RenderRainbowWave();
                        break;

                    case TOPC_RGB_MODE_BREATHING:
                        RenderBreathing();
                        break;

                    case TOPC_RGB_MODE_METEOR:
                        RenderMeteor();
                        break;

                    case TOPC_RGB_MODE_BLINK:
                        RenderBlink();
                        break;

                    default:
                        break;
                }

                if(!effect_colors.empty())
                {
                    controller->SetLEDs(effect_colors.data(), (unsigned int)effect_colors.size());
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(TOPC_RGB_EFFECT_PERIOD_MS));
    }
}

void RGBController_TOPCRGB::RenderStatic()
{
    if(effect_colors.size() != zones[0].leds_count)
    {
        effect_colors.resize(zones[0].leds_count);
    }

    RGBColor    color       = modes[active_mode].colors.empty() ? ToRGBColor(255, 0, 0) : modes[active_mode].colors[0];
    float       brightness  = modes[active_mode].brightness / (float)TOPC_RGB_BRIGHTNESS_MAX;

    unsigned char r = (unsigned char)(RGBGetRValue(color) * brightness);
    unsigned char g = (unsigned char)(RGBGetGValue(color) * brightness);
    unsigned char b = (unsigned char)(RGBGetBValue(color) * brightness);

    for(std::size_t led = 0; led < effect_colors.size(); led++)
    {
        effect_colors[led] = ToRGBColor(r, g, b);
    }
}

void RGBController_TOPCRGB::RenderRainbowWave()
{
    if(effect_colors.size() != zones[0].leds_count)
    {
        effect_colors.resize(zones[0].leds_count);
    }

    float brightness = modes[active_mode].brightness / (float)TOPC_RGB_BRIGHTNESS_MAX;
    int   speed      = (int)modes[active_mode].speed;

    /*-----------------------------------------------------*\
    | Hue advances along the strip and over time, producing  |
    | a moving rainbow (flowing) effect.                     |
    \*-----------------------------------------------------*/
    float step_scale = 0.02f * (float)speed;

    for(unsigned int led = 0; led < zones[0].leds_count; led++)
    {
        float hue = ((float)led / (float)zones[0].leds_count);
        hue += (float)effect_step * step_scale;

        if(modes[active_mode].direction == MODE_DIRECTION_LEFT)
        {
            hue = -hue;
        }

        hue = hue - std::floor(hue);    /* wrap to 0..1 */

        unsigned char r;
        unsigned char g;
        unsigned char b;

        HSVToRGB(hue, 1.0f, brightness, &r, &g, &b);

        effect_colors[led] = ToRGBColor(r, g, b);
    }

    effect_step++;
}

void RGBController_TOPCRGB::RenderBreathing()
{
    if(effect_colors.size() != zones[0].leds_count)
    {
        effect_colors.resize(zones[0].leds_count);
    }

    RGBColor    color           = modes[active_mode].colors.empty() ? ToRGBColor(255, 0, 0) : modes[active_mode].colors[0];
    float       brightness      = modes[active_mode].brightness / (float)TOPC_RGB_BRIGHTNESS_MAX;
    unsigned int speed          = modes[active_mode].speed;

    /*-----------------------------------------------------*\
    | 0..1..0 sine wave, matching backend/effects/breathing. |
    | The higher the speed, the faster the wave completes.   |
    \*-----------------------------------------------------*/
    unsigned int period         = (unsigned int)(60.0f / (float)speed);
    float        phase          = (float)(effect_step % period) / (float)period;
    float        intensity      = 0.2f + 0.8f * std::pow(std::sin(3.14159265f * phase), 1.8f);

    intensity *= brightness;

    unsigned char r = (unsigned char)(RGBGetRValue(color) * intensity);
    unsigned char g = (unsigned char)(RGBGetGValue(color) * intensity);
    unsigned char b = (unsigned char)(RGBGetBValue(color) * intensity);

    for(std::size_t led = 0; led < effect_colors.size(); led++)
    {
        effect_colors[led] = ToRGBColor(r, g, b);
    }

    effect_step++;
}

void RGBController_TOPCRGB::RenderMeteor()
{
    if(effect_colors.size() != zones[0].leds_count)
    {
        effect_colors.resize(zones[0].leds_count);
    }

    RGBColor    color       = modes[active_mode].colors.empty() ? ToRGBColor(255, 0, 0) : modes[active_mode].colors[0];
    float       brightness  = modes[active_mode].brightness / (float)TOPC_RGB_BRIGHTNESS_MAX;
    unsigned int speed      = modes[active_mode].speed;

    /*-----------------------------------------------------*\
    | A short comet of LEDs moves along the strip, mirroring |
    | backend/effects/meteor.py.  The tail uses descending   |
    | brightness levels.                                     |
    \*-----------------------------------------------------*/
    const int tail_levels[] = { 3, 3, 2, 1, 1 };
    const int tail_len      = (int)(sizeof(tail_levels) / sizeof(tail_levels[0]));
    const int max_level     = 3;

    unsigned int led_count  = zones[0].leds_count;

    /*-----------------------------------------------------*\
    | The comet advances once per render tick; speed scales  |
    | how far it moves so higher speeds sweep the strip      |
    | faster.                                                |
    \*-----------------------------------------------------*/
    int          position   = (int)((effect_step * speed) % (led_count + tail_len)) - tail_len;

    RGBColor background     = ToRGBColor(0, 0, 0);

    for(unsigned int led = 0; led < led_count; led++)
    {
        effect_colors[led] = background;
    }

    for(int offset = 0; offset < tail_len; offset++)
    {
        int led_index = position - offset;

        if((led_index >= 0) && (led_index < (int)led_count))
        {
            float scale = (float)tail_levels[offset] / (float)max_level * brightness;

            unsigned char r = (unsigned char)(RGBGetRValue(color) * scale);
            unsigned char g = (unsigned char)(RGBGetGValue(color) * scale);
            unsigned char b = (unsigned char)(RGBGetBValue(color) * scale);

            effect_colors[led_index] = ToRGBColor(r, g, b);
        }
    }

    effect_step++;
}

void RGBController_TOPCRGB::RenderBlink()
{
    if(effect_colors.size() != zones[0].leds_count)
    {
        effect_colors.resize(zones[0].leds_count);
    }

    RGBColor    color       = modes[active_mode].colors.empty() ? ToRGBColor(255, 0, 0) : modes[active_mode].colors[0];
    float       brightness  = modes[active_mode].brightness / (float)TOPC_RGB_BRIGHTNESS_MAX;
    unsigned int speed      = modes[active_mode].speed;

    /*-----------------------------------------------------*\
    | Alternate on/off blocks, matching backend/effects/     |
    | blinking.py.  Higher speed means shorter on/off times. |
    \*-----------------------------------------------------*/
    unsigned int half_period = (unsigned int)(30.0f / (float)speed);
    bool         on          = ((effect_step / half_period) % 2) == 0;

    unsigned char r = on ? (unsigned char)(RGBGetRValue(color) * brightness) : 0;
    unsigned char g = on ? (unsigned char)(RGBGetGValue(color) * brightness) : 0;
    unsigned char b = on ? (unsigned char)(RGBGetBValue(color) * brightness) : 0;

    for(std::size_t led = 0; led < effect_colors.size(); led++)
    {
        effect_colors[led] = ToRGBColor(r, g, b);
    }

    effect_step++;
}

/*---------------------------------------------------------*\
| HSV to RGB conversion, hue in 0..1                         |
\*---------------------------------------------------------*/
void RGBController_TOPCRGB::HSVToRGB(float h, float s, float v, unsigned char* r, unsigned char* g, unsigned char* b)
{
    if(s <= 0.0f)
    {
        *r = (unsigned char)(v * 255.0f);
        *g = (unsigned char)(v * 255.0f);
        *b = (unsigned char)(v * 255.0f);
        return;
    }

    h = h - std::floor(h);

    float h6    = h * 6.0f;
    int   i     = (int)std::floor(h6);
    float f     = h6 - (float)i;
    float p     = v * (1.0f - s);
    float q     = v * (1.0f - (s * f));
    float t     = v * (1.0f - (s * (1.0f - f)));

    float rf;
    float gf;
    float bf;

    switch(i % 6)
    {
        case 0: rf = v; gf = t; bf = p; break;
        case 1: rf = q; gf = v; bf = p; break;
        case 2: rf = p; gf = v; bf = t; break;
        case 3: rf = p; gf = q; bf = v; break;
        case 4: rf = t; gf = p; bf = v; break;
        default: rf = v; gf = p; bf = q; break;
    }

    *r = (unsigned char)(rf * 255.0f);
    *g = (unsigned char)(gf * 255.0f);
    *b = (unsigned char)(bf * 255.0f);
}
