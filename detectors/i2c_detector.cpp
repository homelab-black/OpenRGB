#include "i2c_detector.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "pci_ids.h"

I2CDetectorGroup::I2CDetectorGroup(DetectionManager* dm) : DetectorGroup(dm)
{
}

I2CDetectorGroup::~I2CDetectorGroup()
{
    for (i2c_smbus_interface* bus : i2c_buses)
    {
        delete bus;
    }
}

/*---------------------------------------------------------*\
| Functions for registering I2C buses from within detectors |
\*---------------------------------------------------------*/
void I2CDetectorGroup::RegisterI2CBus(i2c_smbus_interface *bus)
{
    LOG_INFO("[%s] Registering I2C interface: %s Device %04X:%04X Subsystem: %04X:%04X", GroupName().c_str(), bus->device_name, bus->pci_vendor, bus->pci_device, bus->pci_subsystem_vendor, bus->pci_subsystem_device);

    /*-----------------------------------------------------*\
    | Add the new bus to the list                           |
    \*-----------------------------------------------------*/
    i2c_buses.push_back(bus);

    /*-----------------------------------------------------*\
    | Signal I2C Bus List Update                            |
    \*-----------------------------------------------------*/
    SignalUpdate(DETECTIONMANAGER_UPDATE_REASON_I2C_BUS_REGISTERED);
}

void I2CDetectorGroup::RegisterI2CBusDetector(I2CBusDetectorFunction detector)
{
    LOG_TRACE("[%s] Registering I2C Bus detector", GroupName().c_str());
    i2c_bus_detectors.push_back(detector);
}

void I2CDetectorGroup::RegisterI2CDeviceDetector(std::string name, I2CDeviceDetectorFunction detector)
{
    LOG_TRACE("[%s] Registering I2C Device detector %s", GroupName().c_str(), name.c_str());
    I2CDeviceDetectorBlock block
    {
        .name = name,
        .function = detector,
    };
    i2c_device_detectors.push_back(block);
}

void I2CDetectorGroup::RegisterI2CDRAMDeviceDetector(std::string name, I2CDRAMDeviceDetectorFunction detector, uint16_t jedec_id, uint8_t dram_type)
{
    LOG_TRACE("[%s] Registering I2C DRAM Detector %s", GroupName().c_str(), name.c_str());
    I2CDRAMDeviceDetectorBlock block
    {
        .name          = name,
        .function      = detector,
        .jedec_id      = jedec_id,
        .dram_type     = dram_type,
    };

    i2c_dram_device_detectors.push_back(block);
}

void I2CDetectorGroup::RegisterI2CPCIDeviceDetector(std::string name, I2CPCIDeviceDetectorFunction detector, uint16_t ven_id, uint16_t dev_id, uint16_t subven_id, uint16_t subdev_id, uint8_t i2c_addr)
{
    LOG_TRACE("[%s] Registering I2C PCI Detector %s", GroupName().c_str(), name.c_str());
    I2CPCIDeviceDetectorBlock block
    {
        .name          = name,
        .function      = detector,
        .ven_id        = ven_id,
        .dev_id        = dev_id,
        .subven_id     = subven_id,
        .subdev_id     = subdev_id,
        .i2c_addr      = i2c_addr,
    };

    i2c_pci_device_detectors.push_back(block);
}

std::string I2CDetectorGroup::GroupName()
{
    return "I2C Detector";
}

unsigned int I2CDetectorGroup::GetNumDetectors()
{
    return i2c_device_detectors.size()
            + i2c_dram_device_detectors.size()
            + i2c_pci_device_detectors.size();
}


std::vector<const std::string*> I2CDetectorGroup::GetDetectorNames() const
{
    std::vector<const std::string*> names;
    // todo: fix
    for (const auto& thing : i2c_device_detectors)
    {
        names.push_back(&thing.name);
    }
    for (const auto& thing : i2c_dram_device_detectors)
    {
        names.push_back(&thing.name);
    }

    for (const auto& thing : i2c_pci_device_detectors)
    {
        names.push_back(&thing.name);
    }

    return names;
}


std::vector<i2c_smbus_interface*>& I2CDetectorGroup::GetI2CBuses()
{
    return i2c_buses;
}

void I2CDetectorGroup::Prepare()
{
    /*--------------------------*\
    | Clear any existing buses   |
    \*--------------------------*/
    for(i2c_smbus_interface* bus : i2c_buses)
    {
        delete bus;
    }
    i2c_buses.clear();
}

void I2CDetectorGroup::CleanUp()
{
}

bool I2CDetectorGroup::I2CInterfaceFailure()
{
    return i2c_interface_fail;
}

void I2CDetectorGroup::RunDetection()
{
    /*---------------------------*\
    | Detect buses must run first |
    \*---------------------------*/
    DetectBuses();
    DetectedControllers found_devices = DetectDevices();
    DetectedControllers found_dram = DetectDRAMDevices();
    DetectedControllers found_pci = DetectPCIDevices();

    /*-------------------------------------*\
    | put them all in 1 vec for convenience |
    | this is gonna have like length 10 max |
    | who's gonna have more i2c devices?    |
    \*-------------------------------------*/
    DetectedControllers found = DetectedControllers {};
    found.insert(found.end(), found_devices.begin(), found_devices.end());
    found.insert(found.end(), found_dram.begin(), found_dram.end());
    found.insert(found.end(), found_pci.begin(), found_pci.end());

    for (RGBController* ctrl : found)
    {
        RegisterRGBController(ctrl);
    }
}

void I2CDetectorGroup::DetectBuses()
{
    LOG_INFO("------------------------------------------------------");
    LOG_INFO("|             Detecting I2C interfaces               |");
    LOG_INFO("------------------------------------------------------");

    OnStartDetection("Detecting I2C Interfaces");
    i2c_interface_fail = false;

    for(std::size_t i2c_bus_detector_idx = 0; i2c_bus_detector_idx < i2c_bus_detectors.size(); i2c_bus_detector_idx++)
    {
        if(i2c_bus_detectors[i2c_bus_detector_idx]() == false)
        {
            i2c_interface_fail = true;
        }
    }
}

DetectedControllers I2CDetectorGroup::DetectDevices()
{
    LOG_INFO("------------------------------------------------------");
    LOG_INFO("|               Detecting I2C devices                |");
    LOG_INFO("------------------------------------------------------");

    OnStartDetection("Detecting I2C Devices");

    DetectedControllers found;

    for (I2CDeviceDetectorBlock& detector : i2c_device_detectors)
    {
        if (!IsDetectionEnabledFor(detector.name))
        {
            OnSkipDetection(detector.name, DETECTOR_DISABLED_REASON);
            continue;
        }

        OnStartDetection(detector.name);
        DetectedControllers new_found = detector.function(i2c_buses);
        found.insert(found.end(), new_found.begin(), new_found.end());
        OnEndDetection(detector.name);
    }
    return found;
}

DetectedControllers I2CDetectorGroup::DetectDRAMDevices()
{
    LOG_INFO("------------------------------------------------------");
    LOG_INFO("|            Detecting I2C DRAM modules              |");
    LOG_INFO("------------------------------------------------------");

    OnStartDetection("Detecting I2C DRAM Modules");

    DetectedControllers found;
    if (IsAnyDimmDetectorEnabled())
    {
        LOG_DEBUG("[%s] No DIMM detectors enabled, skipping", GroupName().c_str());
        return found;
    }

    for(std::size_t idx = 0; idx < i2c_buses.size(); idx++)
    {
        i2c_smbus_interface* bus = i2c_buses[idx];
        if(!IS_DRAM_SMBUS(bus->pci_vendor, bus->pci_device))
        {
            continue;
        }

        std::vector<SPDWrapper> slots;
        SPDMemoryType dram_type = SPD_RESERVED;

        for(uint8_t spd_addr = 0x50; spd_addr < 0x58; spd_addr++)
        {
            /*-----------------------------------------------*\
            | dram_type changes in the loop, is this correct? |
            | Shouldn't it always start as `SPD_RESERVED`?    |
            \*-----------------------------------------------*/
            SPDDetector spd(bus, spd_addr, dram_type);
            if(spd.is_valid())
            {
                SPDWrapper accessor(spd);
                dram_type = spd.memory_type();
                LOG_INFO("[%s] Detected occupied slot %d, bus %d, type %s", GroupName().c_str(), spd_addr - 0x50 + 1, idx, spd_memory_type_name[dram_type]);
                LOG_DEBUG("[%s] JEDEC ID: 0x%04x", GroupName().c_str(), accessor.jedec_id());
                slots.push_back(accessor);
            }
        }

        for(I2CDRAMDeviceDetectorBlock& detector : i2c_dram_device_detectors)
        {
            if(!(detector.dram_type == dram_type && is_jedec_in_slots(slots, detector.jedec_id)))
            {
                OnSkipDetection(detector.name, "Detector is incorrect DRAM type");
                continue;
            }
            if(!IsDetectionEnabledFor(detector.name))
            {
                OnSkipDetection(detector.name, DETECTOR_DISABLED_REASON);
                continue;
            }
            OnStartDetection(detector.name);

            std::vector<SPDWrapper*> matching_slots = slots_with_jedec(slots, detector.jedec_id);
            DetectedControllers new_found = detector.function(bus, matching_slots, detector.name);
            found.insert(found.end(), new_found.begin(), new_found.end());
            OnEndDetection(detector.name);
        }
    }
    return found;
}

DetectedControllers I2CDetectorGroup::DetectPCIDevices()
{
    LOG_INFO("------------------------------------------------------");
    LOG_INFO("|               Detecting I2C PCI devices            |");
    LOG_INFO("------------------------------------------------------");
    OnStartDetection("Detecting I2C PCI Devices");
    DetectedControllers found;

    for(I2CPCIDeviceDetectorBlock& detector : i2c_pci_device_detectors)
    {
        if(!IsDetectionEnabledFor(detector.name))
        {
            OnSkipDetection(detector.name, DETECTOR_DISABLED_REASON);
            continue;
        }

        OnStartDetection(detector.name);
        for(i2c_smbus_interface* bus : i2c_buses)
        {
            if(!(bus->pci_vendor            == detector.ven_id    &&
                bus->pci_device           == detector.dev_id    &&
                bus->pci_subsystem_vendor == detector.subven_id &&
                bus->pci_subsystem_device == detector.subdev_id))
            {
                continue;
            }

            DetectedControllers new_found = detector.function(
                bus,
                detector.i2c_addr,
                detector.name
            );
            found.insert(found.end(), new_found.begin(), new_found.end());
        }
        OnEndDetection(detector.name);
    }
    return found;
}


bool I2CDetectorGroup::IsAnyDimmDetectorEnabled()
{
    const json& detector_settings = GetDetectorSettings();
    for(std::size_t i2c_detector_idx = 0; i2c_detector_idx < i2c_dram_device_detectors.size(); i2c_detector_idx++)
    {
        const std::string& detector_name_string = i2c_dram_device_detectors[i2c_detector_idx].name.c_str();
        /*-------------------------------------------------*\
        | Check if this detector is enabled                 |
        \*-------------------------------------------------*/
        if(detector_settings.contains("detectors") && detector_settings["detectors"].contains(detector_name_string) &&
           detector_settings["detectors"][detector_name_string] == true)
        {
            return true;
        }
    }
    return false;
}
