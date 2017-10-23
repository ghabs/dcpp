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
  struct rn_struct {
    int err;
    string data;
    sockaddr_in peer_sockaddr;
  };
  class Node {
  private:
    const string id = to_string(rand() % 100);
    //CHORD: UUID
    int MSIZE = 64;
    //TODO change to c++ rand number
    int chord_id = rand() % MSIZE;
    storage::RoutingTable rt;
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
    //TOPOLOGy MAP
    storage::peer_list peers;
  public:
    Node(int port, string partner_address, int partner_port);
    ~Node();
    int run();
    node::rn_struct remote_node_controller(const string option, sockaddr_in client_sockaddr);
    static void* callPingFunction(void *arg) { return ((Node*)arg)->ping(); }
    static void* callServerFunction(void *arg) { return ((Node*)arg)->server(); }
    void* ping(void);
    void* server(void);
    int disconnect();
    int* get_keyspace();
    int hash_chord(string val){
      hash<string> hashf;
      size_t key;
      key = hashf(val);
      key = key % MSIZE;
      return key;
    }
    //TODO replace nearest neighbor and also template with successor
    commands::ro<int> query_chord(int k) {
        commands::ro<int> io;
        io.data = this->rt.find_successor(k);
        if (!io.data) {
          io.s = -1;
          io.data = rt.nearest_neighbor(k);
          return io;
        }
        io.s = 1;
        return io;
    };
    commands::ro<string> get_value(size_t);
    commands::ro<string> reshuffle_chord(int p){
      commands::ro<string> ro;
      for (auto& kv : storage) {
        bool member = rt.check_membership(p, chord_id, kv.first);
        if (!member){
          ro.data += (kv.second + ':');
        }
      }
      ro.s = 1;
      return ro;
    }
    void join_chord(void);
    int notify_check(int);
    int set_keyspace(int nodes, int key_lower, int key_higher);
    int client_send(string data, sockaddr_in client_sockaddr);
    int stabilize_chord();
    string print_chord_id() {
      return to_string(chord_id);
    }
    commands::ro<string> put_request_list(string send_data, sockaddr_in client_sockaddr, string response);
    commands::ro<string> reshuffle();
    int status_socket();
    string current_time();
    //CHORD: FIND NEAREST NODE ON RING
    int find_successor();
    string print_successor(){
      auto s = rt.get_successor();
      return to_string(s.id);
    }
    //TODO replace everywhere and also get from running server socket info instead of presets
    sockaddr_in get_server_info() {
      struct sockaddr_in server;
      server.sin_port = htons(_port);
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = inet_addr("127.0.0.1");
      return server;
    }
  };
} // node

#endif
