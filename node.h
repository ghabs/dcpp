#ifndef NODE_H
#define NODE_H
#include <string>
#include <map>
#include <vector>
#include "socket/socket.h"
#include "storage/storage.h"
#include "commands.h"
#include "storage/storage.h"
#include <stdlib.h>

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
    void remote_node_controller(const string option, sockaddr_in client_sockaddr);
    static void* callAddFunction(void *arg) { return ((Node*)arg)->add(); }
    static void* callPingFunction(void *arg) { return ((Node*)arg)->ping(); }
    static void* callServerFunction(void *arg) { return ((Node*)arg)->server(); }
    void* add(void);
    void* ping(void);
    void* server(void);
    int disconnect();
    size_t put_value(string);
    int get_value(size_t, string *);
    int* get_keyspace();
    int set_keyspace(int nodes, int key_lower, int key_higher);
    int clientsend(string data, sockaddr_in client_sockaddr);
    string current_time();
    string print_partner();
  private:
    const string id = to_string(rand() % 100);
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
    map<size_t, string> storage;
    int num_clients = 0;
    map<const string, storage::peer_storage> request_list;
    map<char*, sockaddr_in> client_storage;
  };
} // node

#endif
