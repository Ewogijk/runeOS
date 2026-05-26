
/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef RUNEOS_DEVICEMODULETEST_H
#define RUNEOS_DEVICEMODULETEST_H

#include <Device/DeviceModule.h>

#include <Test/Heimdall/Heimdall.h>

#include <KRE/System/System.h>

using namespace Rune;

// ============================================================================================== //
// Test Environment
// ============================================================================================== //

const auto     DUMMY_DEV_ID     = Device::BasicDeviceID("Dummy Dev");
constexpr auto DUMMY_DEV_HANDLE = static_cast<Device::Handle>(0xFFFF);

class DummyDriver : public Device::Driver {

  public:
    int m_bound_dev_count = 0;

    DummyDriver() = default;

    [[nodiscard]] auto vendor() const -> String override { return "Dummy Inc."; }

    [[nodiscard]] auto version() const -> Version override {
        return {.major = 1, .minor = 0, .patch = 0};
    }

    [[nodiscard]] auto target_device_ID() const -> const Device::DeviceID* override {
        return &DUMMY_DEV_ID;
    }

    auto accept_device(const SharedPointer<Device::Device>& device) -> bool override {
        m_bound_dev_count++;
        return true;
    }

    void remove_device(const SharedPointer<Device::Device>& device) override {
        m_bound_dev_count--;
    }

    auto handle_request(const SharedPointer<Device::Device>& device, Device::IORequest request)
        -> Device::IORequestStatus override {
        SILENCE_UNUSED(device)
        SILENCE_UNUSED(request)
        return Device::IORequestStatus::UNSUPPORTED;
    }
};

auto make_dummy_dev(Device::Handle handle) -> SharedPointer<Device::Device> {
    return SharedPointer<Device::Device>(new Device::BasicDevice(handle,
                                                                 "Dummy Device",
                                                                 "Dummy Inc.",
                                                                 "1.0",
                                                                 "1003",
                                                                 Device::DeviceType::GENERIC,
                                                                 DUMMY_DEV_ID));
}

// ============================================================================================== //
// Test Suite
// ============================================================================================== //

// ========================================================================================== //
// register_device_driver
// ========================================================================================== //

TEST("register_device_driver - No matching devices", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    auto dummy_driver_base = SharedPointer<Device::Driver>(new DummyDriver());
    auto dummy_driver_sub  = SharedPointer<DummyDriver>(dummy_driver_base);
    REQUIRE(ds->register_device_driver(dummy_driver_base))
    REQUIRE(ds->device_driver_store().contains(dummy_driver_base))
    REQUIRE(dummy_driver_sub->m_bound_dev_count == 0)

    // Cleanup
    ds->unregister_device_driver(dummy_driver_base);
}

TEST("register_device_driver - Already registered", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    auto dummy_driver_base = SharedPointer<Device::Driver>(new DummyDriver());
    auto dummy_driver_sub  = SharedPointer<DummyDriver>(dummy_driver_base);
    auto dummy_dev         = make_dummy_dev(ds->get_device_handle());
    REQUIRE(ds->register_device(ds->device_tree(), dummy_dev))
    REQUIRE(ds->register_device_driver(dummy_driver_base))
    REQUIRE(!ds->register_device_driver(dummy_driver_base))
    REQUIRE(dummy_driver_sub->m_bound_dev_count == 1)

    // Cleanup
    ds->unregister_device_driver(dummy_driver_base);
    ds->unregister_device(dummy_dev);
}

TEST("register_device_driver - Driver is null", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    REQUIRE(!ds->register_device_driver(SharedPointer<Device::Driver>()))
}

// ========================================================================================== //
// unregister_device_driver
// ========================================================================================== //

TEST("unregister_device_driver - Registered Driver with bound devices", "DeviceModule") {
    // Setup
    auto* ds         = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  dummy_dev1 = make_dummy_dev(ds->get_device_handle());
    auto  dummy_dev2 = make_dummy_dev(ds->get_device_handle());

    // Test Body
    auto dummy_driver_base = SharedPointer<Device::Driver>(new DummyDriver());
    auto dummy_driver_sub  = SharedPointer<DummyDriver>(dummy_driver_base);
    REQUIRE(ds->register_device_driver(dummy_driver_base))
    REQUIRE(ds->register_device(ds->device_tree(), dummy_dev1))
    REQUIRE(ds->register_device(ds->device_tree(), dummy_dev2))
    REQUIRE(ds->unregister_device_driver(dummy_driver_base))
    REQUIRE(dummy_driver_sub->m_bound_dev_count == 0)
    REQUIRE(dummy_dev1->driver().get() == nullptr)
    REQUIRE(dummy_dev2->driver().get() == nullptr)

    // Cleanup
    ds->unregister_device(dummy_dev1);
    ds->unregister_device(dummy_dev2);
}

TEST("unregister_device_driver - Registered Driver without bound devices", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    auto dummy_driver_base = SharedPointer<Device::Driver>(new DummyDriver());
    auto dummy_driver_sub  = SharedPointer<DummyDriver>(dummy_driver_base);
    REQUIRE(ds->register_device_driver(dummy_driver_base))
    REQUIRE(ds->unregister_device_driver(dummy_driver_base))
    REQUIRE(dummy_driver_sub->m_bound_dev_count == 0)
}

TEST("unregister_device_driver - Unregistered Driver", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    auto dummy_driver = SharedPointer<Device::Driver>(new DummyDriver());

    REQUIRE(!ds->unregister_device_driver(dummy_driver))
}

TEST("unregister_device_driver - Driver is null", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    REQUIRE(!ds->unregister_device_driver(SharedPointer<Device::Driver>()))
}

// ========================================================================================== //
// device_tree
// ========================================================================================== //

TEST("device_tree", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    auto root_device = ds->device_tree();
    REQUIRE(root_device.get() != nullptr)
}

// ========================================================================================== //
// get_devices
// ========================================================================================== //

TEST("get_devices - Valid DeviceType", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    auto generic_devs = ds->get_devices<Device::Device>(Device::DeviceType::GENERIC);
    REQUIRE(!generic_devs.empty())
}

TEST("get_devices - Invalid DeviceType", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    auto generic_devs = ds->get_devices<Device::Device>(Device::DeviceType::NONE);
    REQUIRE(generic_devs.empty())
}

// ========================================================================================== //
// get_device
// ========================================================================================== //

TEST("get_device - Valid Input", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();

    // Test Body
    auto dev = ds->get_device<Device::Device>(root_device->get_handle());
    REQUIRE(dev.get() != nullptr)
    REQUIRE(root_device->device_ID()->equals(root_device->device_ID()))
}

TEST("get_device - Unknown Handle", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    auto dev = ds->get_device<Device::Device>(Resource<Device::Handle>::HANDLE_NONE);
    REQUIRE(dev.get() == nullptr)
}

// ========================================================================================== //
// register_device
// ========================================================================================== //

TEST("register_device - First Time Registration (No driver available)", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto  dummy_dev   = make_dummy_dev(ds->get_device_handle());

    // Test Body
    REQUIRE(ds->register_device(root_device, dummy_dev))
    REQUIRE(ds->get_device<Device::Device>(dummy_dev->get_handle()).get() == dummy_dev.get())
    REQUIRE(dummy_dev->bus_device().get() == root_device.get())
    REQUIRE(root_device->child_devices().contains(dummy_dev))
    REQUIRE(dummy_dev->driver().get() == nullptr)

    // Cleanup
    ds->unregister_device(dummy_dev);
}

TEST("register_device - First Time Registration (Driver available)", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto  dummy_dev   = make_dummy_dev(ds->get_device_handle());
    SharedPointer<Device::Driver> dummy_driver_base(new DummyDriver());
    auto                          dummy_driver_sub = SharedPointer<DummyDriver>(dummy_driver_base);

    // Test Body
    REQUIRE(ds->register_device_driver(dummy_driver_base))
    REQUIRE(ds->register_device(root_device, dummy_dev))
    REQUIRE(ds->get_device<Device::Device>(dummy_dev->get_handle()).get() == dummy_dev.get())
    REQUIRE(dummy_dev->bus_device().get() == root_device.get())
    REQUIRE(root_device->child_devices().contains(dummy_dev))
    REQUIRE(dummy_dev->driver().get() == dummy_driver_base.get())
    REQUIRE(dummy_driver_sub->m_bound_dev_count == 1)

    // Cleanup
    ds->unregister_device(dummy_dev);
    ds->unregister_device_driver(dummy_driver_base);
}

TEST("register_device - Bus Device is null", "DeviceModule") {
    // Setup
    auto* ds        = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  dummy_dev = make_dummy_dev(ds->get_device_handle());

    // Test Body
    REQUIRE(!ds->register_device(SharedPointer<Device::Device>(), dummy_dev))
    REQUIRE(ds->get_device<Device::Device>(dummy_dev->get_handle()).get() == nullptr)

    // Cleanup
    ds->unregister_device(dummy_dev);
}

TEST("register_device - Device is null", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto  dummy_dev   = make_dummy_dev(ds->get_device_handle());

    // Test Body
    REQUIRE(!ds->register_device(root_device, SharedPointer<Device::Device>()))
    REQUIRE(ds->get_device<Device::Device>(dummy_dev->get_handle()).get() == nullptr)
    REQUIRE(!root_device->child_devices().contains(dummy_dev))

    // Cleanup
    ds->unregister_device(dummy_dev);
}

TEST("register_device - Device type is None", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto dummy_dev = SharedPointer<Device::Device>(new Device::BasicDevice(ds->get_device_handle(),
                                                                           "Dummy Device",
                                                                           "Dummy Inc.",
                                                                           "1.0",
                                                                           "1003",
                                                                           Device::DeviceType::NONE,
                                                                           DUMMY_DEV_ID));

    // Test Body
    REQUIRE(!ds->register_device(root_device, dummy_dev))
    REQUIRE(ds->get_device<Device::Device>(dummy_dev->get_handle()).get() == nullptr)
    REQUIRE(dummy_dev->bus_device().get() == nullptr)
    REQUIRE(!root_device->child_devices().contains(dummy_dev))

    // Cleanup
    ds->unregister_device(dummy_dev);
}

TEST("register_device - Device already registered", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto  dummy_dev   = make_dummy_dev(ds->get_device_handle());

    // Test Body
    REQUIRE(ds->register_device(root_device, dummy_dev))
    REQUIRE(!ds->register_device(root_device, dummy_dev))

    // Cleanup
    ds->unregister_device(dummy_dev);
}

TEST("register_device - Unknown bus device", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  dummy_bus_dev = make_dummy_dev(ds->get_device_handle());
    auto  dummy_dev     = make_dummy_dev(ds->get_device_handle());

    // Test Body
    REQUIRE(!ds->register_device(dummy_bus_dev, dummy_dev))
    REQUIRE(ds->get_device<Device::Device>(dummy_dev->get_handle()).get() == nullptr)
    REQUIRE(dummy_dev->bus_device().get() == nullptr)
    REQUIRE(!dummy_bus_dev->child_devices().contains(dummy_dev))

    // Cleanup
    ds->unregister_device(dummy_dev);
}

TEST("register_device - Bus Device and Device are the same device", "DeviceModule") {
    // Setup
    auto* ds        = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  dummy_dev = make_dummy_dev(ds->get_device_handle());

    // Test Body
    REQUIRE(!ds->register_device(dummy_dev, dummy_dev))
    REQUIRE(ds->get_device<Device::Device>(dummy_dev->get_handle()).get() == nullptr)
    REQUIRE(dummy_dev->bus_device().get() == nullptr)
    REQUIRE(!dummy_dev->child_devices().contains(dummy_dev))

    // Cleanup
    ds->unregister_device(dummy_dev);
}

// ========================================================================================== //
// unregister_device
// ========================================================================================== //

TEST("unregister_device - Registered Device with Driver", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto  dummy_driver_base = SharedPointer<Device::Driver>(new DummyDriver());
    auto  dummy_driver_sub  = SharedPointer<DummyDriver>(dummy_driver_base);
    auto  dummy_dev         = make_dummy_dev(ds->get_device_handle());

    // Test Body
    REQUIRE(ds->register_device_driver(dummy_driver_base))
    REQUIRE(ds->register_device(root_device, dummy_dev))
    REQUIRE(ds->unregister_device(dummy_dev))
    REQUIRE(dummy_dev->driver().get() == nullptr)
    REQUIRE(dummy_dev->bus_device().get() == nullptr)
    REQUIRE(!root_device->child_devices().contains(dummy_dev))
    REQUIRE(dummy_driver_sub->m_bound_dev_count == 0)

    // Cleanup
    ds->unregister_device_driver(dummy_driver_base);
}

TEST("unregister_device - Registered Device without Driver", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto  dummy_dev   = make_dummy_dev(ds->get_device_handle());

    REQUIRE(ds->register_device(root_device, dummy_dev))
    REQUIRE(ds->unregister_device(dummy_dev))
    REQUIRE(dummy_dev->driver().get() == nullptr)
    REQUIRE(dummy_dev->bus_device().get() == nullptr)
    REQUIRE(!root_device->child_devices().contains(dummy_dev))
}

TEST("unregister_device - Non-registered Device", "DeviceModule") {
    // Setup
    auto* ds        = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  dummy_dev = make_dummy_dev(ds->get_device_handle());

    // Test Body
    REQUIRE(!ds->unregister_device(dummy_dev))
}

TEST("unregister_device - Device is null", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    REQUIRE(!ds->unregister_device(SharedPointer<Device::Device>()))
}

TEST("unregister_device - The root device", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    REQUIRE(!ds->unregister_device(ds->device_tree()))
}

// ========================================================================================== //
// control_device
// ========================================================================================== //

TEST("control_device - Valid Input", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto  dummy_driver = SharedPointer<Device::Driver>(new DummyDriver());
    auto  dummy_dev    = make_dummy_dev(ds->get_device_handle());

    // Test Body
    Device::IORequest req{.m_in_buffer = nullptr, .m_out_buffer = nullptr};
    REQUIRE(ds->register_device_driver(dummy_driver))
    REQUIRE(ds->register_device(root_device, dummy_dev))
    REQUIRE(ds->control_device(dummy_dev->get_handle(), req)
            == Device::IORequestStatus::UNSUPPORTED)

    // Cleanup
    ds->unregister_device_driver(dummy_driver);
    ds->unregister_device(dummy_dev);
}

TEST("control_device - Unknown Device", "DeviceModule") {
    // Setup
    auto* ds = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);

    // Test Body
    Device::IORequest       req{.m_in_buffer = nullptr, .m_out_buffer = nullptr};
    Device::IORequestStatus req_status =
        ds->control_device(Resource<Device::Handle>::HANDLE_NONE, req);
    REQUIRE(req_status == Device::IORequestStatus::UNKNOWN_DEVICE)
}

TEST("control_device - Non-operational Device", "DeviceModule") {
    // Setup
    auto* ds          = System::instance().get_module<Device::DeviceModule>(ModuleSelector::DEVICE);
    auto  root_device = ds->device_tree();
    auto  dummy_dev   = make_dummy_dev(ds->get_device_handle());

    // Test Body
    Device::IORequest req{.m_in_buffer = nullptr, .m_out_buffer = nullptr};
    REQUIRE(ds->register_device(root_device, dummy_dev))
    REQUIRE(ds->control_device(dummy_dev->get_handle(), req)
            == Device::IORequestStatus::DEVICE_NOT_OPERATIONAL)

    // Cleanup
    ds->unregister_device(dummy_dev);
}

#endif // RUNEOS_DEVICEMODULETEST_H
