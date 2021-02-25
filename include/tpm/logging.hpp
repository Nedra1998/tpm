#ifndef TPM_LOGGING_HPP_
#define TPM_LOGGING_HPP_

#include <memory>

#include <spdlog/common.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#define TPM_LOG(logger, level, file, line, function, ...)                   \
  tpm::logging::get(logger)->log(spdlog::source_loc{file, line, function},  \
                                    level, __VA_ARGS__)

#define TPM_LOG_TRACE(logger, ...)                                          \
  TPM_LOG(logger, spdlog::level::trace, __FILE__, __LINE__,                 \
             SPDLOG_FUNCTION, __VA_ARGS__)
#define TPM_LOG_DEBUG(logger, ...)                                          \
  TPM_LOG(logger, spdlog::level::debug, __FILE__, __LINE__,                 \
             SPDLOG_FUNCTION, __VA_ARGS__)
#define TPM_LOG_INFO(logger, ...)                                           \
  TPM_LOG(logger, spdlog::level::info, __FILE__, __LINE__, SPDLOG_FUNCTION, \
             __VA_ARGS__)
#define TPM_LOG_WARN(logger, ...)                                           \
  TPM_LOG(logger, spdlog::level::warn, __FILE__, __LINE__, SPDLOG_FUNCTION, \
             __VA_ARGS__)
#define TPM_LOG_ERR(logger, ...)                                            \
  TPM_LOG(logger, spdlog::level::err, __FILE__, __LINE__, SPDLOG_FUNCTION,  \
             __VA_ARGS__)
#define TPM_LOG_CRITICAL(logger, ...)                                       \
  TPM_LOG(logger, spdlog::level::critical, __FILE__, __LINE__,              \
             SPDLOG_FUNCTION, __VA_ARGS__)

#define LTRACE(...) TPM_LOG_TRACE(__VA_ARGS__)
#define LDEBUG(...) TPM_LOG_DEBUG(__VA_ARGS__)
#define LINFO(...) TPM_LOG_INFO(__VA_ARGS__)
#define LWARN(...) TPM_LOG_WARN(__VA_ARGS__)
#define LERR(...) TPM_LOG_ERR(__VA_ARGS__)
#define LCRITICAL(...) TPM_LOG_CRITICAL(__VA_ARGS__)

namespace tpm::logging {
extern std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink;
void configure_logging();

inline std::shared_ptr<spdlog::logger> get(const std::string &logger) {
  std::shared_ptr<spdlog::logger> spdlog_logger = spdlog::get(logger);
  if (!spdlog_logger) {
    if (dist_sink) {
      spdlog_logger = std::make_shared<spdlog::logger>(logger, dist_sink);
    } else {
      spdlog_logger = std::make_shared<spdlog::logger>(logger);
    }
    spdlog::register_logger(spdlog_logger);
    tpm::logging::configure_logging();
  }
  return spdlog_logger;
}
} // namespace tpm::logging

#endif /* end of include guard: TPM_LOGGING_HPP_ */
