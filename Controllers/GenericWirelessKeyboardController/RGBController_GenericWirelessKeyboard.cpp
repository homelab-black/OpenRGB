#include "RGBController_GenericWirelessKeyboard.h"
#include "RGBControllerKeyNames.h"

using namespace std::chrono_literals;

#define NA 0xFFFFFFFF
#define WIDTH 18
#define HEIGHT 6

static unsigned int matrix_map[HEIGHT][WIDTH] = {
    {0, 6,  12, 18, 23, 28, 34, 39, 44, 50, 56, 62, 67, 71, 77, 83, 89, 95},
    {1, 7,  13, 19, 24, 29, 35, 40, 45, 51, 57, 63, 68, 72, 78, 84, 90, 96},
    {2, 8,  14, 20, 25, 30, 36, 41, 46, 52, 58, 64, 69, 73, 79, 85, 91, 97},
    {3, 9,  15, 21, 26, 31, 37, 42, 47, 53, 59, 65, NA, 74, 80, 86, 92, NA},
    {4, 10, 16, 22, 27, 32, 38, 43, 48, 54, 60, 66, NA, 75, 81, 87, 93, 98},
    {5, 11, 17, NA, NA, 33, NA, NA, 49, 55, 61, NA, 70, 76, 82, 88, 94, NA}
};

typedef struct
{
    const char         *name;
    const unsigned char idx;
} aula_f99_led_type;

static const aula_f99_led_type led_names[] = {
    {KEY_EN_ESCAPE,          0  }, // 0
    {KEY_EN_BACK_TICK,       1  },
    {KEY_EN_TAB,             2  },
    {KEY_EN_CAPS_LOCK,       3  },
    {KEY_EN_LEFT_SHIFT,      4  },
    {KEY_EN_LEFT_CONTROL,    5  },

    {KEY_EN_F1,              6  }, // 6
    {KEY_EN_1,               7  },
    {KEY_EN_Q,               8  },
    {KEY_EN_A,               9  },
    {KEY_EN_Z,               10 },
    {KEY_EN_LEFT_WINDOWS,    11 },

    {KEY_EN_F2,              12 }, // 12
    {KEY_EN_2,               13 },
    {KEY_EN_W,               14 },
    {KEY_EN_S,               15 },
    {KEY_EN_X,               16 },
    {KEY_EN_LEFT_ALT,        17 },

    {KEY_EN_F3,              18 }, // 18
    {KEY_EN_3,               19 },
    {KEY_EN_E,               20 },
    {KEY_EN_D,               21 },
    {KEY_EN_C,               22 },

    {KEY_EN_F4,              24 }, // 23
    {KEY_EN_4,               25 },
    {KEY_EN_R,               26 },
    {KEY_EN_F,               27 },
    {KEY_EN_V,               28 },

    {KEY_EN_F5,              30 }, // 28
    {KEY_EN_5,               31 },
    {KEY_EN_T,               32 },
    {KEY_EN_G,               33 },
    {KEY_EN_B,               34 },
    {KEY_EN_SPACE,           35 },

    {KEY_EN_F6,              36 }, // 34
    {KEY_EN_6,               37 },
    {KEY_EN_Y,               38 },
    {KEY_EN_H,               39 },
    {KEY_EN_N,               40 },

    {KEY_EN_F7,              42 }, // 39
    {KEY_EN_7,               43 },
    {KEY_EN_U,               44 },
    {KEY_EN_J,               45 },
    {KEY_EN_M,               46 },

    {KEY_EN_F8,              48 }, // 44
    {KEY_EN_8,               49 },
    {KEY_EN_I,               50 },
    {KEY_EN_K,               51 },
    {KEY_EN_COMMA,           52 },
    {KEY_EN_RIGHT_ALT,       53 },

    {KEY_EN_F9,              54 }, // 50
    {KEY_EN_9,               55 },
    {KEY_EN_O,               56 },
    {KEY_EN_L,               57 },
    {KEY_EN_PERIOD,          58 },
    {KEY_EN_RIGHT_FUNCTION,  59 },

    {KEY_EN_F10,             60 }, // 56
    {KEY_EN_0,               61 },
    {KEY_EN_P,               62 },
    {KEY_EN_SEMICOLON,       63 },
    {KEY_EN_FORWARD_SLASH,   64 },
    {KEY_EN_RIGHT_CONTROL,   65 },

    {KEY_EN_F11,             66 }, // 62
    {KEY_EN_MINUS,           67 },
    {KEY_EN_LEFT_BRACKET,    68 },
    {KEY_EN_QUOTE,           69 },
    {KEY_EN_RIGHT_SHIFT,     82 },

    {KEY_EN_F12,             72 }, // 67
    {KEY_EN_EQUALS,          73 },
    {KEY_EN_RIGHT_BRACKET,   74 },
    {KEY_EN_LEFT_ARROW,      83 },

    {KEY_EN_DELETE,          78 }, // 71
    {KEY_EN_BACKSPACE,       79 },
    {KEY_EN_ANSI_BACK_SLASH, 80 },
    {KEY_EN_ANSI_ENTER,      81 },
    {KEY_EN_UP_ARROW,        88 },
    {KEY_EN_DOWN_ARROW,      89 },

    {KEY_EN_HOME,            90 }, // 77
    {KEY_EN_NUMPAD_LOCK,     91 },
    {KEY_EN_NUMPAD_7,        92 },
    {KEY_EN_NUMPAD_4,        93 },
    {KEY_EN_NUMPAD_1,        94 },
    {KEY_EN_RIGHT_ARROW,     95 },

    {KEY_EN_END,             96 }, // 83
    {KEY_EN_NUMPAD_DIVIDE,   97 },
    {KEY_EN_NUMPAD_8,        98 },
    {KEY_EN_NUMPAD_5,        99 },
    {KEY_EN_NUMPAD_2,        100},
    {KEY_EN_NUMPAD_0,        101},

    {KEY_EN_PAGE_UP,         102}, // 89
    {KEY_EN_NUMPAD_TIMES,    103},
    {KEY_EN_NUMPAD_9,        104},
    {KEY_EN_NUMPAD_6,        105},
    {KEY_EN_NUMPAD_3,        106},
    {KEY_EN_NUMPAD_PERIOD,   107},

    {KEY_EN_PAGE_DOWN,       108}, // 95
    {KEY_EN_NUMPAD_MINUS,    109},
    {KEY_EN_NUMPAD_PLUS,     110},
    {KEY_EN_NUMPAD_ENTER,    112},
};

RGBController_GenericWirelessKeyboard::RGBController_GenericWirelessKeyboard(
    GenericWirelessKeyboardController *controller)
    : controller(controller)
{
    name        = controller->GetName();
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Generic Wireless Keyboard";
    location    = controller->GetLocation();

    mode Direct;
    Direct.name       = "Direct";
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    active_mode = 0;

    SetupZones();

    /*------------------------------------------------------*\
    | The Generic Wireless Keyboard requires a steady stream |
    | of packets in order to not revert out of direct mode.  |
    | Start a thread to continuously refresh the device      |
    \*------------------------------------------------------*/
    keepalive_thread_run = true;
    keepalive_thread     = new std::thread(
        &RGBController_GenericWirelessKeyboard::KeepaliveThreadFunction, this);
}

RGBController_GenericWirelessKeyboard::
    ~RGBController_GenericWirelessKeyboard()
{
    keepalive_thread_run = false;
    keepalive_thread->join();
    delete keepalive_thread;
}

void RGBController_GenericWirelessKeyboard::SetupZones()
{

    // TODO: dynamically get layout
    zone Keyboard;
    Keyboard.name               = "Keyboard";
    Keyboard.type               = ZONE_TYPE_MATRIX;
    Keyboard.leds_min           = 99;
    Keyboard.leds_max           = 99;
    Keyboard.leds_count         = 99;
    Keyboard.matrix_map         = new matrix_map_type;
    Keyboard.matrix_map->height = HEIGHT;
    Keyboard.matrix_map->width  = WIDTH;
    Keyboard.matrix_map->map    = (unsigned int *)&matrix_map;
    zones.push_back(Keyboard);

    for(size_t i = 0; i < 99; i++)
    {
        led new_led;
        new_led.name  = led_names[i].name;
        new_led.value = led_names[i].idx;
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_GenericWirelessKeyboard::UpdateLEDs()
{
    std::unordered_map<RGBColor, std::vector<unsigned char>> new_color_map;

    for(int i = 0; i < colors.size(); ++i)
    {
        RGBColor color = QuantizeColor(colors[i]);
        new_color_map[color].push_back(leds[i].value);
    }

    if(new_color_map == led_color_map)
    {
        return;
    }

    led_color_map = std::move(new_color_map);
}

void RGBController_GenericWirelessKeyboard::SetCustomMode()
{
}

void RGBController_GenericWirelessKeyboard::ResizeZone(int /*zone*/,
                                                       int /*new_size*/)
{
}

void RGBController_GenericWirelessKeyboard::DeviceUpdateLEDs()
{
    UpdateLEDs();
}

void RGBController_GenericWirelessKeyboard::UpdateZoneLEDs(int /*zone*/)
{
    UpdateLEDs();
}

void RGBController_GenericWirelessKeyboard::UpdateSingleLED(int /*led*/)
{
    UpdateLEDs();
}

void RGBController_GenericWirelessKeyboard::DeviceUpdateMode()
{
}

void RGBController_GenericWirelessKeyboard::KeepaliveThreadFunction()
{
    while(keepalive_thread_run)
    {
        if(active_mode == 0)
        {
            if((std::chrono::steady_clock::now() - last_update_time) >
               20ms)
            {
                controller->SetLEDColors(led_color_map);
                last_update_time = std::chrono::steady_clock::now();
            }
        }
        std::this_thread::sleep_for(5ms);
    }
}

/*---------------------------------------------------------*\
| Quantize a 24-bit color down to a 15-bit color palette    |
| to group similar colors and reduce packet size.           |
\*---------------------------------------------------------*/
RGBColor RGBController_GenericWirelessKeyboard::QuantizeColor(RGBColor color)
{
    unsigned char r = RGBGetRValue(color);
    unsigned char g = RGBGetGValue(color);
    unsigned char b = RGBGetBValue(color);

    const int bits_to_discard = 5;

    unsigned char mask = 0xFF << bits_to_discard;

    unsigned char r_quantized = r & mask;
    unsigned char g_quantized = g & mask;
    unsigned char b_quantized = b & mask;

    return ToRGBColor(r_quantized, g_quantized, b_quantized);
}
