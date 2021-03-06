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

using namespace std;

namespace node {
  Node::Node(int port=4950, string partner_address = "127.0.0.1", int partner_port=3002, int chord_id=1)
  : _partner_address(partner_address), _partner_port(partner_port), _port(port),
   chord_id(chord_id), rt(chord_id, M) {

    server_socket.create();
    auto serverinfo = get_server_info();
    rt.update_successor(chord_id, serverinfo);
    rt.update_predecessor(chord_id, serverinfo);
    rt.update_self(chord_id, serverinfo);
    //TODO(goldhaber): throw exception
    if (!server_socket.bind(port))
    {
      cout << "Could not bind to port." << '\n';
    }
    else {
      sockaddr_in socket_info = server_socket.get_socket_info();
      _address = to_string(socket_info.sin_addr.s_addr);
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
    commands::Commands comm(option);
    commands::Commands sd(to_string(chord_id));
    node::rn_struct rn;
    sockaddr_in socket_info = server_socket.get_socket_info();
    sd.ip = _address;
    sd.ori = to_string(socket_info.sin_port);
    sd.reqres = "REQ";
    if (comm.option == "HANDSHAKE_CHORD") {
      //TODO send reshuffled data
      auto sid = this->query_chord(stoi(comm.data[0]));
      if (!sid.s){
        //TODO check how to set this up
        cout << "!sidhandshake" << endl;
        sd.data.push_back(comm.data[0]);
        sd.ori = comm.ori;
        sd.option = "HANDSHAKE_CHORD";
        sd.reqres = "REQ";
        rn.data = sd.to_string();
        rn.peer_sockaddr = sid.data.peer_sockaddr;
        rn.err = 1;
        return rn;
      }
      struct sockaddr_in server;
      server.sin_port = htons(stoi(comm.ori));
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = inet_addr(comm.ip.c_str());
      sd.data.push_back(to_string(chord_id));
      sd.data.push_back(to_string(_port));
      sd.data.push_back(_address);
      sd.option = "SET_SUCCESSOR";
      sd.reqres = "RES";
      if (sid.data.id == chord_id) {
        rn.data = sd.to_string();
        rt.update_predecessor(stoi(comm.data[0]), server);
        rn.peer_sockaddr = server;
        rn.err = 1;
        return rn;
      }
      //TODO should be this nodes successor
      sd.ori = comm.ori;
      sd.ip = comm.ip;
      rn.data = sd.to_string();
      rn.peer_sockaddr = sid.data.peer_sockaddr;
      rn.err = 1;
      rt.update_successor(stoi(comm.data[0]), server);
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
        auto s = rt.get_successor();
        struct sockaddr_in server;
        server.sin_port = htons(stoi(comm.data[1]));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (rt.stabilize_check(stoi(comm.data[0]), server)) {
          auto s = rt.get_successor();
          sd.option = "NOTIFY_CHORD";
          sd.reqres = "REQ";
          rn.data = sd.to_string();
          rn.peer_sockaddr = s.peer_sockaddr;
          rn.err = 1;
          return rn;
        }
        //Notify successors predecessor
        sd.option = "NOTIFY_CHORD";
        sd.reqres = "REQ";
        rn.data = sd.to_string();
        rn.peer_sockaddr = s.peer_sockaddr;
        rn.err = 1;
        return rn;
      }
    }
    else if (comm.option == "SET_SUCCESSOR") {
      cout << "set" << endl;
      struct sockaddr_in server;
      server.sin_port = htons(stoi(comm.data[1]));
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = inet_addr("127.0.0.1");
      rt.update_successor(stoi(comm.data[0]), server);
      reqtab.remove_request(chord_id, storage::Requests::handshake);

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
      auto sid = this->query_chord(key);
      if (!sid.s) {
        //Send to nearest neighbor with query_chord command
        sd.data.push_back(comm.data[0]);
        sd.ip = comm.ip;
        sd.ori = comm.ori;
        sd.option = "QUERY_CHORD_PUT";
        sd.reqres = "REQ";
        rn.data = sd.to_string();
        rn.peer_sockaddr = sid.data.peer_sockaddr;
        rn.err = 1;
        return rn;
      }
      if (sid.data.id == chord_id) {
        storage[key] = comm.data[0];
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
      sd.ip = comm.ip;
      sd.ori = comm.ori;
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
      int key = stoi(comm.data[0]);
      auto sid = this->query_chord(key);
      if (!sid.s){
        sd.data.push_back(comm.data[0]);
        sd.ip = comm.ip;
        sd.ori = comm.ori;
        sd.option = "QUERY_CHORD_GET";
        sd.reqres = "REQ";
        rn.data = sd.to_string();
        rn.peer_sockaddr = sid.data.peer_sockaddr;
        rn.err = 1;
        return rn;
      }
      if (sid.data.id == chord_id) {
        sd.option = "QUERY_CHORD_GET";
        string value = storage[key];
        sd.reqres = "RES";
        sd.data.push_back(value);
        rn.data = sd.to_string();
        //TODO remove repeated code
        struct sockaddr_in server;
        server.sin_port = htons(stoi(comm.ori));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(comm.ip.c_str());
        rn.peer_sockaddr = server;
        //TODO change err system
        rn.err = 1;
        return rn;
      }
      sd.ip = comm.ip;
      sd.ori = comm.ori;
      sd.option = "GET_CHORD";
      sd.reqres = "REQ";
      sd.data.push_back(comm.data[0]);
      rn.data = sd.to_string();
      rn.peer_sockaddr = sid.data.peer_sockaddr;
      //TODO change err system
      rn.err = 1;
      return rn;
    }
    else if (comm.option == "PUT_CHORD") {
      if (comm.reqres == "RES") {
        cout << print_chord_id() << " Got Key Back: " << comm.to_string() << endl;
        rn.err = -1;
        return rn;
      }
      int key = this->hash_chord(comm.data[0]);
      storage[key] = comm.data[0];
      sd.option = "PUT_CHORD";
      sd.reqres = "RES";
      sd.data.push_back(to_string(key));
      sd.ip = comm.ip;
      cout << comm.ip << '\n';
      struct sockaddr_in server;
      cout << comm.ori << '\n';
      server.sin_port = htons(stoi(comm.ori));
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = inet_addr(comm.ip.c_str());
      rn.data = sd.to_string();
      rn.peer_sockaddr = server;
      rn.err = 1;
      return rn;
    }
    else if (comm.option == "GET_CHORD") {
      if (comm.reqres == "RES") {
        cout << print_chord_id() << "Got VALUE Back" << comm.to_string() << endl;
        rn.err = -1;
        return rn;
      }
      int key = stoi(comm.data[0]);
      string value = storage[key];
      sd.option = "GET_CHORD";
      sd.reqres = "RES";
      sd.data.push_back(value);
      rn.data = sd.to_string();
      //TODO: change it to go to original client
      struct sockaddr_in server;
      server.sin_port = htons(stoi(comm.ori));
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = inet_addr(comm.ip.c_str());
      rn.peer_sockaddr = server;
      //TODO change err system
      rn.err = 1;
      return rn;
    }
    else if (comm.option == "GET_STATS") {
      cout << "id:"<< this->print_chord_id() << ":suc:"<< this->print_successor() << ":pre:" << this->print_predecessor()<< '\n';
      //sd.data.push_back("Chord Id: " + this->print_chord_id());
      rn.data = "" + this->print_chord_id() + ":" + this->print_successor() + ":" + this->print_predecessor();
      rn.peer_sockaddr = client_sockaddr;
      rn.err = 1;
      return rn;
    }
    else if (comm.option == "DISCONNECT") {
        commands::ro<string> ro = this->disconnect();
        if (!ro.s){
          rn.err = -2;
          return rn;
        }
        //TODO replace this double call
        auto s = rt.get_successor();
        sd.option = "PUT_CHORD";
        sd.reqres = "REQ";
        sd.data.push_back(ro.data);
        rn.data = sd.to_string();
        rn.peer_sockaddr = s.peer_sockaddr;
        rn.err = -2;
        return rn;
    }
    else if (comm.option == "UPDATE_SUCCESSOR_LIST") {
      if (comm.reqres == "REQ") {
        sd.option = "UPDATE_SUCCESSOR_LIST";
        sd.reqres = "RES";
        auto ss = rt.successor_list_front();
        sd.data.push_back(to_string(ss.id));
        sd.data.push_back(to_string(ntohs(ss.peer_sockaddr.sin_port)));
        rn.data = sd.to_string();
        rn.peer_sockaddr = client_sockaddr;
        rn.err = 1;
        return rn;
      }
      else if (comm.reqres == "RES") {
        storage::successor ss;
        ss.id = stoi(comm.data[0]);
        struct sockaddr_in server;
        server.sin_port = htons(stoi(comm.data[1]));
        server.sin_family = AF_INET;
        //TODO: Replace with ip addr
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        ss.peer_sockaddr = server;
        rt.successor_list_update(ss);
        reqtab.remove_request(stoi(comm.id), storage::Requests::update);
        rn.err = -1;
        return rn;
      }
    }
    else if (comm.option == "ALIVE_CHECK") {
      if (comm.reqres == "REQ") {
        sd.reqres = "RES";
        sd.option = "ALIVE_CHECK";
        rn.data = sd.to_string();
        rn.peer_sockaddr = client_sockaddr;
        rn.err = 1;
        return rn;
      }
      if (comm.reqres == "RES") {
        reqtab.remove_request(stoi(comm.id), storage::Requests::alive);
        rn.err = -1;
        return rn;
      }
    }
    else {
      // cout << "Unknown Command" << '\n';
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
    string send_data = to_string(chord_id) + ':' + "127.0.0.1" + ':' + to_string((_port));
    send_data += ":HANDSHAKE_CHORD:REQ:";
    send_data += to_string(chord_id);
    int csstatus = client_send(send_data, server);
    if(csstatus){
      //TODO this should be other chords id?
      reqtab.add_request(chord_id, storage::Requests::join, storage::Requests::handshake, server, send_data);
    }
  }

  int Node::stabilize_chord() {
    string send_data = to_string(chord_id) + ':' + "127.0.0.1" + ':' + to_string(ntohs(_port));
    auto s = rt.get_successor();
    auto p = rt.get_predecessor();
    if (s.id != chord_id) {
      send_data += ":STABILIZE_CHORD:REQ:";
      client_send(send_data, s.peer_sockaddr);
      return 1;
    }
    if ((s.id == chord_id) && (p.id != chord_id)){
      rt.stabilize_check(p.id, p.peer_sockaddr);
    }
  }

  void Node::successor_list_update(){
    string send_data = to_string(chord_id) + ':' + "127.0.0.1" + ':' + to_string(ntohs(_port));
    send_data += ":UPDATE_SUCCESSOR_LIST:REQ:";
    auto s = rt.get_successor();
    if (s.id != chord_id) {
      //TODO change in add, might be incorrect
      reqtab.add_request(s.id, storage::Requests::notify, storage::Requests::update, s.peer_sockaddr, send_data);
    }
  }

  void Node::alive_check(){
    string send_data = to_string(chord_id) + ':' + "127.0.0.1" + ':' + to_string(ntohs(_port));
    send_data += ":ALIVE_CHECK:REQ:";
    auto p = rt.get_predecessor();
    if (p.id != chord_id) {
      //TODO change in add, might be incorrect
      reqtab.add_request(p.id, storage::Requests::alive, storage::Requests::alive, p.peer_sockaddr, send_data);
    }
  }

  void* Node::ping(void){
    int stabilize = 0;
    int update_check = 0;
    while (true) {
        sleep(5);
        auto unful = reqtab.clear_requests(3);
        if (unful.size()){
          for (auto kv : unful) {
            int id = kv.id;
            storage::Requests res = kv.res;
            //TODO change from copies
            unfulfilled_handler(id, res);
          }
        }
        auto it = reqtab.request_storage.begin();
        while (it != reqtab.request_storage.end()) {
          auto ps = it->second;
          if (!ps.erase) {
            client_send(ps.data_sent, ps.peer_sockaddr);
          }
          reqtab.request_storage[it->first].ping_count++;
          ++it;
        }
        if (++stabilize == 3) {
          //todo split
          stabilize_chord();
          stabilize = 1;
        }
        if (++update_check == 6){
          successor_list_update();
        }
        if (update_check == 8){
              alive_check();
              update_check = 1;
        }
        //TODO: fix fingers
      }
    return NULL;
  }

  void Node::unfulfilled_handler(int id, storage::Requests res) {
    switch (res) {
      case storage::Requests::join : 0; break;
      // If didn't receive handshake, then throw error;
      case storage::Requests::handshake : break;
      /*If didn't receive stabilize response, then:
        successor is down. Find new successor.
      */
      case storage::Requests::update : rt.successor_fail(); break;
      case storage::Requests::alive : rt.predecessor_fail(); break;
      case storage::Requests::disconnect : 3; break;
    }
  }

  int Node::client_send(string data, sockaddr_in client_sockaddr){
    int numbytes;
    if ((numbytes = sendto(server_socket.get_sock_descriptor(), data.data(), data.size(), 0,
    (sockaddr *) &client_sockaddr, sizeof(client_sockaddr))) == -1) {
      perror("talker: sendto");
      cout << data << endl;
      return -1;
    }
    // cout << "client_sendmodule sent " << data << " to " << ntohs(client_sockaddr.sin_port) << '\n';
    return 1;
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
        // cout << "running server" << '\n';
        memset(buf, 0, sizeof buf);
        ::recvfrom(server_socket.get_sock_descriptor(), buf, 1024 -1 , 0,
        (struct sockaddr *) &their_addr, &addr_len);
        //PEER COMMANDS
        //TODO change to commands.h
        data = buf;
        //TODO not working

          //TODO replace and standardize around string or char
          // cout << "data received from peer " << data << '\n';
          auto rn = this->remote_node_controller(data, their_addr);
          if (rn.err > 0){
            this->client_send(rn.data, rn.peer_sockaddr);
          }
          if (rn.err == -2){
            cout << "disconnect" << '\n';
            this->client_send(rn.data, rn.peer_sockaddr);
            run_server = false;
          }
      }
      return NULL;
    }

    commands::ro<string> Node::disconnect(){
      // call partner node send disconnect message
      // Send disconnect call to partner node
      // TODO: store in internal state whether or not in disconnect mode
      // TODO: Build an internal state object
      // Once keyspace has been updated with lower & higher, run disconnect to update
      auto s = rt.get_successor();
      commands::ro<string> ro;
      if (s.id == chord_id) {
          ro.s = -1;
          return ro;
      }
      for (auto& kv: storage) {
        ro.data += (kv.second + ':');
      }
      ro.s = 1;
      return ro;
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
          // cout << kv.first << '\n';
          resp = this->get_value(kv.first);
          if (resp.s) {
            sd += resp.data;
          }
        }
        //Decide if going to erase from memory
      }
      // cout << "reshuffle " << sd << '\n';
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
