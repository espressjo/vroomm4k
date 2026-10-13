#ifndef VROOMM4K_DEVICES_H
#define VROOMM4K_DEVICES_H

#include "insthandle.h"
#include <labdev/devicesManager.h>
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::   Device and gauge names, as registered in setup_devices.   :::
//:::   The gauge names are built by the labdev device itself,    :::
//:::   keep them in sync when a device name changes.             :::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
constexpr const char *DEV_PRESSURE = "D_Pressure";
constexpr const char *GAUGE_PRESSURE = "D_Pressure_Pressure";

void setup_devices(instHandle *handle);
#endif
