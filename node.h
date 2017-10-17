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
  private:
    const string id = to_string(rand() % 100);
    string _address;
    int _port;
    string _partner_address;
    int _partner_port;
    Socket server_socket;
    bool _client_status;
    int key_space[2];
    map<size_t, string> storage;
    int num_clients = 0;
    map<const string, storage::peer_storage> request_list;
    storage::peer_list peers;
  public:
    Node(int port, string partner_address, int partner_port);
    ~Node();
    int run();
    void remote_node_controller(const string option, sockaddr_in client_sockaddr);
    static void* callPingFunction(void *arg) { return ((Node*)arg)->ping(); }
    static void* callServerFunction(void *arg) { return ((Node*)arg)->server(); }
    void add(void);
    void* ping(void);
    void* server(void);
    int disconnect();
    commands::ro<int> put_value(string);
    commands::ro<string> get_value(size_t);
    int* get_keyspace();
    int set_keyspace(int nodes, int key_lower, int key_higher);
    int client_send(string data, sockaddr_in client_sockaddr);
    commands::ro<string> put_request_list(string send_data, sockaddr_in client_sockaddr, string response);
    commands::ro<string> reshuffle();
    int status_socket();
    string current_time();
  };
} // node

#endif
