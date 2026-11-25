#include "vroomm4k_standard_cmd.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <labdev/sunpowerMT.h>
#include <sstream>
#include <string>
#include <thread>
#include <uics/uics_config.h>
#include <uics/uics_err_code.h>
#include <uics/uics_main.h>
#include <uics/uics_number.h>

#define SUNPOWER_NAME "D_Sunpower"

static std::string to_upper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return s;
}

// cooler set <on|off> | vibe <ON|OFF> [-feature <WAVE|HEAD>] | -status | -vibe
// The serial I/O of sunpowerMT runs on the caller's thread, so the device is
// disabled in the DeviceManager (stops its polling) during the transaction.
void cooler(instHandle *handle, cmd *cc) {
  auto *mgr = handle->dataserver.devManager;

  if ((*cc)["set"].compare("") != 0) {
    std::string ctrl = to_upper((*cc)["set"]);
    if (ctrl != "ON" && ctrl != "OFF") {
      cc->respond("set must be ON or OFF", uicsCMD_ERR_PARAM_VALUE);
      return;
    }
    auto it = mgr->getDevice<sunpowerMT>(SUNPOWER_NAME);
    if (!it) {
      cc->respond("failed to acquire resources", uicsCMD_ERR_PARAM_UNKNOWN);
      return;
    }
    mgr->disableDevice(SUNPOWER_NAME);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bool ok = (ctrl == "ON") ? it->setCoolerOn() : it->setCoolerOff();
    mgr->enableDevice(SUNPOWER_NAME);
    if (!ok) {
      cc->respond("failed to set cooler " + ctrl, uicsCMD_ERR_NOT_SUPPORTED);
    } else {
      cc->respond();
    }
    return;
  }

  // Active Vibration Cancellation: cooler vibe <ON|OFF> [-feature <WAVE|HEAD>]
  if ((*cc)["vibe"].compare("") != 0) {
    std::string ctrl = to_upper((*cc)["vibe"]);
    std::string feature = "WAVE"; // default: wave generator cancellation
    if ((*cc)["-feature"].compare("") != 0) {
      feature = to_upper((*cc)["-feature"]);
    }
    if (ctrl != "ON" && ctrl != "OFF") {
      cc->respond("vibe must be ON or OFF", uicsCMD_ERR_PARAM_VALUE);
      return;
    }
    if (feature != "WAVE" && feature != "HEAD") {
      cc->respond("-feature must be WAVE or HEAD", uicsCMD_ERR_PARAM_VALUE);
      return;
    }
    auto it = mgr->getDevice<sunpowerMT>(SUNPOWER_NAME);
    if (!it) {
      cc->respond("failed to acquire resources", uicsCMD_ERR_PARAM_UNKNOWN);
      return;
    }
    mgr->disableDevice(SUNPOWER_NAME);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::string buff;
    // syntax: VIBE=<FEATURE><SP><CONTROL>
    // (sunpowerMT::AVC() is declared but not implemented in labdev)
    bool ok = it->basic_cmd("VIBE=" + feature + " " + ctrl, buff);
    mgr->enableDevice(SUNPOWER_NAME);
    if (!ok) {
      cc->respond("failed to set vibe " + feature + " " + ctrl,
                  uicsCMD_ERR_NOT_SUPPORTED);
    } else {
      cc->respond();
    }
    return;
  }

  if ((*cc)["-status"].compare("") != 0) {
    auto it = mgr->getDevice<sunpowerMT>(SUNPOWER_NAME);
    if (!it) {
      cc->respond("failed to acquire resources", uicsCMD_ERR_PARAM_UNKNOWN);
      return;
    }
    mgr->disableDevice(SUNPOWER_NAME);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    sunpowerMT_t data{};
    bool ok = it->get_status(data);
    mgr->enableDevice(SUNPOWER_NAME);
    if (!ok) {
      cc->respond("failed to fetch status", uicsCMD_ERR_NOT_SUPPORTED);
      return;
    }
    std::ostringstream s;
    s << "========== STATUS DATA ==========\n"
      << "COOLER:           " << (data.cooler ? "ON" : "OFF") << "\n"
      << "POWER MEASURED:   " << data.power_measured << "\n"
      << "POWER CMD:        " << data.power_commanded << "\n"
      << "TARGET TEMP:      " << data.target_t << "\n"
      << "REJECT TEMP:      " << data.reject_t << "\n"
      << "COLDHEAD TEMP:    " << data.coldhead_t << "\n"
      << "=================================\n";
    cc->respond(s.str());
    return;
  }

  if ((*cc)["-vibe"].compare("") != 0) {
    auto it = mgr->getDevice<sunpowerMT>(SUNPOWER_NAME);
    if (!it) {
      cc->respond("failed to acquire resources", uicsCMD_ERR_PARAM_UNKNOWN);
      return;
    }
    mgr->disableDevice(SUNPOWER_NAME);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::string buff;
    bool ok = it->basic_cmd("vibe", buff);
    mgr->enableDevice(SUNPOWER_NAME);
    if (!ok) {
      cc->respond("failed to read vibration", uicsCMD_ERR_NOT_SUPPORTED);
    } else {
      cc->respond(buff);
    }
    return;
  }

  cc->respond("Undefined behavior", uicsCMD_ERR_PARAM_VALUE);
}
