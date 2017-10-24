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

//RENAME successor and all peer
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
  std::map<int, successor> fingers;
  //Sets the number of bytes in keyspace
  successor s;
  //predecessor
  successor p;
  int id;

public:
  RoutingTable(int id) : id(id) { s.id = id; }
  successor find_successor(int k, int * opt_flag) {
    bool member = check_membership(id, s.id, k);
    if (member) {
      return s;
    }
    else {
      *opt_flag = 0;
      successor n = get_closest_node(k);
      //return struct with data, values, and then use that
      return n;
    }
  }
  bool check_membership(int a, int b, int c) {
    std::cout << std::to_string(a) << ':' << std::to_string(b) << ':' << std::to_string(b) << '\n';

    //CHECK THIS DOESN"T SEEM RIGHT, should be false
    if (a == b) {
      return true;
    }
    if (a < b) {
      return (a <= c && c < b);
    } else {
      return (a <= c || c < b);
    }
  }
  bool notify_check(int k){
    if (!p.id){
      return true;
    }
    return check_membership(p.id, id, k);
  }
  bool stabilize_check(int sip, sockaddr_in address){
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
  void update_successor(int id, sockaddr_in address) {
    std::cout << "new successor" + std::to_string(id) << '\n';
    s.id = id;
    s.peer_sockaddr = address;
  }
  void update_predecessor(int id, sockaddr_in address) {
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
  successor get_closest_node(int k){
    std::map<int,storage::successor>::iterator it = fingers.end();
    while (it != fingers.begin()) {
      bool member = check_membership(it->first, id, k);
      if (member) {
        return it->second;
      }
      else {
        it++;
      }
    }
    return get_successor();
  }
  void fix_fingers(int nf) {
    int * opt;
    fingers[nf] = find_successor(id + (2^(nf-1)), opt);
  }
};
}//namespace storage;

#endif
