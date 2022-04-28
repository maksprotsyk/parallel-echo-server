// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com

#include "server.h"

#define SERVER_PORT 5055
#define THR_NUM 128

int main(int argc, char *argv[]) {
  server s(SERVER_PORT, THR_NUM);

  int option;
  while ((option = getopt(argc, argv, "m:")) != -1) {
    switch (option) {
    case 'm': {
      std::string mode{optarg};
      if (mode == "block") {
        s.run_t();
      } else if (mode == "nonblock") {
        s.run_n();
      }

      break;
    }
    case '?':
      throw std::runtime_error("Unknown option specified");
    default:
      abort();
    }
  }
  return 0;
}
