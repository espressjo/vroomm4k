#include "vroomm4k_devices.h"
#include "vroomm4k_getstatus.h"
#include "devManagerCtrl.h"
#include "insthandle.h"
#include <uics.h>
#include <uics/uics_config.h>
#include <uics/uics_join.h>

using namespace std;
#define ROOT "/opt/vroomm4k"
Log HxRGlog;

int main(int argc, char *argv[]) {
  // we nee to override BASEPATH and CONFIGFILE


  std::string configfile = cfg_get_string(join(ROOT, "/config/vroomm4k.conf",'/'), "CONFIGPATH","");
  if (configfile.compare("")==0){return 1;}
  std::string logpath = cfg_get_string(configfile, "LOGPATH","/var/tmp/");
  // set the log path
  HxRGlog.setPath(logpath);
  HxRGlog.writeto("Startup");
  // initialize the handle
  instHandle handle = {}; // init to zero
  ArgParser parser(argc, argv);

  //:::::::::::::::::::::::::
  //:::   setup devices   :::
  //:::::::::::::::::::::::::

  setup_devices(&handle);

  //::::::::::::::::::::::::::::::::::::::::::::
  //:::   Start the UDP and TCP/IP handler   :::
  //::::::::::::::::::::::::::::::::::::::::::::
  uics UICS(&handle, configfile,
            configfile, false);

  //::::::::::::::::::::::::::::::::::::::
  //:::   Setup the forced transition  :::
  //::::::::::::::::::::::::::::::::::::::
  UICS.executeOnce(ACQ, IDLE);

  //::::::::::::::::::::::::::::
  //:::   Start the threads  :::
  //::::::::::::::::::::::::::::
  UICS.add_thread(vroomm4k_status_t, &handle, "b64vroomm4kstatus", false);

  //::::::::::::::::::::::::::::::::::::::
  //:::   Define all function to the   :::
  //:::   to the proper state          :::
  //::::::::::::::::::::::::::::::::::::::


 // UICS.add_function("cooler", cooler, CONFIG);
  UICS.add_function("devCtrl", dev_ctrl_status, CONFIG);

  UICS.run();
  return 0;
}
