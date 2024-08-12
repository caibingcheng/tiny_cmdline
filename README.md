### tiny_cmdline

A tiny command line library for linux, written in C++11, based on `getopt_long`. It is very simple, less than 200 lines of code, users can easily costumize it.

When I want a command line library, I found that there are many choices, but they are too heavy for me. So I wrote this tiny command line library for myself.

The library was written in C++11, as I found many teams are still only supporting old compilers, so it's not a good idea to support C++ 14 or later.

### How to use

1. Include the header file `tiny_cmdline.h` in your project.
2. Use the namespace `tiny_cmdline`.
3. Create a `TinyCmdline` object.
4. Add arguments with `add_argument` method.
5. Parse the command line with `parse` method.

```cpp
#include "tiny_cmdline.h"

struct ParsedArgs {
  std::string filename;
  std::string ip;
  int32_t port;
};

// convert char* to std::string should be specialized
template <> struct tiny_cmdline::TinyCmdline::convert<std::string> {
  static std::string to(const char *optarg) { return std::string(optarg); }
};

int main(int argc, char* argv[]) {
    using namespace tiny_cmdline;

    ParsedArgs args;
    TinyCmdline cmd;
    cmd.add_argument("file", 'f', args.filename, "The file to be loaded.");
    cmd.add_argument("ip", 'i', args.ip, "The IP address to connect to.");
    cmd.add_argument("port", 'p', args.port, "The port to connect to.");
    cmd.parse(argc, argv);
}
```
