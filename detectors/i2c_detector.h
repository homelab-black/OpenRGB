#pragma once
#include "detector_group.h"
#include "detector_types.h"

class I2CDetectorGroup : public DetectorGroup
{
public:
    I2CDetectorGroup(DetectionManager* dm);
    ~I2CDetectorGroup();

    virtual void Prepare() override;
    virtual void RunDetection() override;
    virtual void CleanUp() override;

    virtual std::string GroupName() override;
    virtual unsigned int GetNumDetectors() override;
    virtual std::vector<const std::string*> GetDetectorNames() const override;

    void RegisterI2CBus(i2c_smbus_interface *bus);
    void RegisterI2CBusDetector(I2CBusDetectorFunction detector);
    void RegisterI2CDeviceDetector(std::string name, I2CDeviceDetectorFunction detector);
    void RegisterI2CDRAMDeviceDetector(std::string name, I2CDRAMDeviceDetectorFunction detector, uint16_t jedec_id, uint8_t dram_type);
    void RegisterI2CPCIDeviceDetector(std::string name, I2CPCIDeviceDetectorFunction detector, uint16_t ven_id, uint16_t dev_id, uint16_t subven_id, uint16_t subdev_id, uint8_t i2c_addr);

    bool I2CInterfaceFailure();
    std::vector<i2c_smbus_interface*>& GetI2CBuses();

private:

    void DetectBuses();
    DetectedControllers DetectDevices();
    DetectedControllers DetectDRAMDevices();
    DetectedControllers DetectPCIDevices();

    bool IsAnyDimmDetectorEnabled();

    bool                                        i2c_interface_fail;
    std::vector<i2c_smbus_interface*>           i2c_buses;
    std::vector<I2CBusDetectorFunction>         i2c_bus_detectors;
    std::vector<I2CDeviceDetectorBlock>         i2c_device_detectors;
    std::vector<I2CDRAMDeviceDetectorBlock>     i2c_dram_device_detectors;
    std::vector<I2CPCIDeviceDetectorBlock>      i2c_pci_device_detectors;
};
