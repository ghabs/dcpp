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
    Commands(string message = "") { parse(message);}
    int parse(string message) {
      istringstream split(message);
      string item;
      int i = 0;
      for (string tkn; getline(split, tkn, ':'); i++) {
        switch (i) {
          case 0: id = tkn; break;
          case 1: ip = tkn; break;
          //TODO change to port
          case 2: ori = tkn; break;
          case 3: option = tkn; break;
          //TODO change to rs
          case 4: reqres = tkn; break;
          default: data.push_back(tkn); break;
        }
      };
      return 1;
    }
    //TODO add timestamp
    //TODO verify string values in this class
    string id;
    string ip;
    string ori;
    string option;
    string reqres;
    vector<string> data;
    string to_string() {
      string sd = id + ':' + ip + ':' + ori
      + ':' + option + ':' + reqres;
      for (string v: data){
        sd += (':' + v);
      }
      return sd;
    };
  };
  template<typename a>
  struct ro {
    int s;
    a data;
  };
} //namespace commands

#endif
