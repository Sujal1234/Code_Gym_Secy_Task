#ifndef util_h
#define util_h

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <string>
#include <optional>

namespace bp = boost::process;
namespace asio = boost::asio;

inline constexpr int responseTimeLimit = 1; //seconds
void buildCpp(std::string code_path, std::string out_name);
std::optional<std::string> readPipeDeadline(bp::async_pipe &readPipe, asio::io_context &ctx,
                      boost::posix_time::seconds deadline);
#endif //util_h