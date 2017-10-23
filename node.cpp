/**************************************************************************
*	Node class that holds and runs/returns properties around
**************************************************************************/
#include <ctime>
#include <iostream>
#include <string>
#include <exception>
#include "node.h"
#include <pthread.h>
#include <poll.h>
#include <cstring>
#include <sys/socket.h>
#include <fcntl.h>
#include <typeinfo>
#include <unordered_map>

#define NUM_THREADS 5

#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 100

using namespace std;

namespace node {
  Node::Node(int port=4950, string partner_address = "127.0.0.1", int partner_port=3002)
  : _partner_address(partner_address), _partner_port(partner_port), _port(port), rt(chord_id) {

    server_socket.create();
    rt.update_successor(chord_id, get_server_info());
    rt.update_predecessor(chord_id, get_server_info());
    //TODO(goldhaber): throw exception
    if (!server_socket.bind(port))
    {
      cout << "Could not bind to port." << '\n';
    }
    else {
      cout << "Node has bound to port " << port << '\n';
    }
  }
  Node::~Node() {
    //call disconnect function

    //reallocate storage
  }

  int Node::run(){
    pthread_t threads[ NUM_THREADS ];
    pthread_create(&threads[0], NULL, Node::callPingFunction, this);
    pthread_detach(threads[0]);
    pthread_create(&threads[1], NULL, Node::callServerFunction, this);
    pthread_join(threads[1], NULL);
    return 0;
  }

  //TODO(): turn into struct of options
  node::rn_struct Node::remote_node_controller(const string option, sockaddr_in client_sockaddr) {
    //handle incoming connection and parsed data
    //call other internal functions
    //send to other functions
    commands::Commands comm(option);
    commands::Commands sd(to_string(chord_id));
    node::rn_struct rn;
    cout << "option " << comm.option << '\n';
    string send_data = id + ':';
    sd.ori = "NAN";
    sd.reqres = "REQ";
    string ip_addr = inet_ntoa(client_sockaddr.sin_addr);
    //TODO replace with hash table
    if (comm.option == "JOIN") {
      if (comm.reqres == "ACK") {
        request_list.erase(ip_addr);
      }
    }
    else if (comm.option == "HANDSHAKE_CHORD") {
      //TODO send reshuffled data
      request_list.erase(ip_addr);
      auto sid = this->query_chord(stoi(comm.data[0]));
      if (!sid.s){
        //TODO REDIRECT TO NEIGHBOR, HAVE THEM SEARCH
      }
      if (sid.data == chord_id) {
        sd.option = "SET_SUCCESSOR";
        sd.reqres = "RES";

        sd.data.push_back(to_string(chord_id));
        sd.data.push_back(to_string(_port));
        rn.data = sd.to_string();
        if (comm.ori == "NAN") {
          rt.update_predecessor(stoi(comm.data[0]), client_sockaddr);
          rn.peer_sockaddr = client_sockaddr;
        }
        else {
          struct sockaddr_in server;
          server.sin_port = htons(stoi(comm.ori));
          server.sin_family = AF_INET;
          server.sin_addr.s_addr = inet_addr("127.0.0.1");
          rt.update_predecessor(stoi(comm.data[0]), server);
          rn.peer_sockaddr = server;
        }
        rn.err = 1;
        return rn;
      }
      auto s = rt.get_successor();
      sd.ori = comm.ori;
      sd.option = "SET_SUCCESSOR";
      sd.reqres = "RES";
      sd.data.push_back(to_string(chord_id));
      sd.data.push_back(to_string(_port));
      rn.data = sd.to_string();
      rn.peer_sockaddr = s.peer_sockaddr;
      rn.err = 1;
      //check if need to use ori
      rt.update_successor(stoi(comm.data[0]), client_sockaddr);
      return rn;
    }
    else if (comm.option == "STABILIZE_CHORD") {
      if (comm.reqres == "REQ") {
        sd.option = "STABILIZE_CHORD";
        sd.reqres = "RES";
        auto p = rt.get_predecessor();
        sd.data.push_back(to_string(p.id));
        sd.data.push_back(to_string(ntohs(p.peer_sockaddr.sin_port)));
        rn.data = sd.to_string();
        rn.peer_sockaddr = client_sockaddr;
        rn.err = 1;
        return rn;
      }
      if (comm.reqres == "RES") {
        struct sockaddr_in server;
        server.sin_port = htons(stoi(comm.data[1]));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (rt.stabilize_check(stoi(comm.data[0]), server)){
          sd.option = "NOTIFY_CHORD";
          sd.reqres = "REQ";
          rn.data = sd.to_string();
          rn.peer_sockaddr = server;
          rn.err = -1;
          return rn;
        }
        //Notify successors predecessor

      }
    }
    else if (comm.option == "SET_SUCCESSOR") {
      request_list.erase(ip_addr);
      struct sockaddr_in server;
      server.sin_port = htons(stoi(comm.data[1]));
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = inet_addr("127.0.0.1");
      rt.update_successor(stoi(comm.data[0]), server);
      //Notify successors predecessor
      sd.option = "NOTIFY_CHORD";
      sd.reqres = "REQ";
      rn.data = sd.to_string();
      rn.peer_sockaddr = server;
      rn.err = 1;
      return rn;
    }
    else if (comm.option == "NOTIFY_CHORD") {
      if (rt.notify_check(stoi(comm.id))){
        cout << "notify check pass" << '\n';
        rt.update_predecessor(stoi(comm.id), client_sockaddr);
      }
      commands::ro<string> rd = this->reshuffle_chord(stoi(comm.id));
      if (rd.data != "") {
        sd.option = "PUT_CHORD";
        sd.reqres = "REQ";
        sd.data.push_back(rd.data);
        rn.data = sd.to_string();
        rn.peer_sockaddr = client_sockaddr;
        rn.err = 1;
        return rn;
      }
      rn.data = "notify chord test";
      rn.err = -1;
      return rn;
    }
    else if (comm.option == "QUERY_CHORD_PUT") {
      int key = this->hash_chord(comm.data[0]);
      cout << key << '\n';
      auto sid = this->query_chord(key);
      cout << sid.data << '\n';
      if (!sid.s) {
        //Send to nearest neighbor with query_chord command
      }
      if (sid.data == chord_id) {
        storage[key] = comm.data[0];
        sd.ori = to_string(ntohs(client_sockaddr.sin_port));
        sd.option = "PUT_CHORD";
        sd.reqres = "RES";
        sd.data.push_back(to_string(key));
        rn.data = sd.to_string();
        rn.peer_sockaddr = client_sockaddr;
        //TODO change err system
        rn.err = 1;
        return rn;
      }
      //Send to successor with PUT command
      auto s = rt.get_successor();
      sd.ori = to_string(ntohs(client_sockaddr.sin_port));
      sd.option = "PUT_CHORD";
      sd.reqres = "REQ";
      sd.data.push_back(comm.data[0]);
      rn.data = sd.to_string();
      rn.peer_sockaddr = s.peer_sockaddr;
      //TODO change err system
      rn.err = 1;
      return rn;
    }
    else if (comm.option == "QUERY_CHORD_GET"){
      if (comm.reqres == "RES") {
        cout << "Got VALUE Back" << comm.to_string() << endl;
        rn.err = -1;
        return rn;
      }
      int key = stoi(comm.data[0]);
      auto sid = this->query_chord(key);
      if (!sid.s){
        //FORWARD TO NEIGHBOR NODE
        //sid.data has neighbor node to check
      }
      sd.ori = to_string(ntohs(client_sockaddr.sin_port));
      sd.option = "QUERY_CHORD_GET";
      if (sid.data == chord_id) {
        string value = storage[key];
        sd.reqres = "RES";
        sd.data.push_back(value);
        rn.data = sd.to_string();
        rn.peer_sockaddr = client_sockaddr;
        //TODO change err system
        rn.err = 1;
        return rn;
      }
      auto s = rt.get_successor();
      sd.reqres = "REQ";
      sd.data.push_back(comm.data[0]);
      rn.data = sd.to_string();
      rn.peer_sockaddr = s.peer_sockaddr;
      rn.err = 1;
      return rn;
    }
    else if (comm.option == "PUT_CHORD") {
      if (comm.reqres == "RES") {
        cout << "Got Key Back" << comm.to_string() << endl;
        rn.err = -1;
        return rn;
      }
      int key = this->hash_chord(comm.data[0]);
      sd.ori = to_string(ntohs(client_sockaddr.sin_port));
      sd.option = "PUT_CHORD";
      sd.reqres = "RES";
      sd.data.push_back(to_string(key));
      rn.data = sd.to_string();
      //TODO: change it to go to original client
      rn.peer_sockaddr = client_sockaddr;
      //TODO change err system
      rn.err = 1;
      return rn;
    }
    else if (comm.option == "GET_STATS") {
      sd.data.push_back("Chord Id: " + this->print_chord_id());
      sd.data.push_back("Successor: " + this->print_successor());
      rn.data = sd.to_string();
      rn.peer_sockaddr = client_sockaddr;
      rn.err = 1;
      return rn;
    }
    else {
      cout << "Unknown Command" << '\n';
    }
    //TODO():remove when all updated;
    rn.err = -1;
    return rn;
  }

  void Node::join_chord(void){
    struct sockaddr_in server;
    server.sin_port = htons(_partner_port);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    string send_data = to_string(chord_id) + ':';
    send_data += "NAN:HANDSHAKE_CHORD:REQ:";
    send_data += to_string(chord_id);
    int csstatus = client_send(send_data, server);
    if(csstatus){
      this->put_request_list(send_data, server, "JOIN");
    }
  }
  int Node::stabilize_chord(){
    string sd = print_chord_id() + ':';
    auto s = rt.get_successor();
    auto p = rt.get_predecessor();
    cout << "stabilize" << endl;
    if (s.id != chord_id) {
      sd += "NAN:STABILIZE_CHORD:REQ:";
      client_send(sd, s.peer_sockaddr);
      return 1;
    }
    if ((s.id == chord_id) && (p.id != chord_id)){
      rt.stabilize_check(p.id, p.peer_sockaddr);
    }
  }

  void* Node::ping(void){
    int stabilize = 0;
    while (true) {
      sleep(5);
      std::map<string,storage::peer_storage>::iterator it = request_list.begin();
        while (it != request_list.end()) {
          cout << "ping\n";
          auto ps = it->second;
          client_send(ps.data_sent, ps.peer_sockaddr);
          cout << it->first << " pinged." << '\n';
          request_list[it->first].ping_count++;
          if (ps.ping_count > 2){
            it = request_list.erase(it);
          } else {
            ++it;
          }
        }
        ++stabilize;
        if (stabilize == 3) {
          stabilize_chord();
          stabilize = 0;
        }
      }
    return NULL;
  }

  int Node::client_send(string data, sockaddr_in client_sockaddr){
    int numbytes;
    if ((numbytes = sendto(server_socket.get_sock_descriptor(), data.data(), data.size(), 0,
    (sockaddr *) &client_sockaddr, sizeof(client_sockaddr))) == -1) {
      perror("talker: sendto");
      return -1;
    }
    cout << "client_sendmodule sent " << data << " to " << ntohs(client_sockaddr.sin_port) << '\n';
    return 1;
  }

  commands::ro<string> Node::put_request_list(string send_data,
    sockaddr_in client_sockaddr,
    string response) {
      commands::ro<string> ro;
      storage::peer_storage ps;
      ps.peer_sockaddr = client_sockaddr;
      ps.data_sent = send_data;
      ps.response_needed = response;
      //TODO(add unique identifier to message)
      string ip_addr = inet_ntoa(client_sockaddr.sin_addr);
      request_list[ip_addr] = ps;
      ro.s = 1;
      return ro;
    }

    void* Node::server(void) {
      char buf [ 1024 ];
      struct sockaddr_in their_addr;
      socklen_t addr_len;
      string data = "";
      bool run_server = true;
      this->join_chord();
      while(run_server) {
        sleep(1);
        cout << "running server" << '\n';
        ::recvfrom(server_socket.get_sock_descriptor(), buf, 1024 -1 , 0,
        (struct sockaddr *) &their_addr, &addr_len);
        //PEER COMMANDS
        //TODO change to commands.h
        if (strcmp(buf, "close") == 0){
          cout << "shutting down" << '\n';
          run_server = false;
        }
        else {
          //TODO replace and standardize around string or char
          data = buf;
          cout << "data received from peer " << data << '\n';
          auto rn = this->remote_node_controller(data, their_addr);
          if (rn.err > 0){
            this->client_send(rn.data, rn.peer_sockaddr);
          }
        }
      }
      return NULL;
    }

    int Node::disconnect(){
      // call partner node send disconnect message
      // Send disconnect call to partner node
      // TODO: store in internal state whether or not in disconnect mode
      // TODO: Build an internal state object
      // Once keyspace has been updated with lower & higher, run disconnect to update
      return -1;
    }


    commands::ro<string> Node::get_value(size_t key){
      commands::ro<string> ro;
      map<size_t, string>::iterator iter = storage.find(key);
      if ( storage.end() != iter ) {
        ro.data = storage[key];
        ro.s = 1;
        return ro;
      }
      ro.s = -1;
      return ro;
    }

    commands::ro<string> Node::reshuffle() {
      //Go through internal storage
      //Move everything that is no longer in this node over to the new node
      commands::ro<string> ro;
      commands::ro<string> resp;
      string sd = "";
      for (auto& kv : storage) {
        if (kv.first < key_space[0] || kv.first > key_space[1]) {
          //Change to not a pointer?
          cout << kv.first << '\n';
          resp = this->get_value(kv.first);
          if (resp.s) {
            sd += resp.data;
          }
        }
        //Decide if going to erase from memory
      }
      cout << "reshuffle " << sd << '\n';
      if (sd == ""){
        ro.s = -1;
        return ro;
      }
      ro.data = resp.data;
      ro.s = 1;
      return ro;
    }

    int* Node::get_keyspace(){
      return key_space;
    }

    string Node::current_time(){
      time_t now = std::time(0);
      return ctime(&now);
    }

  } // node
