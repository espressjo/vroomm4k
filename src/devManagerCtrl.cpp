#include "devManagerCtrl.h"
#include "vroomm4k_devices.h"
#include "uics/uics_err_code.h"
#include <chrono>
#include <sstream>

void dev_ctrl_status(instHandle *handle, cmd *cc) {
  //:::::::::::::::::::::::::::::::::::::
  //:::   list the state of devices   :::
  //:::::::::::::::::::::::::::::::::::::
  if ((*cc)["-print"].compare("") != 0) {
    std::vector<std::string> list =
        handle->dataserver.devManager->getDeviceNames();
    std::string msg = "\n";
    int _polling = 0;
    for (auto devName : list) {
      _polling = handle->dataserver.devManager->getDevicePollingTime(devName);
      if (handle->dataserver.devManager->isDeviceEnabled(devName)) {
        msg += devName + " " + std::to_string(_polling) + " ms" +
               " [\033[32m enabled\033[0m ]\n";
      } else {
        msg += devName + " " + std::to_string(_polling) + " ms" +
               " [\033[31m disabled\033[0m ]\n";
      }
    } // for loop
    cc->respond(msg);
    return;
  }

  else if ((*cc)["-status"].compare("") != 0) {
    std::vector<std::string> list =
        handle->dataserver.devManager->getDeviceNames();
    std::string msg = "\n";
    int _polling = 0;
    for (auto devName : list) {
      //_polling = handle->dataserver.devManager->getDevicePollingTime(devName);
      if (handle->dataserver.devManager->deviceOnline(devName)) {

        msg += devName + " " + " [\033[32m Online\033[0m ]\n";
      } else {
        msg += devName + " " + " [\033[31m Offline\033[0m ]\n";
      }
    } // for loop
    cc->respond(msg);
    return;
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //:::   last value read by each device on its update()   :::
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  else if ((*cc)["-values"].compare("") != 0) {
    std::vector<std::string> list;
    if ((*cc)["dev"].compare("") != 0) {
      list.push_back((*cc)["dev"]);
    } else {
      list = handle->dataserver.devManager->getDeviceNames();
    }
    std::ostringstream msg;
    msg << "\n";
    double value = 0.0;
    for (auto devName : list) {
      for (auto gaugeName :
           handle->dataserver.devManager->getDeviceGaugeNames(devName)) {
        if (handle->dataserver.devManager->getDeviceValue(devName, gaugeName,
                                                          value)) {
          msg << devName << " " << gaugeName << " " << value << "\n";
        }
      }
    } // for loop
    cc->respond(msg.str());
    return;
  }

  //:::::::::::::::::::::::::::
  //:::   Trigger devices   :::
  //:::::::::::::::::::::::::::

  // first get the device name
  std::string dev = "";
  int polling_ms = 0;
  if ((*cc)["dev"].compare("") != 0) {
    dev = (*cc)["dev"];

    if (!handle->dataserver.devManager->deviceExist(dev)) {
      cc->respond("given device does not exist", uicsCMD_ERR_PARAM_FORMAT);
      return;
    }
  } else {
    cc->respond("Expect a device to continue", uicsCMD_ERR_PARAM_FORMAT);
    return;
  }

  if ((*cc)["-disable"].compare("") != 0) {
    if (!handle->dataserver.devManager->disableDevice(dev)) {
      cc->respond("Fail to disable device", uicsCMD_ERR_PARAM_UNKNOWN);
      return;
    }
    cc->respond();
    return;
  }
  if ((*cc)["-enable"].compare("") != 0) {
    if (!handle->dataserver.devManager->enableDevice(dev)) {
      cc->respond("Fail to enable device", uicsCMD_ERR_PARAM_UNKNOWN);
      return;
    }
    cc->respond();
    return;
  }

  if ((*cc)["polling"].compare("") != 0) {
    polling_ms = std::stod((*cc)["polling"]);
    auto ms = std::chrono::milliseconds(polling_ms);
    handle->dataserver.devManager->setDevicePollInterval(dev, ms);
    cc->respond();
    return;
  }

  cc->respond();
  return;
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//:::   Last pressure (Torr) read by the MKS972b. No I/O is   :::
//:::   done here, we only read back what update() cached.    :::
//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
void pressure_status(instHandle *handle, cmd *cc) {
  double pressure = 0.0;

  if (!handle->dataserver.devManager->getDeviceValue(DEV_PRESSURE,
                                                     GAUGE_PRESSURE,
                                                     pressure)) {
    cc->respond("Pressure gauge is not available", uicsCMD_ERR_PARAM_UNKNOWN);
    return;
  }

  if (!handle->dataserver.devManager->deviceOnline(DEV_PRESSURE)) {
    cc->respond("Pressure gauge is offline", uicsCMD_ERR_PARAM_UNKNOWN);
    return;
  }

  std::ostringstream msg;
  msg << pressure;
  cc->respond(msg.str());
  return;
}
