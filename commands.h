#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>

using namespace std;
namespace commands {
  struct Commands {
    string option;
    string data[];
  };
} //namespace commands

#endif
