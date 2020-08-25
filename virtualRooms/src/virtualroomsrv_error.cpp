#include "virtualroomsrv_error.h"

namespace virtualroom {
namespace virtualroomsrv {

Error::Error(const std::string &msg) : std::runtime_error(msg) {}

} // namespace virtualroomsrv
} // namespace virtualroom