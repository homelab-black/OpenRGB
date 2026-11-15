/* SPDX-License-Identifier: GPL-2.0-or-later */
// Fake HID transport: this executable never opens real hardware.
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>
#include "AkkoMU02Controller.h"
#ifdef AKKO_MU02_TEST_ADAPTER
#include "RGBController_AkkoMU02.h"

class TestRGBController_AkkoMU02 : public RGBController_AkkoMU02
{
public:
    using RGBController_AkkoMU02::RGBController_AkkoMU02;
    void SelectMode(unsigned int mode) { active_mode = mode; }
    void UpdateMode() override {} // Call device callbacks synchronously, without worker-startup races.
    unsigned int KeyValue(unsigned int index) const { return leds[index].value; }
};
#endif

struct hid_device_
{
    unsigned int protocol = 12;
    std::array<unsigned char, 5> values = {255, 4, 153, 192, 255};
    std::array<std::array<unsigned char, 2>, 90> key_colors = []
    {
        std::array<std::array<unsigned char, 2>, 90> colors;
        for(unsigned int i = 0; i < colors.size(); i++)
        {
            colors[i] = {static_cast<unsigned char>(i * 7), static_cast<unsigned char>(100 + i)};
        }
        return colors;
    }(); // Hue, saturation; deliberately not RGB-round-trip friendly.
    std::vector<std::array<unsigned char, 33>> writes;
    bool short_write = false;
    int read_result = 32;
    int corrupt_byte = -1;
    int corrupt_key_get_byte = -1;
    bool ignore_set = false;
    bool allow_brightness = false;
    bool allow_save = false;
    int closes = 0;
};

extern "C" int hid_write(hid_device* dev, const unsigned char* data, size_t length)
{
    assert(length == 33 && data[0] == 0);
    const auto command = data[1];
    assert(command == 1 || command == 7 || command == 8);
    unsigned int used = 2;
    const bool save = command == 7 && data[2] == 0 && data[3] == 2;
    const bool per_key = command != 1 && data[2] == 0 && !save;
    if(save)
    {
        assert(dev->allow_save && data[4] == 0); // Only an explicitly requested palette commit.
        used = 5;
    }
    else if(per_key)
    {
        assert(data[3] == 3 && data[4] == 0 && data[5] < 6 && data[6] < 15);
        used = command == 7 ? 9 : 7;
        if(command == 7 && !dev->ignore_set && !dev->short_write)
        {
            dev->key_colors[data[5] * 15 + data[6]] = {data[8], data[7]};
        }
    }
    else if(command != 1)
    {
        assert(data[2] == 3 && data[3] >= 1 && data[3] <= 4);
        used = 4;
    }
    if(command == 7 && !per_key && !save)
    {
        assert(data[3] != 1 || dev->allow_brightness); // Brightness requires an explicit test request.
        used += data[3] == 4 ? 2 : 1;
        if(!dev->ignore_set && !dev->short_write)
        {
            dev->values[data[3] - 1] = data[3] == 1 && data[4] == 255 ? 253 : data[4];
            if(data[3] == 4)
            {
                dev->values[4] = data[5];
            }
        }
    }
    for(size_t i = used; i < length; i++)
    {
        assert(data[i] == 0);
    }
    std::array<unsigned char, 33> packet;
    std::memcpy(packet.data(), data, length);
    dev->writes.push_back(packet);
    return dev->short_write ? 32 : 33;
}

extern "C" int hid_read_timeout(hid_device* dev, unsigned char* data, size_t length, int timeout)
{
    assert(length == 32 && timeout == 1000);
    if(dev->read_result != 32)
    {
        return dev->read_result;
    }
    std::memcpy(data, dev->writes.back().data() + 1, length);
    if(data[0] == 1)
    {
        data[1] = dev->protocol >> 8;
        data[2] = dev->protocol & 255;
    }
    else if(data[0] == 8)
    {
        if(data[1] == 0)
        {
            const auto& color = dev->key_colors[data[4] * 15 + data[5]];
            data[6] = color[1];
            data[7] = color[0];
            if(dev->corrupt_key_get_byte >= 0)
            {
                data[dev->corrupt_key_get_byte] ^= 0x80;
            }
        }
        else
        {
            data[3] = dev->values[data[2] - 1];
            if(data[2] == 4)
            {
                data[4] = dev->values[4];
            }
        }
    }
    if(dev->corrupt_byte >= 0)
    {
        data[dev->corrupt_byte] ^= 0x80;
    }
    return 32;
}

extern "C" void hid_close(hid_device* dev)
{
    dev->closes++;
}

static unsigned int sets(const hid_device& dev)
{
    unsigned int count = 0;
    for(const auto& packet : dev.writes)
    {
        count += packet[1] == 7;
    }
    return count;
}

static unsigned int key_sets(const hid_device& dev)
{
    unsigned int count = 0;
    for(const auto& packet : dev.writes)
    {
        count += packet[1] == 7 && packet[2] == 0 && packet[3] == 3;
    }
    return count;
}

static unsigned int saves(const hid_device& dev)
{
    unsigned int count = 0;
    for(const auto& packet : dev.writes)
    {
        count += packet[1] == 7 && packet[2] == 0 && packet[3] == 2;
    }
    return count;
}

static void check_key_cache(const AkkoMU02Controller& controller, const hid_device& dev)
{
    const AkkoMU02KeyColor* colors = controller.GetKeyColors();
    for(unsigned int i = 0; i < 90; i++)
    {
        assert(colors[i].hue == dev.key_colors[i][0]);
        assert(colors[i].saturation == dev.key_colors[i][1]);
    }
}

int main()
{
    hid_device dev;
    const auto original = dev.values;
    {
        AkkoMU02Controller controller(&dev, "/fake/mu02");
        assert(controller.GetSupported() && dev.writes.size() == 95 && sets(dev) == 0);
        assert(dev.writes[0][1] == 1);
        for(unsigned int i = 1; i < 5; i++)
        {
            assert(dev.writes[i][1] == 8 && dev.writes[i][2] == 3 && dev.writes[i][3] == i);
        }
        for(unsigned int i = 0; i < 90; i++)
        {
            const auto& packet = dev.writes[5 + i];
            assert(packet[1] == 8 && packet[2] == 0 && packet[3] == 3 && packet[4] == 0);
            assert(packet[5] == i / 15 && packet[6] == i % 15);
        }
        check_key_cache(controller, dev);
        const auto& state = controller.GetState();
        assert(state.brightness == 255 && state.effect == 4 && state.speed == 153);
        assert(state.hue == 192 && state.saturation == 255);
        assert(controller.Apply(4, nullptr, nullptr) && sets(dev) == 0);
        assert(!controller.Apply(46, nullptr, nullptr) && sets(dev) == 0);

        const unsigned char color[] = {0, 255};
        const unsigned char speed = 64;
        assert(controller.Apply(1, &speed, color));
        assert((dev.values == std::array<unsigned char, 5>{255, 1, 64, 0, 255}));
        assert(sets(dev) == 3);
        assert(controller.Apply(1, &speed, color) && sets(dev) == 3);
        const unsigned char old_color[] = {192, 255};
        const unsigned char old_speed = 153;
        assert(controller.Apply(4, &old_speed, old_color));
        assert(dev.values == original && sets(dev) == 6);

        // Physical Fn-key changes must not be overwritten by an unrelated mode update.
        dev.values[3] = 100;
        assert(controller.Apply(1, nullptr, nullptr));
        assert(dev.values[3] == 100 && dev.values[0] == 255);

        dev.ignore_set = true;
        assert(!controller.Apply(4, nullptr, nullptr)); // Echo alone is not successful readback.
        dev.ignore_set = false;
        dev.corrupt_byte = 2;
        const auto before_failure = sets(dev);
        assert(!controller.Apply(4, nullptr, nullptr));
        assert(sets(dev) == before_failure);
    }
    assert(dev.closes == 1);

    for(const auto protocol : {13u, 0x0C00u})
    {
        hid_device bad;
        bad.protocol = protocol;
        AkkoMU02Controller controller(&bad, "/fake/mu02");
        assert(!controller.GetSupported() && sets(bad) == 0);
        const auto before = bad.writes.size();
        assert(!controller.SetKeyColor(0, 0, 12, 34) && bad.writes.size() == before);
    }
    for(const auto read_result : {-1, 0, 31})
    {
        hid_device bad;
        bad.read_result = read_result;
        AkkoMU02Controller controller(&bad, "/fake/mu02");
        assert(!controller.GetSupported() && sets(bad) == 0);
    }
    for(int byte = 0; byte <= 2; byte++)
    {
        hid_device bad;
        AkkoMU02Controller controller(&bad, "/fake/mu02");
        bad.corrupt_byte = byte;
        assert(!controller.Apply(1, nullptr, nullptr) && sets(bad) == 0);
    }
    {
        hid_device bad;
        bad.short_write = true;
        AkkoMU02Controller controller(&bad, "/fake/mu02");
        assert(!controller.GetSupported() && sets(bad) == 0);
    }
    {
        hid_device bad;
        bad.values[1] = 250;
        AkkoMU02Controller controller(&bad, "/fake/mu02");
        assert(!controller.GetSupported() && sets(bad) == 0);
    }
    {
        hid_device keys_dev;
        AkkoMU02Controller controller(&keys_dev, "/fake/mu02");
        const auto globals = keys_dev.values;
        const auto original_keys = keys_dev.key_colors;
        const auto before = keys_dev.writes.size();
        for(const auto row : {6, 255})
        {
            assert(!controller.SetKeyColor(row, 0, 23, 45));
        }
        for(const auto col : {15, 255})
        {
            assert(!controller.SetKeyColor(0, col, 23, 45));
        }
        assert(keys_dev.writes.size() == before);
        assert(controller.SetKeyColor(5, 14, 23, 45));
        assert(keys_dev.writes.size() == before + 2 && key_sets(keys_dev) == 1);
        const std::array<unsigned char, 33> expected_set = {0, 7, 0, 3, 0, 5, 14, 45, 23};
        const std::array<unsigned char, 33> expected_get = {0, 8, 0, 3, 0, 5, 14};
        assert(keys_dev.writes[before] == expected_set && keys_dev.writes[before + 1] == expected_get);
        for(unsigned int i = 0; i < 89; i++)
        {
            assert(keys_dev.key_colors[i] == original_keys[i]);
        }
        check_key_cache(controller, keys_dev);
        for(const auto& color : std::array<std::array<unsigned char, 2>, 2>{{{0, 0}, {1, 0}}})
        {
            assert(controller.SetKeyColor(0, 0, color[0], color[1]));
            check_key_cache(controller, keys_dev);
        }
        assert(keys_dev.values == globals);
    }
    // Readback must match both components, not merely echo the SET or accept a valid header.
    for(const auto corrupt : {-1, 0, 1, 2, 3, 4, 5, 6, 7})
    {
        hid_device keys_dev;
        AkkoMU02Controller controller(&keys_dev, "/fake/mu02");
        const auto old = controller.GetKeyColors()[0];
        keys_dev.ignore_set = corrupt == -1;
        keys_dev.corrupt_key_get_byte = corrupt;
        assert(!controller.SetKeyColor(0, 0, 23, 45));
        assert(controller.GetKeyColors()[0].hue == old.hue
            && controller.GetKeyColors()[0].saturation == old.saturation);
    }
    for(const bool short_write : {false, true})
    {
        hid_device keys_dev;
        AkkoMU02Controller controller(&keys_dev, "/fake/mu02");
        const auto old = controller.GetKeyColors()[0];
        keys_dev.short_write = short_write;
        keys_dev.read_result = -1;
        assert(!controller.SetKeyColor(0, 0, 23, 45));
        assert(controller.GetKeyColors()[0].hue == old.hue
            && controller.GetKeyColors()[0].saturation == old.saturation);
    }
    {
        hid_device keys_dev;
        keys_dev.corrupt_key_get_byte = 4;
        AkkoMU02Controller controller(&keys_dev, "/fake/mu02");
        assert(!controller.GetSupported() && sets(keys_dev) == 0);
    }
    {
        hid_device brightness_dev;
        brightness_dev.allow_brightness = true;
        AkkoMU02Controller controller(&brightness_dev, "/fake/mu02");
        unsigned char brightness = 0;
        assert(controller.Apply(4, nullptr, nullptr, &brightness));
        assert(brightness_dev.values[0] == 0 && sets(brightness_dev) == 1);
        brightness = 255;
        assert(controller.Apply(4, nullptr, nullptr, &brightness));
        assert(controller.GetState().brightness == 253 && sets(brightness_dev) == 2);
        assert(controller.Apply(4, nullptr, nullptr) && sets(brightness_dev) == 2);
        assert(brightness_dev.values[1] == 4 && brightness_dev.values[2] == 153
            && brightness_dev.values[3] == 192 && brightness_dev.values[4] == 255);
    }
#ifdef AKKO_MU02_TEST_ADAPTER
    {
        hid_device adapter_dev;
        TestRGBController_AkkoMU02 rgb(new AkkoMU02Controller(&adapter_dev, "/fake/mu02"));
        assert(rgb.GetActiveMode() == 4 && rgb.GetModeCount() == 46);
        for(unsigned int index = 0; index < rgb.GetModeCount(); index++)
        {
            assert(!(rgb.GetModeFlags(index) & MODE_FLAG_AUTOMATIC_SAVE));
            assert(bool(rgb.GetModeFlags(index) & MODE_FLAG_MANUAL_SAVE) == (index == 45));
            assert(bool(rgb.GetModeFlags(index) & MODE_FLAG_HAS_PER_LED_COLOR) == (index == 45));
            assert(bool(rgb.GetModeFlags(index) & MODE_FLAG_HAS_BRIGHTNESS) == (index != 0));
            assert(bool(rgb.GetModeFlags(index) & MODE_FLAG_HAS_SPEED) == (index != 0 && index != 45));
        }
        rgb.DeviceUpdateMode();
        assert(sets(adapter_dev) == 0); // No RGB/HSV round-trip writes on an untouched mode.
        adapter_dev.values[2] = 64; // Physical adjustments after discovery.
        adapter_dev.values[0] = 200;
        rgb.SetModeColor(4, 0, ToRGBColor(255, 0, 0));
        rgb.DeviceUpdateMode();
        assert(adapter_dev.values[2] == 64 && adapter_dev.values[3] == 0 && sets(adapter_dev) == 1);
        assert(rgb.GetModeSpeed(4) == 64 && rgb.GetModeBrightness(4) == 200);
        rgb.DeviceUpdateMode();
        assert(sets(adapter_dev) == 1);
        rgb.SetModeSpeed(4, 32);
        rgb.DeviceUpdateMode();
        assert(adapter_dev.values[2] == 32 && sets(adapter_dev) == 2);
        rgb.SetModeColor(4, 0, ToRGBColor(0, 0, 0));
        rgb.DeviceUpdateMode();
        assert(sets(adapter_dev) == 2 && adapter_dev.values[0] == 200); // Black cannot become white.
        adapter_dev.allow_brightness = true; // A rejected black picker must not block brightness.
        rgb.SetModeBrightness(4, 0);
        rgb.DeviceUpdateMode();
        assert(sets(adapter_dev) == 3 && rgb.GetModeBrightness(4) == 0);
        rgb.SetModeBrightness(4, 255);
        rgb.DeviceUpdateMode();
        assert(sets(adapter_dev) == 4 && rgb.GetModeBrightness(4) == 253);
        rgb.DeviceUpdateMode();
        assert(sets(adapter_dev) == 4); // Do not repeatedly rescale the readback.
    }
    {
        hid_device adapter_dev;
        adapter_dev.key_colors[0] = {0, 0};
        adapter_dev.key_colors[1] = {1, 0};
        const auto original_keys = adapter_dev.key_colors;
        const auto globals = adapter_dev.values;
        TestRGBController_AkkoMU02 rgb(new AkkoMU02Controller(&adapter_dev, "/fake/mu02"));
        assert(rgb.GetLEDCount() == 83 && rgb.GetZoneCount() == 1);
        assert(rgb.GetZone(0).type == ZONE_TYPE_MATRIX);
        assert(rgb.GetZoneMatrixMapHeight(0) == 6 && rgb.GetZoneMatrixMapWidth(0) == 15);
        const unsigned int* matrix = rgb.GetZoneMatrixMapData(0);
        unsigned int count = 0;
        for(unsigned int slot = 0; slot < 90; slot++)
        {
            const bool hole = slot == 14 || slot == 43 || slot == 78 || slot == 79
                           || slot == 80 || slot == 82 || slot == 83;
            if(hole)
            {
                assert(matrix[slot] == 0xFFFFFFFFu);
            }
            else
            {
                assert(matrix[slot] == count && rgb.KeyValue(count) == slot);
                count++;
            }
        }
        const unsigned int esc = matrix[0];
        assert(rgb.GetLEDName(esc) == "Esc");
        assert(rgb.GetColor(esc) == ToRGBColor(0, 0, 0));
        assert(rgb.GetColor(matrix[1]) == ToRGBColor(255, 255, 255));
        assert(rgb.GetColor(matrix[2]) != ToRGBColor(0, 0, 0));
        assert(rgb.GetModeName(45) == "Custom");
        assert(rgb.GetModeFlags(45) == (MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_MANUAL_SAVE));
        assert(rgb.GetModeColorMode(45) == MODE_COLORS_PER_LED);
        assert(sets(adapter_dev) == 0 && adapter_dev.writes.size() == 95);
        rgb.DeviceUpdateLEDs();
        rgb.DeviceUpdateZoneLEDs(0);
        rgb.DeviceUpdateSingleLED(esc);
        rgb.DeviceUpdateMode();
        assert(sets(adapter_dev) == 0);

        // Every global effect ignores per-key requests, even when a color is dirty.
        for(unsigned int effect = 0; effect < 45; effect++)
        {
            rgb.SelectMode(effect);
            rgb.SetColor(esc, ToRGBColor(0, 255, 0));
            const auto before = adapter_dev.writes.size();
            rgb.DeviceUpdateLEDs();
            rgb.DeviceUpdateZoneLEDs(0);
            rgb.DeviceUpdateSingleLED(esc);
            assert(adapter_dev.writes.size() == before);
            rgb.SetColor(esc, ToRGBColor(0, 0, 0));
        }
        rgb.SelectMode(45);
        rgb.DeviceUpdateMode();
        rgb.DeviceUpdateLEDs();
        rgb.DeviceUpdateZoneLEDs(0);
        assert(key_sets(adapter_dev) == 0 && sets(adapter_dev) == 1);
        assert(adapter_dev.key_colors == original_keys);
        assert(adapter_dev.values[0] == globals[0] && adapter_dev.values[2] == globals[2]
            && adapter_dev.values[3] == globals[3] && adapter_dev.values[4] == globals[4]);
        auto expected_keys = original_keys;
        const auto before = adapter_dev.writes.size();
        rgb.SetColor(esc, ToRGBColor(0, 128, 0)); // V is not per-key brightness.
        rgb.DeviceUpdateSingleLED(esc);
        expected_keys[0] = {85, 255}; // 120 degrees scaled to the 256-step hue wheel.
        assert(adapter_dev.key_colors == expected_keys && key_sets(adapter_dev) == 1);
        assert(adapter_dev.writes.size() == before + 2);
        rgb.DeviceUpdateSingleLED(esc);
        rgb.DeviceUpdateLEDs();
        rgb.DeviceUpdateZoneLEDs(0);
        rgb.DeviceUpdateMode();
        assert(key_sets(adapter_dev) == 1); // Cache requested RGB, not lossy HSV readback.

        rgb.SetColor(esc, ToRGBColor(0, 0, 0));
        rgb.DeviceUpdateZoneLEDs(0);
        expected_keys[0] = {0, 0};
        assert(adapter_dev.key_colors == expected_keys && key_sets(adapter_dev) == 2);
        rgb.SetColor(esc, ToRGBColor(255, 255, 255));
        rgb.DeviceUpdateLEDs();
        expected_keys[0] = {1, 0}; // H0/S0 is reserved for off.
        assert(adapter_dev.key_colors == expected_keys && key_sets(adapter_dev) == 3);
        rgb.SetColor(esc, ToRGBColor(64, 64, 64));
        rgb.DeviceUpdateSingleLED(esc);
        assert(adapter_dev.key_colors == expected_keys && key_sets(adapter_dev) == 4);
        rgb.DeviceUpdateLEDs();
        rgb.DeviceUpdateZoneLEDs(0);
        assert(key_sets(adapter_dev) == 4);

        // A physically applied SET with a failed readback invalidates the RGB cache.
        rgb.SetColor(esc, ToRGBColor(0, 255, 0));
        adapter_dev.corrupt_key_get_byte = 6;
        rgb.DeviceUpdateSingleLED(esc);
        assert((adapter_dev.key_colors[0] == std::array<unsigned char, 2>{85, 255}));
        adapter_dev.corrupt_key_get_byte = -1;
        rgb.SetColor(esc, ToRGBColor(64, 64, 64)); // The previous cached RGB must still be sent.
        rgb.DeviceUpdateSingleLED(esc);
        assert(adapter_dev.key_colors == expected_keys && key_sets(adapter_dev) == 6);

        // Invalid callbacks must not send packets or flush a valid but dirty LED.
        rgb.SetColor(esc, ToRGBColor(255, 0, 0));
        const auto before_invalid = adapter_dev.writes.size();
        rgb.DeviceUpdateSingleLED(-1);
        rgb.DeviceUpdateSingleLED(rgb.GetLEDCount());
        rgb.DeviceUpdateZoneLEDs(-1);
        rgb.DeviceUpdateZoneLEDs(1);
        assert(adapter_dev.writes.size() == before_invalid);
        assert(adapter_dev.values[0] == globals[0] && adapter_dev.values[2] == globals[2]
            && adapter_dev.values[3] == globals[3] && adapter_dev.values[4] == globals[4]);
    }
    {
        hid_device adapter_dev;
        TestRGBController_AkkoMU02 rgb(new AkkoMU02Controller(&adapter_dev, "/fake/mu02"));
        adapter_dev.values[0] = 200;
        adapter_dev.values[2] = 64;
        rgb.DeviceUpdateMode(); // Read physical changes; keep untouched modes in sync.
        assert(sets(adapter_dev) == 0);
        rgb.SetCustomMode(); // Exercise the framework/SDK path, not just the explicit mode index.
        assert(rgb.GetActiveMode() == 45);
        rgb.DeviceUpdateMode();
        assert(adapter_dev.values[0] == 200 && adapter_dev.values[2] == 64 && sets(adapter_dev) == 1);
        rgb.SelectMode(1);
        rgb.DeviceUpdateMode();
        assert(adapter_dev.values[0] == 200 && adapter_dev.values[2] == 64 && sets(adapter_dev) == 2);
    }
    {
        hid_device adapter_dev;
        TestRGBController_AkkoMU02 rgb(new AkkoMU02Controller(&adapter_dev, "/fake/mu02"));
        rgb.SaveMode();
        assert(saves(adapter_dev) == 0); // No commit outside Custom.
        rgb.SelectMode(45);
        rgb.DeviceUpdateMode();
        rgb.SetColor(0, ToRGBColor(0, 0, 255));
        rgb.DeviceUpdateSingleLED(0);
        assert(saves(adapter_dev) == 0); // Colour changes and mode selection never auto-save.
        const auto globals = adapter_dev.values;
        const auto keys = adapter_dev.key_colors;
        adapter_dev.allow_save = true;
        rgb.SaveMode();
        assert(saves(adapter_dev) == 1 && adapter_dev.values == globals && adapter_dev.key_colors == keys);
        adapter_dev.corrupt_key_get_byte = 6;
        rgb.SetColor(1, ToRGBColor(0, 0, 0));
        rgb.SaveMode();
        assert(saves(adapter_dev) == 1); // Never commit a partly unverified palette.
        adapter_dev.corrupt_key_get_byte = -1;
        rgb.SaveMode();
        assert(saves(adapter_dev) == 2 && key_sets(adapter_dev) == 3);
    }
#endif
    {
        hid_device save_dev;
        AkkoMU02Controller controller(&save_dev, "/fake/mu02");
        assert(!controller.SaveKeyColors() && saves(save_dev) == 0);
        assert(controller.Apply(45, nullptr, nullptr));
        save_dev.allow_save = true;
        assert(controller.SaveKeyColors() && saves(save_dev) == 1);
        save_dev.corrupt_byte = 2;
        assert(!controller.SaveKeyColors() && saves(save_dev) == 2); // No automatic retry.
    }
    std::cout << "PASS: global/per-key framing, readback, brightness rounding and explicit-only palette save/apply\n";
}
