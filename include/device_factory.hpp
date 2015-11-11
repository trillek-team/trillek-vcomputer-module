#ifndef __DEVICE_FACTORY_HPP__
#define __DEVICE_FACTORY_HPP__ 1

#include <list>
#include <memory>
#include <functional>

#include "types.hpp"
#include "vc_dll.hpp"

#include "device.hpp"

namespace trillek {
namespace computer {

void DECLDIR registerDefaultDevices();

struct DECLDIR DeviceRecord
{
    Byte devType;
    Byte devSubType;
    Byte devID;
    DWord vendorID;
    std::function<Device*()> creator; ///Pointer to a function to invoke when this device must be instanciated.

    DeviceRecord(Byte devType, Byte devSubType, Byte devId, DWord vendorID, std::function<Device*()> creator);
    bool deviceMatch(Byte devType, Byte devSubType, Byte devId, DWord vendorID) const;
};

/**
 * \class DeviceFactory Given a device type, sub type, device id and vendor id,
 * this class return a new corresponding Device. Devices must be registred in
 * order to be created through this class.
 */
class DECLDIR DeviceFactory
{
    std::list< DeviceRecord > d_deviceTemplates; ///A vector of all available devices template

    static DeviceFactory* d_instance;

    DeviceFactory()=default;
    DeviceFactory(const DeviceFactory& other)=delete;
    ~DeviceFactory()=default;

    void operator=(const DeviceFactory&)=delete;

public:

    /**
     * Return the current instance of the device factory.
     * If none exists, create a new one.
     */
    static DeviceFactory* GetInstance();

    /**
     * Destroy the factory if exists.
     */
    static void Destroy();

    /**
     * Add a new device to the list of registred device template.
     */
    void RegisterNewDevice(const DeviceRecord& rec);

    /**
     * Given a device type, sub type, device id and vendor id, construct a new
     * corresponding Device. If the corresponding record is not found, return a
     * nullptr.
     *
     *The factory does not own the returned pointer anymore
     */
    std::shared_ptr<Device> CreateDevice(Byte devType, Byte devSubType, Byte devId, DWord devVendorId);

    /**
     * Remove a device from the list of registred device template.
     */
    void UnregisterDevice(Byte devType, Byte devSubType, Byte devId, DWord devVendorId);
};

}
}

#endif
