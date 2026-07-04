#include "DarkProjectKD87AController.h"
#include "StringUtils.h"
#include <cstring>
#include <cstdio>   // для printf

DarkProjectKD87AController::DarkProjectKD87AController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev = dev_handle;
    location = path;
    name = dev_name;
}

DarkProjectKD87AController::~DarkProjectKD87AController()
{
    hid_close(dev);
}

std::string DarkProjectKD87AController::GetLocation()
{
    return("HID: " + location);
}

std::string DarkProjectKD87AController::GetName()
{
    return(name);
}

std::string DarkProjectKD87AController::GetSerial()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if (ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void DarkProjectKD87AController::SetLedsDirect(std::vector<RGBColor> colors)
{
    // ========== ОТЛАДОЧНЫЙ ВЫВОД (убедимся, что функция вызывается) ==========
    printf("[DarkProject] SetLedsDirect called with %zu colors\n", colors.size());
    fflush(stdout);

    if (!dev)
    {
        printf("[DarkProject] ERROR: device handle is NULL\n");
        return;
    }

    if (colors.empty())
    {
        printf("[DarkProject] ERROR: colors array is empty\n");
        return;
    }

    // ========== ФОРМИРУЕМ ПАКЕТ ==========
    // Берём цвет первого LED (для всей клавиатуры сразу)
    RGBColor color = colors[0];
    unsigned char r = RGBGetRValue(color);
    unsigned char g = RGBGetGValue(color);
    unsigned char b = RGBGetBValue(color);

    printf("[DarkProject] Sending color: R=%d G=%d B=%d\n", r, g, b);

    // Шаблон пакета (чёрный цвет, все огни выключены — из твоего дампа)
    uint8_t packet[556] = {
        0x1c, 0x00, 0x20, 0x95, 0x68, 0x27, 0x85, 0xd5, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x00,
        0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x02, 0x10, 0x02, 0x00, 0x00, 0x00, 0x21, 0x09, 0x00, 0x03,
        0x03, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x05, 0x01, 0xd4, 0xff, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    // Остальные байты (с 64 по 555) — нули, что уже есть, так как массив инициализирован

    // Устанавливаем цвет в нужные смещения (по данным Wireshark)
    packet[0x2F] = r;   // R (последний байт строки 0020)
    packet[0x30] = g;   // G (первый байт строки 0030)
    packet[0x31] = b;   // B (второй байт строки 0030)

    // ========== ОТПРАВЛЯЕМ ==========
    int ret = hid_write(dev, packet, 556);
    if (ret < 0)
    {
        printf("[DarkProject] hid_write failed: %d\n", ret);
    }
    else
    {
        printf("[DarkProject] hid_write success, sent %d bytes\n", ret);
    }
    fflush(stdout);
}
