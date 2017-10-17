#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include <vector>
#include <sstream>

using namespace std;
namespace commands {
  class Commands {
  public:
    //TODO(): ADD Validation
    Commands(string message) { parse(message);}
    int parse(string message) {
      istringstream split(message);
      string item;
      int i = 0;
      for (string each; getline(split, each, ':'); i++){
        if (i == 0) {
          id = each;
        }
        else if (i == 1) {
          ori = each;
        }
        else if (i == 2) {
          option = each;
        } else if (i == 3) {
          reqres = each;
        }
        else {
          data.push_back(each);
        }
      };
      return 1;
    }
  string id;
  string ori;
  string option;
  string reqres;
  vector<string> data;
  };
  template<typename a>
  struct ro {
    int s;
    a data;
  };
} //namespace commands

#endif
