#ifndef DevManagerCtrl_H
#define DevManagerCtrl_H
#include "insthandle.h"
#include <string>
#include <uics/uics_cmds.h>
#include <vector>
void dev_ctrl_status(instHandle *handle, cmd *cc);
void pressure_status(instHandle *handle, cmd *cc);

#endif
