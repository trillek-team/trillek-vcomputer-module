#include "device_factory.hpp"
#include "device.hpp"
#include <memory>

#include "devices/gkeyb.hpp"
#include "devices/tda.hpp"
#include "devices/m5fdd.hpp"

namespace trillek {
namespace computer {

void registerDefaultDevices()
{
    DeviceFactory::GetInstance()->RegisterNewDevice(DeviceRecord(3, 1, 1, 0, &gkeyboard::GKeyboardDev::CreateNew));
    DeviceFactory::GetInstance()->RegisterNewDevice(DeviceRecord(0x0E, 0x01, 0x01, 0x1C6C8B36, &tda::TDADev::CreateNew));
    DeviceFactory::GetInstance()->RegisterNewDevice(DeviceRecord(0x08, 0x01, 0x01, 0x1EB37E91, &m5fdd::M5FDD::CreateNew));
}

DeviceRecord::DeviceRecord(Byte devType, Byte devSubType, Byte devId, DWord vendorID, std::function<Device *()> creator) : devType(devType), devSubType(devSubType), devID(devId), vendorID(vendorID), creator(creator) {}

bool DeviceRecord::deviceMatch(Byte devType, Byte devSubType, Byte devId, DWord vendorID) const
{
    return (this->devType == devType &&
            this->devSubType == devSubType &&
            this->devID == devId &&
            this->vendorID == vendorID);
}

DeviceFactory* DeviceFactory::d_instance = nullptr;

DeviceFactory* DeviceFactory::GetInstance()
{
    if(!d_instance)
    {
        d_instance = new DeviceFactory();
    }

    return d_instance;
}

void DeviceFactory::Destroy()
{
    if(d_instance)
    {
        delete d_instance;
        d_instance = nullptr;
    }
}

void DeviceFactory::RegisterNewDevice(const DeviceRecord &rec)
{
    d_deviceTemplates.push_back(rec);
}

std::shared_ptr<Device> DeviceFactory::CreateDevice(Byte devType, Byte devSubType, Byte devId, DWord devVendorId)
{
    auto it = std::find_if(d_deviceTemplates.begin(), d_deviceTemplates.end(), [devType, devSubType, devId, devVendorId](const DeviceRecord& d)
    {
        return d.deviceMatch(devType, devSubType, devId, devVendorId);
    });

    return std::shared_ptr<Device>((it == d_deviceTemplates.end() ? nullptr : (*it).creator()));
}

void DeviceFactory::UnregisterDevice(Byte devType, Byte devSubType, Byte devId, DWord devVendorId)
{
    d_deviceTemplates.remove_if([devType, devSubType, devId, devVendorId] (const DeviceRecord& o)
    {
        return  o.deviceMatch(devType, devSubType, devId, devVendorId);
    });
}

}
}
