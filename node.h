#ifndef NODE_H
#define NODE_H
#include <string>
#include <map>

using namespace std;
namespace node {
  class Node {
  public:
    Node(string partner);
    ~Node();
    string get_address();
    int run(string port);
    int add(string address);
    int disconnect();
    int put_value(string);
    int get_value(int);
    string current_time();
    string print_partner();
  private:
    string address;
    string _partner;
    double key_space[2];
    std::map<int, string> storage;
  };
} // node

#endif
