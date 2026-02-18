#include "detector_types.h"
#include "DetectionManager.h"

/*---------------------------------------------------------*\
| BasicHIDBlock Implementation                              |
\*---------------------------------------------------------*/
bool BasicHIDBlock::compare(hid_device_info* info) const
{
    return(((vid        == HID_VID_ANY)
         || (vid == info->vendor_id))
        && ((pid        == HID_PID_ANY)
         || (pid == info->product_id))
        && ((usage_page == HID_USAGE_PAGE_ANY)
         || (usage_page == info->usage_page))
        && ((usage      == HID_USAGE_ANY)
         || (usage      == info->usage))
        && ((interface  == HID_INTERFACE_ANY)
         || (interface  == info->interface_number))
        );
}

bool BasicHIDBlock::matching_id(hid_device_info* info) const
{
    return((vid == info->vendor_id) && (pid == info->product_id));
}

HIDDeviceDetectorBlock::~HIDDeviceDetectorBlock()
{
}

DetectedControllers HIDDeviceDetectorBlock::run(hid_device_info* device)
{
    return function(device, name);
}

HIDWrappedDeviceDetectorBlock::~HIDWrappedDeviceDetectorBlock()
{
}

DetectedControllers HIDWrappedDeviceDetectorBlock::run(hid_device_info* device)
{
    return function(*hid_wrapper, device, name);
}

DeviceDetector::DeviceDetector(std::string name, DeviceDetectorFunction detector)
{
    DetectionManager::get()->RegisterDeviceDetector(name, detector);
}

I2CDeviceDetector::I2CDeviceDetector(std::string name, I2CDeviceDetectorFunction detector)
{
    DetectionManager::get()->RegisterI2CDeviceDetector(name, detector);
}

I2CDRAMDeviceDetector::I2CDRAMDeviceDetector(std::string name, I2CDRAMDeviceDetectorFunction detector, uint16_t jedec_id, uint8_t dram_type)
{
    DetectionManager::get()->RegisterI2CDRAMDeviceDetector(name, detector, jedec_id, dram_type);
}

I2CPCIDeviceDetector::I2CPCIDeviceDetector(std::string name, I2CPCIDeviceDetectorFunction detector, uint16_t ven_id, uint16_t dev_id, uint16_t subven_id, uint16_t subdev_id, uint8_t i2c_addr)
{
    DetectionManager::get()->RegisterI2CPCIDeviceDetector(name, detector, ven_id, dev_id, subven_id, subdev_id, i2c_addr);
}

I2CBusDetector::I2CBusDetector(I2CBusDetectorFunction detector)
{
    DetectionManager::get()->RegisterI2CBusDetector(detector);
}

HIDDeviceDetector::HIDDeviceDetector(std::string name, HIDDeviceDetectorFunction detector, int vid, int pid, int interface, int usage_page, int usage)
{
    DetectionManager::get()->RegisterHIDDeviceDetector(name, detector, vid, pid, interface, usage_page, usage);
}

HIDWrappedDeviceDetector::HIDWrappedDeviceDetector(std::string name, HIDWrappedDeviceDetectorFunction detector, int vid, int pid, int interface, int usage_page, int usage)
{
    DetectionManager::get()->RegisterHIDWrappedDeviceDetector(name, detector, vid, pid, interface, usage_page, usage);
}

DynamicDetector::DynamicDetector(std::string name, DynamicDetectorFunction detector)
{
    DetectionManager::get()->RegisterDynamicDetector(name, detector);
}

PreDetectionHook::PreDetectionHook(PreDetectionHookFunction hook)
{
    DetectionManager::get()->RegisterPreDetectionHook(hook);
}
