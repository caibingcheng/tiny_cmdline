/*
  MIT License
*/

#ifndef TINY_CMDLINE_H
#define TINY_CMDLINE_H

#include <getopt.h>

#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tiny_cmdline {
class TinyCmdline {  // shortname 'h' and longname "help" are reserved for help
 public:
  enum class Argument {
    none = no_argument,
    required = required_argument,
    optional = optional_argument,
  };

 private:
  using operator_t = std::function<void(const char *)>;
  using void_operator_t = std::function<void()>;
  struct operator_option {
    char short_name;
    std::string long_name;
    operator_t op;  // operator function, takes the argument value as a parameter
    std::string help;
    Argument type;
  };

  static operator_t convert_operator_f(operator_t &&f) { return std::forward<operator_t>(f); }
  static operator_t convert_operator_f(void_operator_t &&f) {
    // the attribute  is supported since C++17, I wrote this for placeholder
    return [f]([[maybe_unused]] const char *) { f(); };
  }

 public:
  /**
   * Converts the argument value to the desired type. Specializations can be added for custom types.
   *
   * @tparam T The desired type.
   */
  template <typename T> struct convert {
    static T to(const char *optarg) { return static_cast<T>(std::stoll(optarg)); }
  };

  /**
   * Prints the help information.
   */
  void print_help() {
    for (const auto &option : operators_) {
      if (option.second.short_name == 'h' || option.second.long_name == "help") {
        option.second.op(nullptr);
        return;
      }
    }
    usage_();
  }

  /**
   * Parses the command line arguments.
   *
   * @param argc The number of command line arguments.
   * @param argv The command line arguments.
   */
  void parse(int argc, char *argv[]) {
    std::string short_options;
    std::vector<option> long_options;
    for (const auto &option_it : operators_) {
      const auto &option = option_it.second;
      if (option.short_name != '\0') {
        short_options = short_options + option.short_name + (option.type != Argument::none ? ":" : "");
      }
      if (!option.long_name.empty()) {
        long_options.push_back({option.long_name.c_str(), static_cast<int32_t>(option.type), nullptr, option_it.first});
      }
    }
    long_options.push_back({nullptr, 0, nullptr, 0});

    int32_t c = 0;
    int32_t option_index = 0;
    const int32_t opterr_tmp = opterr;
    opterr = 0;
    while ((c = getopt_long(argc, argv, short_options.c_str(), long_options.data(), &option_index)) != -1) {
      // if is a help command
      if (c == 'h' || std::string(argv[optind - 1]) == "--help" || std::string(argv[optind - 1]) == "-h") {
        print_help();
        exit(0);
      }
      if (c == '?') {
        print_help();
        exit(1);
      }
      // options are generated from operators_, so we can safely use the short name as the key
      operators_[c].op(optarg);
    }
    opterr = opterr_tmp;
  }

  /**
   * Adds an argument to the command line parser.
   *
   * @param long_name The long name of the argument.
   * @param short_name The short name of the argument.
   * @param f The operator function associated with the argument, which takes the argument value as a parameter.
   * @param type The type of the argument (default: Argument::Required).
   * @param help The help text for the argument (default: "").
   */
  template <typename T>
  void add_argument(const std::string &long_name, char short_name, T &&f, Argument type, const std::string &help = "") {
    using decay_f = typename std::decay<T>::type;
    constexpr bool is_operator_f = std::is_convertible<decay_f, operator_t>::value;
    constexpr bool is_void_operator_f = std::is_convertible<decay_f, void_operator_t>::value;
    static_assert(is_operator_f || is_void_operator_f, "The operator function must be operator_t or void_operator_t.");

    const auto opt_val = static_cast<int32_t>((short_name == '\0') ? opt_val_++ : short_name);
    auto operator_f = convert_operator_f(std::forward<T>(f));
    if (!operators_.emplace(opt_val, operator_option{short_name, long_name, operator_f, help, type}).second) {
      fprintf(stderr, "duplicate option -%c, --%s\n", short_name, long_name.c_str());
    }
  }

  /**
   * Adds an argument to the command line parser.
   *
   * @param long_name The long name of the argument.
   * @param short_name The short name of the argument.
   * @param value The value to be set by the argument.
   * @param help The help text for the argument (default: "").
   */
  template <typename T>
  void add_argument(const std::string &long_name, char short_name, T &value, const std::string &help = "") {
    auto operator_f = [&value](const char *optarg) { value = convert<T>::to(optarg); };
    add_argument(long_name, short_name, operator_f, Argument::required, help);
  }

  /**
   * Adds an argument to the command line parser.
   *
   * @param long_name The long name of the argument.
   * @param short_name The short name of the argument.
   * @param value The value to be set by the argument.
   * @param default_val The default value to be set if the argument is not present.
   * @param placed_val The value to be set if the argument is present.
   * @param help The help text for the argument (default: "").
   */
  template <typename T, typename U>
  void add_argument(const std::string &long_name, char short_name, T &value, const U &default_val, const U &placed_val,
                    const std::string &help = "") {
    value = static_cast<T>(default_val);
    auto operator_f = [&value, placed_val]([[maybe_unused]] const char *) { value = static_cast<T>(placed_val); };
    add_argument(long_name, short_name, operator_f, Argument::none, help);
  }

 private:
  /**
   * Prints the usage information, automatically generated from the added arguments.
   */
  void usage_() {
    for (const auto &option_it : operators_) {
      const auto &option = option_it.second;
      const char *arg_str = (option.type == Argument::required) ? " <arg> " : " ";
      if (option.short_name == '\0') {
        fprintf(stdout, "\t--%s%s%s\n", option.long_name.c_str(), arg_str, option.help.c_str());
      } else if (option.long_name.empty()) {
        fprintf(stdout, "\t-%c%s%s\n", option.short_name, arg_str, option.help.c_str());
      } else {
        fprintf(stdout, "\t-%c, --%s%s%s\n", option.short_name, option.long_name.c_str(), arg_str, option.help.c_str());
      }
    }
  }

 private:
  int32_t opt_val_{static_cast<int32_t>(256)};  // std::numeric_limits<uint8_t>::max() + 1
  std::unordered_map<int32_t, operator_option> operators_;
};

}  // namespace tiny_cmdline

#endif  // TINY_CMDLINE_H
