/*---------------------------------------------------------*\
| AcerClevoKeyboardController_Linux.cpp                     |
|                                                           |
|   Hardware driver for Acer Clevo keyboard backlight       |
|   Communicates via Linux sysfs (TUXEDO drivers)           |
|                                                           |
|   Shivam Purohit                               08 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "AcerClevoKeyboardController_Linux.h"

#include <string>
#include <vector>
#include <sstream>
#include <climits>
#include <glob.h>
#include <unistd.h>

AcerClevoKeyboardController::AcerClevoKeyboardController(const std::string& path, const std::string& name)
{
    sysfs_path  = path;
    device_name = name;
    zone_count  = 1;
    brightness  = 255;
    color_r     = 255;
    color_g     = 255;
    color_b     = 255;

    zone_count = DetectZoneCount();

    std::string brightness_str = ReadSysfsFile("brightness");
    if(!brightness_str.empty())
    {
        brightness = static_cast<unsigned char>(std::stoi(brightness_str));
    }

    std::string color_str = ReadSysfsFile("multi_intensity");
    if(!color_str.empty())
    {
        std::istringstream iss(color_str);
        int r, g, b;
        if(iss >> r >> g >> b)
        {
            color_r = static_cast<unsigned char>(r);
            color_g = static_cast<unsigned char>(g);
            color_b = static_cast<unsigned char>(b);
        }
    }
}

AcerClevoKeyboardController::~AcerClevoKeyboardController()
{
}

std::string AcerClevoKeyboardController::GetDeviceName()
{
    return device_name;
}

std::string AcerClevoKeyboardController::GetLocation()
{
    return sysfs_path;
}

unsigned int AcerClevoKeyboardController::GetZoneCount()
{
    return zone_count;
}

void AcerClevoKeyboardController::SetBrightness(unsigned char new_brightness)
{
    std::lock_guard<std::mutex> lock(mutex);
    brightness = new_brightness;
    WriteSysfsFile("brightness", std::to_string(brightness));
}

unsigned char AcerClevoKeyboardController::GetBrightness()
{
    std::lock_guard<std::mutex> lock(mutex);
    return brightness;
}

void AcerClevoKeyboardController::SetColor(unsigned char r, unsigned char g, unsigned char b)
{
    std::lock_guard<std::mutex> lock(mutex);
    color_r = r;
    color_g = g;
    color_b = b;
    std::string intensity = std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b);
    WriteSysfsFile("multi_intensity", intensity);
}

bool AcerClevoKeyboardController::WriteSysfsFile(const std::string& filename, const std::string& value)
{
    std::string filepath = sysfs_path + "/" + filename;
    FILE* fp = fopen(filepath.c_str(), "w");
    if(!fp)
    {
        return false;
    }
    fputs(value.c_str(), fp);
    fclose(fp);
    return true;
}

std::string AcerClevoKeyboardController::ReadSysfsFile(const std::string& filename)
{
    std::string filepath = sysfs_path + "/" + filename;
    FILE* fp = fopen(filepath.c_str(), "r");
    if(!fp)
    {
        return "";
    }
    char buf[256];
    if(fgets(buf, sizeof(buf), fp))
    {
        fclose(fp);
        std::string content(buf);
        while(!content.empty() && (content.back() == '\n' || content.back() == '\r'))
        {
            content.pop_back();
        }
        return content;
    }
    fclose(fp);
    return "";
}

unsigned int AcerClevoKeyboardController::DetectZoneCount()
{
    glob_t glob_result;
    std::string pattern = "/sys/class/leds/rgb:kbd_backlight*";

    if(glob(pattern.c_str(), GLOB_NOSORT, nullptr, &glob_result) != 0)
    {
        return 1;
    }

    std::vector<std::string> unique_paths;
    for(size_t i = 0; i < glob_result.gl_pathc; i++)
    {
        char target[PATH_MAX];
        ssize_t len = readlink(glob_result.gl_pathv[i], target, sizeof(target) - 1);
        if(len != -1)
        {
            target[len] = '\0';
            std::string target_str(target);
            bool found = false;
            for(size_t j = 0; j < unique_paths.size(); j++)
            {
                if(unique_paths[j] == target_str)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                unique_paths.push_back(target_str);
            }
        }
    }

    globfree(&glob_result);

    unsigned int count = static_cast<unsigned int>(unique_paths.size());
    return (count > 0) ? count : 1;
}
