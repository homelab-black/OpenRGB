#pragma once

#include <cstdint>
#include <functional>
#include "hidapi_wrapper.h"
#include "SPDWrapper.h"
#include "RGBController.h"
#include "i2c_smbus.h"

/*---------------------------------------------------------*\
| HID Default Values                                        |
\*---------------------------------------------------------*/
#define HID_PID_ANY                     -1
#define HID_VID_ANY                     -1
#define HID_INTERFACE_ANY               -1
#define HID_USAGE_ANY                   -1
#define HID_USAGE_PAGE_ANY              -1



/*---------------------------------------------------------*\
| Detector Function Types                                   |
\*---------------------------------------------------------*/
typedef std::vector<RGBController*> DetectedControllers;

typedef std::function<bool()>                                                                                       I2CBusDetectorFunction;
typedef std::function<DetectedControllers()>                                                                        DeviceDetectorFunction;
typedef std::function<DetectedControllers(std::vector<i2c_smbus_interface*>&)>                                      I2CDeviceDetectorFunction;
typedef std::function<DetectedControllers(i2c_smbus_interface*, std::vector<SPDWrapper*>&, const std::string&)>     I2CDRAMDeviceDetectorFunction;
typedef std::function<DetectedControllers(i2c_smbus_interface*, uint8_t, const std::string&)>                       I2CPCIDeviceDetectorFunction;
typedef std::function<DetectedControllers(hid_device_info*, const std::string&)>                                    HIDDeviceDetectorFunction;
typedef std::function<DetectedControllers(hidapi_wrapper wrapper, hid_device_info*, const std::string&)>            HIDWrappedDeviceDetectorFunction;
typedef std::function<void()>                                                                                       DynamicDetectorFunction;
typedef std::function<void()>                                                                                       PreDetectionHookFunction;

/*---------------------------------------------------------*\
| Detector Block Types                                      |
\*---------------------------------------------------------*/

/*---------------------------------------------------*\
| Dynamic detectors register new detectors at runtime |
\*---------------------------------------------------*/
struct DynamicDetectorBlock
{
    std::string name;
    DynamicDetectorFunction detector;
};

struct DeviceDetectorBlock
{
    std::string name;
    DeviceDetectorFunction detector;
};

class BasicHIDBlock
{
public:
    std::string                         name;
    int                                 vid;
    int                                 pid;
    int                                 interface;
    int                                 usage_page;
    int                                 usage;

    virtual ~BasicHIDBlock() = default;
    bool compare(hid_device_info* info) const;
    bool matching_id(hid_device_info* info) const;
    virtual DetectedControllers run(hid_device_info* device) = 0;
};

class HIDDeviceDetectorBlock : public BasicHIDBlock
{
public:
    HIDDeviceDetectorFunction           function;
    ~HIDDeviceDetectorBlock();

    DetectedControllers run(hid_device_info* device) override;
};

class HIDWrappedDeviceDetectorBlock : public BasicHIDBlock
{
public:
    HIDWrappedDeviceDetectorFunction    function;
    hidapi_wrapper*                     hid_wrapper;

    ~HIDWrappedDeviceDetectorBlock();

    DetectedControllers run(hid_device_info* device) override;
};

struct I2CDeviceDetectorBlock
{
    std::string name;
    I2CDeviceDetectorFunction function;
};

struct I2CPCIDeviceDetectorBlock
{
    std::string                         name;
    I2CPCIDeviceDetectorFunction        function;
    uint16_t                            ven_id;
    uint16_t                            dev_id;
    uint16_t                            subven_id;
    uint16_t                            subdev_id;
    uint8_t                             i2c_addr;
};


struct I2CDRAMDeviceDetectorBlock
{
    std::string                         name;
    I2CDRAMDeviceDetectorFunction       function;
    uint16_t                            jedec_id;
    uint8_t                             dram_type;
};

/*---------------------------------------------------------*\
| Detector Types                                            |
\*---------------------------------------------------------*/
class DeviceDetector
{
public:
    DeviceDetector(std::string name, DeviceDetectorFunction detector);
};

class I2CDeviceDetector
{
public:
    I2CDeviceDetector(std::string name, I2CDeviceDetectorFunction detector);
};

class I2CDRAMDeviceDetector
{
public:
    I2CDRAMDeviceDetector(std::string name, I2CDRAMDeviceDetectorFunction detector, uint16_t jedec_id, uint8_t dram_type);
};

class I2CPCIDeviceDetector
{
public:
    I2CPCIDeviceDetector(std::string name, I2CPCIDeviceDetectorFunction detector, uint16_t ven_id, uint16_t dev_id, uint16_t subven_id, uint16_t subdev_id, uint8_t i2c_addr);
};

class I2CBusDetector
{
public:
    I2CBusDetector(I2CBusDetectorFunction detector);
};

class HIDDeviceDetector
{
public:
    HIDDeviceDetector(std::string name, HIDDeviceDetectorFunction detector, int vid, int pid, int interface, int usage_page, int usage);
};

class HIDWrappedDeviceDetector
{
public:
    HIDWrappedDeviceDetector(std::string name, HIDWrappedDeviceDetectorFunction detector, int vid, int pid, int interface, int usage_page, int usage);
};

class DynamicDetector
{
public:
    DynamicDetector(std::string name, DynamicDetectorFunction detector);
};

class PreDetectionHook
{
public:
    PreDetectionHook(PreDetectionHookFunction hook);
};
