#include "vroomm4k_getstatus.h"
#include <cstring>

void vroomm4k_status_t(instHandle *handle) {

  int fd = create_socket(7777);
  cmd *cc = new cmd;

  while (1) {
    cc->recvCMD(fd);
    static char *encodedStructure = NULL;
    vroomm4k_telemetry_t tlm{};



    // b64
    size_t length =
        islb64EncodeAlloc((const char *)&tlm, sizeof(tlm), &encodedStructure);
    cc->respond(std::to_string(length) + " " + std::string(encodedStructure));
  }
}
/*
  char observator[128];
  char operator_name[128];




 */
