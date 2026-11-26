#include "vroomm4k_standard_cmd.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <labdev/lakeshore336.h>
#include <labdev/sunpowerMT.h>
#include <sstream>
#include <string>
#include <thread>
#include <uics/uics_config.h>
#include <uics/uics_err_code.h>
#include <uics/uics_main.h>
#include <uics/parse_number.h>
#include <uics/uics_number.h>

#define SUNPOWER_NAME "D_Sunpower"
#define LAKESHORE_NAME "D_Lakeshore"

static std::string to_upper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return s;
}

// cooler set <on|off> | target <K> | vibe <ON|OFF> [-feature <WAVE|HEAD>] | -status | -vibe
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

  // Temperature target: cooler target <K>
  if ((*cc)["target"].compare("") != 0) {
    double kelvin = 0;
    if (!parse_double((*cc)["target"], &kelvin) || kelvin <= 0) {
      cc->respond("target must be a temperature in kelvin",
                  uicsCMD_ERR_PARAM_VALUE);
      return;
    }
    auto it = mgr->getDevice<sunpowerMT>(SUNPOWER_NAME);
    if (!it) {
      cc->respond("failed to acquire resources", uicsCMD_ERR_PARAM_UNKNOWN);
      return;
    }
    mgr->disableDevice(SUNPOWER_NAME);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bool ok = it->setTargetTemperature(kelvin);
    mgr->enableDevice(SUNPOWER_NAME);
    if (!ok) {
      cc->respond("failed to set target temperature",
                  uicsCMD_ERR_NOT_SUPPORTED);
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
      << "ERRORS (binary):  " << data.error_binary << "\n"
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

// Heater range: names or the controller's own numbers. Outputs 1-2 have
// OFF/LOW/MEDIUM/HIGH (0-3), outputs 3-4 have OFF/ON (0-1).
static bool parse_range(const std::string &arg, int output, int &range) {
  std::string v = to_upper(arg);
  if (v == "OFF")
    range = 0;
  else if (v == "LOW" || (v == "ON" && output > 2))
    range = 1;
  else if (v == "MEDIUM" || v == "MED")
    range = 2;
  else if (v == "HIGH")
    range = 3;
  else if (!parse_int(v, &range))
    return false;
  return true;
}

static const char *range_name(int output, int range) {
  static const char *n12[] = {"OFF", "LOW", "MEDIUM", "HIGH"};
  static const char *n34[] = {"OFF", "ON"};
  if (output <= 2)
    return (range >= 0 && range <= 3) ? n12[range] : "?";
  return (range >= 0 && range <= 1) ? n34[range] : "?";
}

// lakeshore [output <1-4>] [range <r>] [rate <K/min>] [ramp <ON|OFF>]
//           [setpoint <val>] | -status
// Several settings may be combined; they are applied in the order range,
// ramp/rate, setpoint, and the first failure stops the command. Like the
// cooler, the device is disabled in the DeviceManager during the exchange.
void lakeshore(instHandle *handle, cmd *cc) {
  auto *mgr = handle->dataserver.devManager;

  bool do_status = (*cc)["-status"].compare("") != 0;
  bool do_range = (*cc)["range"].compare("") != 0;
  bool do_ramp = (*cc)["ramp"].compare("") != 0;
  bool do_rate = (*cc)["rate"].compare("") != 0;
  bool do_setp = (*cc)["setpoint"].compare("") != 0;

  if (!do_status && !do_range && !do_ramp && !do_rate && !do_setp) {
    cc->respond("Undefined behavior", uicsCMD_ERR_PARAM_VALUE);
    return;
  }

  // Validate everything before touching the instrument.
  int output = 0, range = 0;
  double rate = 0, setp = 0;
  bool ramp_on = false;
  if (!do_status) {
    if ((*cc)["output"].compare("") == 0 ||
        !parse_int((*cc)["output"], &output) || output < 1 || output > 4) {
      cc->respond("output must be 1 to 4", uicsCMD_ERR_PARAM_VALUE);
      return;
    }
    if ((do_ramp || do_rate) && output > 2) {
      cc->respond("ramp and rate only apply to outputs 1 and 2",
                  uicsCMD_ERR_PARAM_VALUE);
      return;
    }
    if (do_range) {
      int max = output <= 2 ? 3 : 1;
      if (!parse_range((*cc)["range"], output, range) || range < 0 ||
          range > max) {
        cc->respond(output <= 2 ? "range must be OFF, LOW, MEDIUM or HIGH"
                                : "range must be OFF or ON",
                    uicsCMD_ERR_PARAM_VALUE);
        return;
      }
    }
    if (do_ramp) {
      std::string r = to_upper((*cc)["ramp"]);
      if (r != "ON" && r != "OFF") {
        cc->respond("ramp must be ON or OFF", uicsCMD_ERR_PARAM_VALUE);
        return;
      }
      ramp_on = (r == "ON");
    }
    if (do_rate && (!parse_double((*cc)["rate"], &rate) || rate < 0.1 ||
                    rate > 100)) {
      cc->respond("rate must be 0.1 to 100 K/min", uicsCMD_ERR_PARAM_VALUE);
      return;
    }
    if (do_setp && !parse_double((*cc)["setpoint"], &setp)) {
      cc->respond("setpoint must be a number", uicsCMD_ERR_PARAM_VALUE);
      return;
    }
  }

  auto it = mgr->getDevice<lakeshore336>(LAKESHORE_NAME);
  if (!it) {
    cc->respond("failed to acquire resources", uicsCMD_ERR_PARAM_UNKNOWN);
    return;
  }
  mgr->disableDevice(LAKESHORE_NAME);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  if (do_status) {
    std::ostringstream s;
    for (int out = 1; out <= 2; out++) {
      double sp = out == 1 ? it->setp_1() : it->setp_2();
      bool on = false;
      double r = 0;
      int rg = -1;
      bool ok_ramp = it->getRamp(out, on, r);
      bool ok_range = it->getHeaterRange(out, rg);
      s << "OUTPUT " << out << "\n"
        << "  SETPOINT: " << sp << "\n";
      if (ok_ramp)
        s << "  RAMP:     " << (on ? "ON" : "OFF") << " (" << r << " K/min)\n";
      else
        s << "  RAMP:     unavailable\n";
      if (ok_range)
        s << "  RANGE:    " << range_name(out, rg) << "\n";
      else
        s << "  RANGE:    unavailable\n";
    }
    mgr->enableDevice(LAKESHORE_NAME);
    cc->respond(s.str());
    return;
  }

  std::string failed;
  if (do_range && !it->setHeaterRange(output, range))
    failed = "range";
  else if (do_ramp && do_rate && !it->setRamp(output, ramp_on, rate))
    failed = "ramp/rate";
  else if (do_ramp && !do_rate && !it->setRampEnabled(output, ramp_on))
    failed = "ramp";
  else if (do_rate && !do_ramp && !it->setRampRate(output, rate))
    failed = "rate";
  else if (do_setp && !it->setSetpoint(output, setp))
    failed = "setpoint";
  mgr->enableDevice(LAKESHORE_NAME);

  if (!failed.empty()) {
    cc->respond("failed to set " + failed, uicsCMD_ERR_NOT_SUPPORTED);
  } else {
    cc->respond();
  }
}
