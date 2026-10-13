#include "vroomm4k_devices.h"
#include <labdev/devicesManager.h>
#include <labdev/dummydevice.h>
#include <labdev/lakeshore336.h>
#include <labdev/mks972b-ethercat.h>
#include <labdev/mks972b-serial.h>
#include <labdev/sunpowerMT.h>
#include <memory>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

void setup_devices(instHandle *handle) {

  handle->dataserver.exposer = new prometheus::Exposer("0.0.0.0:9878");
  handle->dataserver.registry = std::make_shared<prometheus::Registry>();
  handle->dataserver.devManager = new DeviceManager(
      handle->dataserver.registry.get(), std::chrono::milliseconds(5000));

  //::::::::::::::::::::
  //:::    Devices   :::
  //::::::::::::::::::::
  //
  // Dummy Device
  std::shared_ptr<dummydevice> d_device = std::make_shared<dummydevice>(
      "D_device", handle->dataserver.registry.get());
  //  d_device->set_timeout(5);
  //
  // Gauge 972b
  std::shared_ptr<gauge972_e> d_pressure = std::make_shared<gauge972_e>(
      "/opt/vroomm4k/config/mks972b.cfg", DEV_PRESSURE,
      handle->dataserver.registry.get());
  // d_pressure->set_timeout(5);
  //
  //  Lakeshore 336
  std::shared_ptr<lakeshore336> d_lakeshore = std::make_shared<lakeshore336>(
      "/opt/vroomm4k/config/lakeshore.cfg", "D_Lakeshore", -1, true,
      handle->dataserver.registry.get());

  // sunpower
  std::shared_ptr<sunpowerMT> d_sunpower = std::make_shared<sunpowerMT>(
      "/opt/vroomm4k/config/sunpower.cfg", "D_Sunpower",
      handle->dataserver.registry.get());
  // handle->d_filterwheel = d_filterwheel.get();
  handle->dataserver.exposer->RegisterCollectable(handle->dataserver.registry);

  //handle->dataserver.devManager->addDevice(d_filterwheel);
  handle->dataserver.devManager->addDevice(d_pressure);
  handle->dataserver.devManager->addDevice(d_sunpower);
  handle->dataserver.devManager->addDevice(d_lakeshore);
  handle->dataserver.devManager->addDevice(d_device);
  handle->dataserver.devManager->start();

  return;
}
