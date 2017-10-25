#ifndef STORAGE_H
#define STORAGE_H

#include <netinet/in.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>

namespace storage {

  //TODO move to different namespace
  typedef enum {
    join, handshake, stabilize, disconnect
  } Requests;

  struct request_id {
    int id;
    Requests res;

    bool const operator == (const request_id &o) const {
      return id == o.id && res == o.res;
    }

    bool const operator<(const request_id &o) const {
     return id < o.id || (id == o.id && res != o.res);
    }
  };

  typedef struct request_id request_id;


struct peer_storage {
  sockaddr_in peer_sockaddr;
  std::string data_sent;
  //TODO change to be map for multiple responses needed
  Requests request;
  //TODO: deprecate string in favor of enum
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

class RequestTable {
private:
  std::map<request_id, peer_storage> request_storage;
public:
  //add a request
  int add_request(int id, Requests res, sockaddr_in peer_sockaddr,
                  Requests req, std::string data_sent) {
    request_id r; r.id = id; r.res = res;
    struct peer_storage ps; ps.peer_sockaddr = peer_sockaddr; ps.request = req;
    ps.data_sent = data_sent;
    request_storage[r] = ps;
    return 1;
  };

  int check_request(int id, Requests res) {
    request_id r; r.id = id; r.res = res;
    auto it = request_storage.find(r);
    if (it != request_storage.end()) {
      //TODO change
      return remove_request(id, res);
    }
    else -1;
  }
  int get_size() { return request_storage.size(); }
  //if request has been fulfilled, remove it
  int remove_request(int id, Requests res) {
    request_id r; r.id = id; r.res = res;
    request_storage.erase(r);
    return 1;
  };
  //for requests over a certain threshold, remove all of them
  std::vector<request_id> clear_requests(int ping_number) {
    std::vector<request_id> unfulfilled_reqs;
    auto it = request_storage.begin();
    while (it != request_storage.end()) {
      if (it->second.ping_count > ping_number){
        unfulfilled_reqs.push_back(it->first);
        it = request_storage.erase(it);
      } else {
        ++it;
      }
    }
    return unfulfilled_reqs;
  };
  //if a request was unfulfilled, and had unfulfilled conditions, notify
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
  RoutingTable(int id) : id(id) {
    s.id = id;
    fingers[1] = s;
  }
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
    std::cout << std::to_string(a) << ':' << std::to_string(b) << ':' << std::to_string(c) << '\n';
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
  successor fix_fingers(int nf) {
    //int * opt;
    //Check if still online
    //Ping that address, see if it returns traffic
    //fingers[nf] = find_successor(id + (2^(nf-1)), opt);
    return fingers[nf];
  }
};
}//namespace storage;

#endif
