#ifndef LOG_HPP_TA9XZWG1
#define LOG_HPP_TA9XZWG1

#undef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#define SPDLOG_FMT_EXTERNAL

#include <memory>
#include <spdlog/common.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/spdlog.h>

#define TPM_LOG(logger, level, ...)                                            \
  tpm::logging::get(logger)->log(                                              \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level,          \
      __VA_ARGS__)

#define LTRACE(...) TPM_LOG("TPM", spdlog::level::trace, __VA_ARGS__)
#define LDEBUG(...) TPM_LOG("TPM", spdlog::level::debug, __VA_ARGS__)
#define LINFO(...) TPM_LOG("TPM", spdlog::level::info, __VA_ARGS__)
#define LWARN(...) TPM_LOG("TPM", spdlog::level::warn, __VA_ARGS__)
#define LERR(...) TPM_LOG("TPM", spdlog::level::err, __VA_ARGS__)
#define LCRITICAL(...) TPM_LOG("TPM", spdlog::level::critical, __VA_ARGS__)

#define LOG_TRACE(logger, ...)                                                 \
  TPM_LOG(logger, spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(logger, ...)                                                 \
  TPM_LOG(logger, spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(logger, ...) TPM_LOG(logger, spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(logger, ...) TPM_LOG(logger, spdlog::level::warn, __VA_ARGS__)
#define LOG_ERR(logger, ...) TPM_LOG(logger, spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(logger, ...)                                              \
  TPM_LOG(logger, spdlog::level::critical, __VA_ARGS__)

namespace tpm::logging {
extern std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink;
void configure_logging();

inline std::shared_ptr<spdlog::logger> get(const std::string &logger = "TPM") {
  std::shared_ptr<spdlog::logger> spdlog_logger = spdlog::get(logger);
  if (!spdlog_logger) {
    if (dist_sink) {
      spdlog_logger = std::make_shared<spdlog::logger>(logger, dist_sink);
    } else {
      spdlog_logger = std::make_shared<spdlog::logger>(logger);
    }
    spdlog_logger->set_level(spdlog::level::level_enum::trace);
    spdlog::register_logger(spdlog_logger);
    tpm::logging::configure_logging();
  }
  return spdlog_logger;
}

} // namespace tpm::logging

#endif /* end of include guard: LOG_HPP_TA9XZWG1 */
