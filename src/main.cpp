#include <iostream>
#include <limits>
#include <ostream>

#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX
#include <io.h>
#include <windows.h>
#undef min
#undef max
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
#include <unistd.h>
#endif

#include <CL/sycl.hpp>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#define PL_IMPLEMENTATION 1
#include "exit_code.hpp"
#include "log.hpp"
#include "prof.hpp"
#include "render.hpp"
#include "scene.hpp"
#include "version.hpp"

int main(int argc, const char **argv) {
  PINIT("tpm");

  PBEGIN("Setup");
  PBEGIN("Argparse");
  cxxopts::Options options(
      "TPM", fmt::format("TPM v{}.{} - C++ Minimal Path Marcher",
                         tpm::version::major, tpm::version::minor));

  // clang-format off
  options.add_options()
    ("h,help", "Display usage information")
    ("v,verbose", "Enable verbose logging")
    ("V,version", "Display detailed version information")
    ("I,info", "Display detailed application information");

  options.add_options()
    ("scene", "Scene description file", cxxopts::value<std::string>());
  options.parse_positional({"scene"});
  // clang-format on

  tpm::ExitCode status = tpm::ExitCode::OK;
  auto result = options.parse(argc, argv);

  PVAR("help", result.count("help"));
  PVAR("verbose", result.count("verbose"));
  PVAR("version", result.count("version"));
  PVAR("info", result.count("info"));
  PEND("Argparse");

  if (result.count("help") != 0) {
    std::cout << options.help() << std::endl;
    status = tpm::ExitCode::EXIT_OK;
  } else if (result.count("version") != 0) {
    std::cout << tpm::version::semver << std::endl;
    status = tpm::ExitCode::EXIT_OK;
  }

  PBEGIN("Logging");
  try {
    spdlog::level::level_enum log_level = spdlog::level::warn;
    if (result.count("verbose") != 0) {
#ifdef NDEBUG
      log_level = spdlog::level::info;
#else
      log_level = spdlog::level::trace;
#endif
    }

    tpm::logging::dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
    {
#if defined(_WIN32) || defined(_WIN64)
      bool is_tty = _isatty(_fileno(stdout));
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
      bool is_tty = ::isatty(fileno(stdout)) != 0;
#else
      bool is_tty = false;
#endif
      if (is_tty) {
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sink->set_level(log_level);
        tpm::logging::dist_sink->add_sink(sink);
      } else {
        auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        sink->set_level(log_level);
        tpm::logging::dist_sink->add_sink(sink);
      }
    }
    {
      auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("tpm.log");
    }
  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    return -1;
  }
  PEND("Logging");
  PEND("Setup");

  tpm::TpmSpec tpm_spec;
  if (status == tpm::ExitCode::OK && result.count("scene") != 0) {
    std::tie(status, tpm_spec) =
        tpm::parse_spec(result["scene"].as<std::string>());
  } else if (result.count("scene") == 0) {
    LERR("Scene definition file is required!");
    status = tpm::ExitCode::ARGPARSE_MISSING_POSITIONAL;
  }

  /*   status = tpm::render_frame(); */

  if (status == tpm::ExitCode::OK)
    status = tpm::render_frame(std::move(tpm_spec));

  PSTOP();
  if (status == tpm::ExitCode::EXIT_OK)
    return 0;
  return status;
}
