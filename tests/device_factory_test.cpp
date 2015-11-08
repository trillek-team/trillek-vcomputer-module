#include <gtest/gtest.h>

#include <memory>

#include "device_factory.hpp"
#include "device.hpp"

class TestDevice : public trillek::computer::Device
{
public:
    TestDevice() {
        vcomp = nullptr;
    }

    void Reset() {}
    void SendCMD(trillek::Word) {}

    trillek::Byte DevType() const override {return 1; }
    trillek::Byte DevSubType() const override {return 2;}
    trillek::Byte DevID() const override {return 3;}
    trillek::DWord DevVendorID() const override {return 4;}

    void GetState(void*, std::size_t&) const override {}
    bool SetState(const void *ptr, std::size_t size) override {return true;}

    static Device* CreateNew() { return new TestDevice(); }
};

TEST(DeviceFactory, DefaultDevices)
{
    trillek::computer::registerDefaultDevices();

    std::shared_ptr<trillek::computer::Device> tda = trillek::computer::DeviceFactory::GetInstance()->CreateDevice(0x0E, 0x01, 0x01, 0x1C6C8B36);

    ASSERT_TRUE(tda.get() != nullptr);

    trillek::computer::DeviceFactory::Destroy();
}

TEST(DeviceFactory, GlobalTest)
{
    trillek::computer::DeviceFactory::GetInstance()->RegisterNewDevice(trillek::computer::DeviceRecord(1, 2, 3, 4, &TestDevice::CreateNew));

    std::shared_ptr<trillek::computer::Device> test_dev = trillek::computer::DeviceFactory::GetInstance()->CreateDevice(1, 2, 3, 4);
    ASSERT_TRUE(test_dev.get() != nullptr);
    trillek::computer::DeviceFactory::GetInstance()->UnregisterDevice(1, 2, 3, 4);
    std::shared_ptr<trillek::computer::Device> test_dev2 = trillek::computer::DeviceFactory::GetInstance()->CreateDevice(1, 2, 3, 4);
    ASSERT_TRUE(test_dev2 == nullptr);
}
