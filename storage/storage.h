#ifndef STORAGE_H
#define STORAGE_H

#include <netinet/in.h>
#include <map>
#include <string>
#include <iostream>

namespace storage {

struct peer_storage {
  sockaddr_in peer_sockaddr;
  std::string data_sent;
  //TODO change to be map for multiple responses needed
  std::string response_needed;
  int ping_count = 0;
};

struct peer {
  std::string id;
  sockaddr_in peer_sockaddr;
};

struct successor {
  int id;
  sockaddr_in peer_sockaddr;
};

struct peer_list {
  peer lower;
  peer higher;
};

class RoutingTable {
private:
  std::map<int, peer_storage> fingers;
  //Sets the number of bytes in keyspace
  successor s;
  //predecessor
  successor p;
  int id;

public:
  RoutingTable(int id) : id(id) { s.id = id; }
  int find_successor(int k) {
    bool member = check_membership(id, s.id, k);
    if (member) {
      return s.id;
    }
    else {
      //successor n = nearest_neighbor(k);
      //return struct with data, values, and then use that
      return -1;
    }
  }
  bool check_membership(int self, int suc, int key) {
    //CHECK THIS DOESN"T SEEM RIGHT
    if (self == suc) {
      return true;
    }
    if (self <= suc) {
      return (key >= self && key <= suc);
    } else {
      return (key <= self || key >= suc);
    }
  }
  bool notify_check(int k){
    if (!p.id){
      return true;
    }
    return check_membership(p.id, id, k);
  }
  bool stabilize_check(int sip, sockaddr_in address){
    /*  x = successor.predecessor;
    if (x∈(n, successor))
     successor = x;
     successor.notify(n);
   */
    //Ask successor what its predecessor is
    //Check if successor predecessor in range
    if (id == sip){
      return false;
    }
    if (s.id == sip) {
      return false;
    }
    if (check_membership(id, s.id, sip)){
        update_successor(sip, address);
        return true;
    }
    return false;
  }
  int nearest_neighbor(int id){
    //TODO: implement list of other nodes
    return s.id;
  };
  void update_successor(int id, sockaddr_in address) {
    std::cout << "new successor" + std::to_string(id) << '\n';
    s.id = id;
    s.peer_sockaddr = address;
  }
  void update_predecessor(int id, sockaddr_in address){
    p.id = id;
    p.peer_sockaddr = address;
  }
  successor get_successor(){
    //TODO: combine with above functions
      return s;
  }
  successor get_predecessor(){
    //TODO: combine with above functions
      return p;
  }
};
}//namespace storage;

#endif
