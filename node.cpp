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

#define NUM_THREADS     5

#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 100

using namespace std;

namespace node {
  Node::Node(int port=4950, string partner_address = "127.0.0.1", int partner_port=3002)
  : _partner_address(partner_address), _partner_port(partner_port), _port(port) {

    client_socket.create();
    server_socket.create();
    this->set_keyspace(0,0,100);

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
    // start up a server
    // runs until it receives (TODO) a command to turn off
    // Handles all put get calls
    //set up as class variable? Thread pools?
    pthread_t threads[ NUM_THREADS ];
    string thread_args[1];
    int result_code;
    thread_args[0] = "initial client";
    //Refactor call function to not be separate thread, not needed now
    result_code = pthread_create(&threads[0], NULL, Node::callAddFunction, this);
    pthread_join(threads[0], NULL);
    result_code = pthread_create(&threads[1], NULL, Node::callPingFunction, this);
    pthread_detach(threads[1]);
    result_code = pthread_create(&threads[2], NULL, Node::callServerFunction, this);
    pthread_join(threads[2], NULL);

    return 0;
  }

  //TODO(): turn into struct of options
  void Node::remote_node_controller(const string option, sockaddr_in client_sockaddr) {
    //handle incoming connection and parsed data
    //call other internal functions
    //send to other functions
    commands::Commands comm(option);
    cout << "option " << comm.option << '\n';
    string send_data = id + ':';
    string buff = "";
    int i = 2;
    string ip_addr = inet_ntoa(client_sockaddr.sin_addr);
    if (comm.option == "C1") {
      if (comm.reqres == "ACK") {
        request_list.erase(ip_addr);
      }
      else {
        struct sockaddr_in server;
        server.sin_port = htons(_partner_port);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
        send_data += "C2:REQ";
        int csstatus = clientsend(send_data, server);
        if(csstatus){
          storage::peer_storage ps;
          ps.peer_sockaddr = client_sockaddr;
          ps.data_sent = send_data;
          ps.response_needed = "C1";
          request_list[ip_addr] = ps;
        }
      }

    } else if (comm.option == "C2") {
      if (comm.reqres == "ACK") {
        this->set_keyspace(1, stoi(comm.data[0])+1, key_space[1]);
        request_list.erase(ip_addr);
      }
      else {
        int mid = key_space[1] / 2;
        send_data += "C4:RES:" + to_string(key_space[0]) + ':' + to_string(mid);
        int csstatus = clientsend(send_data, client_sockaddr);
        if(csstatus){
          storage::peer_storage ps;
          ps.peer_sockaddr = client_sockaddr;
          ps.data_sent = send_data;
          ps.response_needed = "C2";
          request_list[ip_addr] = ps;
        }
      }
    } else if (comm.option == "C3") {

    }
    //C4: Set Keyspace
    //UGH change this
    else if (comm.option == "C4") {
      request_list.erase(ip_addr);
      cout << "c4" << '\n';
      int key_lower, key_higher;
      key_lower = stoi(comm.data[0]);
      key_higher = stoi(comm.data[1]);
      send_data += "C2:ACK:" + comm.data[1];
      this->set_keyspace(1, key_lower, key_higher);
      clientsend(send_data, client_sockaddr);
    }
    else if (comm.option == "C5"){
      //C5: Put values
      //TODO replace with command struct
      send_data += "N2:RES:";

        size_t key = this->put_value(comm.data[0]);
        send_data += to_string(key);

      cout << "sent key: " << send_data << '\n';
      clientsend(send_data, client_sockaddr);
    }
    else if (comm.option == "C6"){
      string * sdp = &send_data;
      send_data += "N3:RES:";
      int status = this->get_value(stoi(comm.data[0]), sdp);
      if (!status) {
        perror("error in get value; not present");
        //Call pass to peer
      }
      else {
        cout << "sent value: " << send_data << '\n';
        clientsend(send_data, client_sockaddr);
      }
    }
    else {
      cout << "Unknown Command" << '\n';
    }
  }

  int Node::disconnect(){
    // call partner node send disconnect message
  }

  void* Node::add(void){
    struct sockaddr_in server;
    server.sin_port = htons(_partner_port);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    string send_data = id + ':';
    send_data += "C2:REQ";
    int csstatus = clientsend(send_data, server);
    if(csstatus){
      storage::peer_storage ps;
      ps.peer_sockaddr = server;
      ps.data_sent = send_data;
      ps.response_needed = "C1";
      request_list[inet_ntoa(server.sin_addr)] = ps;
    }
  return NULL;
}

void* Node::ping(void){
  std::map<string,storage::peer_storage>::iterator it;
  while (true) {
    /* code */
  sleep(10);
  for (it=request_list.begin(); it!=request_list.end(); ++it) {
    auto ps = it->second;
    clientsend(ps.data_sent, ps.peer_sockaddr);
    cout << it->first << " pinged." << '\n';
  }
  }

  return NULL;
}

int Node::clientsend(string data, sockaddr_in client_sockaddr){
  int numbytes;
  if ((numbytes = sendto(server_socket.get_sock_descriptor(), data.data(), data.size(), 0,
  (sockaddr *) &client_sockaddr, sizeof(client_sockaddr))) == -1) {
    perror("talker: sendto");
    return -1;

  }
  cout << "clientsendmodule sent " << data << " to " << ntohs(client_sockaddr.sin_port) << '\n';
  return 1;
}

void* Node::server(void){

  char buf [ 1024 ];
  char *ip;
  int port;
  int senderr;
  struct sockaddr_in their_addr;
  socklen_t addr_len;
  string data = "";
  bool run_server = true;
  while(run_server) {
    cout << "running server" << '\n';
    ::recvfrom(server_socket.get_sock_descriptor(), buf, 1024 -1 , 0,
    (struct sockaddr *) &their_addr, &addr_len);
    cout << buf << '\n';

    //PEER COMMANDS
    //TODO change
    if (buf[3] == 'C'){
      //TODO replace and standardize around string or char
      data = buf;
      cout << "data received from peer" << data << '\n';
      this->remote_node_controller(data, their_addr);
    }
    //USER COMMANDS
    else if(buf[0] == 'U'){
      cout << "data received from user" << '\n';

    }
    else if (strcmp(buf, "close") == 0){
      cout << "shutting down" << '\n';
      run_server = false;
    }
    else {
      cout << buf << '\n';
    }


  }
  return NULL;
}

size_t Node::put_value(string val){
  hash<string> hashf;
  size_t key;
  key = hashf(val);
  key = key % 100;

  if ((key >= key_space[0]) || (key <= key_space[1])) {
    storage[key] = val;
    return key;
  }
  else {
    //CHECK PEER LIST
    return -1;
  }
}

int Node::get_value(size_t key, string * data){
  map<size_t, string>::iterator iter = storage.find(key);
  if ( storage.end() != iter ) {
    *data += storage[key];
    return 1;
  }
  return -1;
}


int Node::set_keyspace(int nodes = 0, int key_lower = 0, int key_higher = 100){
  if (nodes == 0) {
    key_space[0] = key_lower;
    key_space[1] = key_higher;
  }
  else {
    //TODO(goldhaber): Add more logic for more nodes
    key_space[0] = key_lower;
    key_space[1] = key_higher;
  }
  cout << "Key Space: " << key_space[0] << ':' << key_space[1] << '\n';
  return 1;
  // if connected to network, get the keyspace of the connected node
  // send message to that node that it now is only responsible for start -> mid-1
  // set self keyspace to mid -> end
}

string Node::get_address(){
  // return current address
}

int* Node::get_keyspace(){
  return key_space;
}

string Node::print_partner(){
  cout << _partner_address << '\n';
  cout << client_socket.is_valid() << '\n';
}

string Node::current_time(){
  time_t now = std::time(0);
  return ctime(&now);
}

} // node

//TODO (add command line arguments)
int main(int argc, char *argv[])
{
  if (argc < 2){
    node::Node n;
    n.run();
  } else {
    int port = atoi(argv[1]);
    string paddress = argv[2];
    int pport = atoi(argv[3]);
    node::Node n(port, paddress, pport);
    n.run();
  }
  return 0;
}
