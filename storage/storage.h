#ifndef STORAGE_H
#define STORAGE_H

#include <netinet/in.h>
#include <map>
#include <string>

namespace storage {

struct peer_storage {
  sockaddr_in peer_sockaddr;
  std::string data_sent;
  //TODO change to be map for multiple responses needed
  std::string response_needed;
};

struct peer {
  std::string id;
  sockaddr_in peer_sockaddr;
};

struct peer_list {
  peer lower;
  peer higher;
};
}//namespace storage;

#endif
