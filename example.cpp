#include "tiny_cmdline.h"

#include <cstdio>
#include <string>

struct ParsedArgs {
  std::string filename;
  std::string ip;
  int32_t port;
  int8_t val;
};

// convert char* to std::string should be specialized
template <> struct tiny_cmdline::TinyCmdline::convert<std::string> {
  static std::string to(const char *optarg) { return std::string(optarg); }
};

void get_version() { printf("1.0.0"); }
int32_t main(int32_t argc, char *argv[]) {
  using namespace tiny_cmdline;
  using argument = TinyCmdline::Argument;

  ParsedArgs args;
  TinyCmdline cmd;
  // just execute the function when the option is found
  cmd.add_argument("version", 'v', get_version, argument::none, "Prints the version information.");
  // load the argument value into the variable
  cmd.add_argument("file", 'f', args.filename, "The file to be loaded.");
  cmd.add_argument("ip", 'i', args.ip, "The IP address to connect to.");
  cmd.add_argument("port", 'p', args.port, "The port to connect to.");

  // output:
  // $ ./example
  //       -p, --port <arg> The port to connect to.
  //       -f, --file <arg> The file to be loaded.
  //       -i, --ip <arg> The IP address to connect to.
  //       -v, --version Prints the version information.

  // -h, --help are reserved for help, user can add customized help information
  //   const std::string help = "Prints the help information.";
  // bad execution, this can cause recursive call
  //   cmd.add_argument(
  //       "help", 'h', [&cmd]() { cmd.print_help(); }, help);
  // good execution
  //   cmd.add_argument(
  //       "help", 'h', [&cmd, &help]() { printf("%s\n", help.c_str()); }, help);
  // ./example --help:
  // Prints the help information.

  // user check the value themselves
  //   cmd.add_argument("val", 0, args.val, "The value to be set.");
  // set default value
  cmd.add_argument("default_val", 0, args.val, 0, 66, "The value to be set.");
  // check the range
  cmd.add_argument(
      "val", 0,
      [&args](const char *optarg) {
        const auto val = std::stoi(optarg);
        if (val < 0 || val > 100) {
          fprintf(stderr, "The value should be in the range [0, 100].\n");
          exit(1);
        } else {
          args.val = static_cast<int8_t>(val);
        }
      },
      argument::required, "The value to be set.");  // argument may be optional, so it need to be specified
  cmd.add_argument(
      "user_val", 0,
      [&]() {
        printf("Previous value is %d\n", args.val);
        printf("Please input the value again: ");
        scanf("%hhd", &args.val);
        printf("User defined value again.\n");
      },
      argument::none,
      "User defined value again.");  // argument must be none, so it not specified
  cmd.parse(argc, argv);

  // $ ./example -f README.md -i 127.0.0.1 -p 8080 --val 66 --user_val
  //         -p, --port <arg> The port to connect to.
  //         -f, --file <arg> The file to be loaded.
  //         -i, --ip <arg> The IP address to connect to.
  //         -v, --version Prints the version information.
  // >>>>>>>> print_help end
  // Previous value is 66
  // Please input the value again: 123
  // User defined value again.
  // filename: README.md
  // ip: 127.0.0.1
  // port: 8080
  // val: 123

  cmd.print_help();
  printf(">>>>>>>> print_help end\n");
  printf("filename: %s\n", args.filename.c_str());
  printf("ip: %s\n", args.ip.c_str());
  printf("port: %d\n", args.port);
  printf("val: %d\n", args.val);
}