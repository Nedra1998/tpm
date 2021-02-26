#ifndef TPM_CLI_VALIDATORS_HPP_
#define TPM_CLI_VALIDATORS_HPP_

#include <CLI/Validators.hpp>

#include <regex>

namespace CLI {
/// Check for an existing path
class RegexValidator : public Validator {
public:
  RegexValidator(std::string pattern,
                 std::regex_constants::match_flag_type flags =
                     std::regex_constants::match_default)
      : Validator() {
    std::stringstream out;
    out << " regex matches /" << pattern << "/";
    description(out.str());
    std::regex re(pattern);
    func_ = [re, pattern, flags](std::string &arg) -> std::string {
      if (!std::regex_match(arg, re, flags)) {
        return "Argument does not match regex /" + pattern + "/: " + arg;
      }
      return std::string();
    };
  }
};
} // namespace CLI

#endif /* end of include guard: TPM_CLI_VALIDATORS_HPP_ */
