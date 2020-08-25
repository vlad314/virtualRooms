#include "virtualroomsrv_App.h"
#include "virtualroomsrv_error.h"

#include <iostream>
#include <stdexcept>
#include <string_view>

// namespace virtualroom {
// namespace virtualroomsrv {

void usage(const std::string_view &argv0) {
  std::cerr << "Usage:\n  argv[0] = " << argv0
            << "\n  argv[1] = <config_file>\n";
}

int main(int argc, char **argv) {

  using namespace virtualroom::virtualroomsrv;

  for (int i = 0; i < argc; i++)
    std::cout << argv[i] << " ";
  std::cout << "\n";

  if (argc != 2) {
    usage(argv[0]);
    std::cerr << "argc: " << argc << "\n";
    return 1;
  }

  try {
    App app{argv[1]};
  } catch (const Error &e) {
    std::cerr << "Caught unhandled virtualrooms::Error: " << e.what() << "\n";
    return 2;
  } catch (const std::exception &e) {
    std::cerr << "Caught unhandled std::exception: " << e.what() << "\n";
    return 3;
  }

  return 0;
}
//} // namespace virtualroomsrv
//} // namespace virtualroom
