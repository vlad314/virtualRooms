#include "virtualroomsrv_configreader.h"
#include "virtualroomsrv_error.h"

#include <boost/exception/all.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace virtualroom {
namespace virtualroomsrv {

Config ConfigReader::readerConfig(const std::string_view &file) {
  boost::property_tree::ptree pt;
  Config config;

  try {
    boost::property_tree::read_json(std::string{file}, pt);
  } catch (const boost::exception &e) {
    throw Error{boost::diagnostic_information(e)};
  }

  try {
    return Config{pt.get<std::string>("addr"), pt.get<unsigned short>("port"),
                  pt.get<std::string>("client_path")};
  } catch (const boost::property_tree::ptree_bad_data &e) {
    throw Error(boost::diagnostic_information(e));
  }
}

} // namespace virtualroomsrv
} // namespace virtualroom