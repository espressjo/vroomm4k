#ifndef INSTHANDLE_H
#define INSTHANDLE_H

#include <cstdint>
#include <labdev/devicesManager.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <stdint.h>
#include <string>
#include <uics/states.h>
//:::::::::::::::::::::::::::::::::::::::::::::::
//:::   All stuff related to Macie hardware   :::
//:::::::::::::::::::::::::::::::::::::::::::::::




#pragma pack(push, 1)
typedef struct {

  prometheus::Exposer *exposer;
  DeviceManager *devManager;
  std::shared_ptr<prometheus::Registry> registry;
} DataServer;
#pragma pack(pop)

typedef struct {

  // UICS stuff
  STATE nextState;
  STATE state;
  STATUS status;
  std::string log;       // initHandle
  std::string root_path; // initHandle     //chaged to only 1 path /opt/astro
  DataServer dataserver;
} instHandle;

#endif // INSTHANDLE_H
