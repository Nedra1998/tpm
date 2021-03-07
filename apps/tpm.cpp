#include <iostream>
#include <map>
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <windows.h>
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
#include <unistd.h>
#endif

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Validators.hpp>
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <spdlog/common.h>
#include <tpm/linalg.hpp>
#include <tpm/logging.hpp>
#include <tpm/tpm.hpp>
#include <tpm/version.hpp>

#include "cli_validators.hpp"

enum TermColor { AUTO, ALWAYS, NEVER };
static const std::map<std::string, TermColor> term_color_map{
    {"auto", AUTO}, {"always", ALWAYS}, {"never", NEVER}};
static const std::map<std::string, spdlog::level::level_enum> log_level_map{
    {"trace", spdlog::level::trace}, {"debug", spdlog::level::debug},
    {"info", spdlog::level::info},   {"warn", spdlog::level::warn},
    {"err", spdlog::level::err},     {"critical", spdlog::level::critical},
    {"off", spdlog::level::off}};

int main(int argc, char **argv) {
  CLI::App app{fmt::format("TPM v{} - Tiny Path Marcher", tpm::version::core)};

  bool version_message;
  app.add_flag("-V,--version", version_message,
               "Print the version information and exit");

  TermColor term_color = TermColor::AUTO;
  spdlog::level::level_enum log_level = spdlog::level::level_enum::err;
  std::string log_file = "logs/tpm.log";
  std::size_t log_file_size = 5e6;

  app.add_flag("-c{always},--color{always},-C{never},--no-color{never}",
               term_color, "Configure terminal logging coloring")
      ->default_str("auto")
      ->transform(CLI::CheckedTransformer(term_color_map, CLI::ignore_case))
      ->group("Logging");
  app.add_flag("-v{debug},--verbose{debug},-q{off},--quiet{off}", log_level,
               "Configure logging verbosity")
      ->default_str("err")
      ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case))
      ->group("Logging");
  app.add_option("--log-file", log_file, "Set logfile output path")
      ->default_str("tpm.log")
      ->group("Logging");
  app.add_option("--log-file-size", log_file_size, "Set log file maximum size")
      ->default_str("5mb")
      ->transform(CLI::AsSizeValue(true))
      ->group("Logging");

  std::string output = "output/{:05d}.png";

  app.add_option("-o,--output", output, "Set output file path", true)
      ->check(CLI::RegexValidator(
          ".*\\.((bmp)|(hdr)|(jpeg|jpg)|(png)|(tga))"))
      ->group("Output");

  try {
    app.parse(argc, argv);
  } catch (CLI::ParseError &err) {
    return app.exit(err);
  }

#if defined(_WIN32) || defined(_WIN64)
  bool isatty = _isatty(_fileno(stdout));
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
  bool isatty = ::isatty(fileno(stdout)) != 0;
#else
  bool isatty = false;
#endif

  if (version_message) {
    fmt::print("TPM v{}\n", tpm::version::semver);
    return 0;
  }

  try {
    std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink =
        std::make_shared<spdlog::sinks::dist_sink_mt>();
    if (term_color == ALWAYS || (term_color == AUTO && isatty)) {
      auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      sink->set_level(log_level);
      dist_sink->add_sink(sink);
    } else {
      auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
      sink->set_level(log_level);
      dist_sink->add_sink(sink);
    }

    {
      auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
          log_file, log_file_size, 5);
#ifdef NDEBUG
      sink->set_level(spdlog::level::info);
#else
      sink->set_level(spdlog::level::trace);
      dist_sink->add_sink(sink);
#endif
    }
    tpm::logging::dist_sink = dist_sink;
  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    return -1;
  }

  tpm::Scene scene{tpm::Camera{90.0f, tpm::fVector3(0.0, 0.0, 0.0),
                               tpm::fVector3(0.0, 0.0, 10.0),
                               tpm::fVector3(0.0, 1.0, 0.0)},
                   tpm::Film{output, {50, 50}},
                   {0.0, 0.0},
                   60.0};

  tpm::render(scene);

  return 0;
}
