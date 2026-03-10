#include <filesystem>
#include <iomanip>
#include <sstream>

#include "static_server.h"

int main(int argc, char* argv[]) {
  // Set temporary config
  // std::string ip{"0.0.0.0"};
  // int port{35565};
  // std::string base_dir{"."};
  //

  // StaticServer svr(ip, port, base_dir);
  StaticServer svr;
  svr.Run();
  return 0;
}
