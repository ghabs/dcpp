#ifndef NODE_H
#define NODE_H
#include <string>
#include <map>
#include "socket/socket.h"
#include "commands.h"

//TODO(): create struct for keyspace;
//TODO(): Define interface between nodes
//TODO(): Add more constants

using namespace std;
namespace node {
  class Node {
  public:
    Node(int port, string partner_address, int partner_port);
    ~Node();
    string get_address();
    int run();
    void remote_node_controller(const string option);
    int add(string address);
    int disconnect();
    int put_value(string);
    int get_value(int);
    int* get_keyspace();
    int set_keyspace(int nodes, int key_lower, int key_higher);
    string current_time();
    string print_partner();
  private:
    string _address;
    int _port;
    string _partner_address;
    int _partner_port;
    Socket server_socket;
    //Change to allow many sockets to be created, stored in array
    Socket accept_socket;
    Socket client_socket;
    bool _client_status;
    int key_space[2];
    map<int, string> storage;
  };
} // node

#endif
