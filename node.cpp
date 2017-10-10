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

#define NUM_THREADS     5

using namespace std;

namespace node {
  Node::Node(int port=3001, string partner_address = "127.0.0.1", int partner_port=3000)
  : _partner_address(partner_address), _partner_port(partner_port), _port(port) {
    client_socket.create();
    server_socket.create();
    server_socket.set_non_blocking(true);

    //TODO(goldhaber): throw exception
    if (!server_socket.bind(port))
    {
      cout << "Could not bind to port." << '\n';
    }
    else {
      cout << "Node has bound to port " << port << '\n';
    }
    //listen for incoming connections (while not yet running / accepting new connections)
    if (!server_socket.listen()) {
      cout << "Could not listen to socket."  << '\n';
    }
    else {
      cout << "Node is listening on port " << port << '\n';
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
  result_code = pthread_create(&threads[0], NULL, Node::callAddFunction, this);
  pthread_detach(threads[0]);
  //Should be changed to allow multiple server connections
  //That means multiple accept sockets and polling, monitoring if they are all shut
  result_code = pthread_create(&threads[1], NULL, Node::callServerFunction, this);
  pthread_join(threads[1], NULL);

  return 0;
}

//TODO(): turn into struct of options
void Node::remote_node_controller(const string option) {
    //handle incoming connection and parsed data
    //call other internal functions
    //send to other functions
    commands::Commands comm;
    comm.option = option.substr(0,2);
    string send_data = "";
    string buff = "";
    int i = 2;
    if (comm.option == "C1") {
      /* code */
    } else if (comm.option == "C2") {
      //THIS IS INCORRECT
      cout << key_space[1] << '\n';
      send_data += "C4" + to_string(key_space[0]) + ':' + to_string(key_space[1]);
      accept_socket.send(send_data);
    } else if (comm.option == "C3") {

    }
    //C4: Set Keyspace
    //UGH change this
    else if (comm.option == "C4") {
      int key_lower, key_higher;
      //TODO replace with command struct
      while (option[i] != '\n') {
        if (option[i] == ':') {
          key_lower ? key_higher = stoi(buff) : key_lower = stoi(buff);
          buff = "";
        }
        else {
          buff += option[i];
        }
        i++;
      }
      this->set_keyspace(1, key_lower, key_higher);
    }
    else if (comm.option == "C5"){
      //C5: Put values
      //TODO replace with command struct
      while (option[i] != '\n') {
        if (option[i] == ':') {
          //return keys to original caller
          comm.data[(sizeof(comm.data)-1)] = buff;
          buff = "";
        }
        else {
          buff += option[i];
        }
        i++;
      }
    send_data += "N2";
    for (size_t i = (sizeof(comm.data)-1); i >= 0; i--) {
      int key = this->put_value(comm.data[i]);
      send_data += key;
      send_data += ':';
    }
    //Is this blocking? How does this work async?;
    accept_socket.send(send_data);
    }
    else {
      cout << "Unknown Command" << '\n';
    }
}

int Node::disconnect(){
 // call partner node send disconnect message
}

// To be called in separate thread
void* Node::add(void){
  cout << "called add!" << '\n';
  string data;
  _client_status = client_socket.connect(_partner_address, _partner_port);

  if (_client_status) {
    //gets the key space it needs to operate in
    //receive a connection message from the node
    //this->set_keyspace(1, , );
    bool sent;
    sent = client_socket.send("C2");
    if (!sent){
      cout << "Error sending message." << '\n';
    }
    client_socket.recv(data);
    cout << "client: " << data << '\n';
    this->remote_node_controller(data);
    // Check if theres data to send, (how to do that between threads)
    //How to keep socket open and then pass in more data to the thread?
    client_socket.close();
  }
  else {
    cout << "Could not bind to client port. Setting keyspace as all." << '\n';
    this->set_keyspace(0,0,100);
    return NULL;
  }
  return NULL;
}

void* Node::server(void){
  struct pollfd fds[NUM_THREADS];
  memset(fds, 0 , sizeof(fds));
  char buf [ 1024 ];
  int opts;
  opts = ( opts | O_NONBLOCK );
  //5 second polling rate
  int timeout = (5 * 1000);
  //number of sockets to poll
  int nsock = 1;
  int current_size;
  int rc;
  int test_server;
  test_server = socket ( AF_INET, SOCK_STREAM, 0 );
  opts =  fcntl ( test_server, F_GETFL );

  fcntl ( test_server, F_SETFL, opts );
  string data;

  fds[0].fd = test_server;
  fds[0].events = POLLIN;


  bool run_server = true;
  while(run_server) {
    cout << "running server" << '\n';
    //This only blocks until something happens on the monitored port, then it becomes available
    sleep(3);
    rc = poll(fds, nsock, timeout);
    current_size = nsock;
    for (size_t i = 0; i < current_size; i++) {
      if (rc == 0) {
        cout << "nothing happened" << '\n';
      }
      else if (fds[0].revents & POLLIN){
        cout << "accepting data" << '\n';
        server_socket.accept(accept_socket);
        accept_socket.set_non_blocking(true);
        fds[nsock].fd = accept_socket.get_sock_descriptor();
        fds[nsock].events = POLLIN;
        nsock++;
        cout << nsock << '\n';
      }
      else {
        ::read ( fds[1].fd, buf, 1023);
        for (size_t i = 0; i < current_size; i++) {
          cout << fds[i].revents << '\n';
        }
        cout << buf << '\n';
      }
        if (data[0] == 'C'){
          cout << "data received" << '\n';
          this->remote_node_controller(data);
        }
        else if (data == "close\n"){
          cout << "shutting down" << '\n';
          run_server = false;
        }
        else {
          //Currently echoes data received. Parse for commands.
          //cout << data << '\n';
        }
      data = "";
    }
  }
  return NULL;
}

int Node::put_value(string val){
  int hash;
  int ord;
  for (size_t i = 0; i < val.size(); i++) {
    ord = val[i];
    hash += ord;
  }
  hash = hash % 100;
  if ((hash >= key_space[0]) || (hash <= key_space[1])) {
    storage[hash] = val;
    return hash;
  }
  else {
    //CHECK PEER LIST
    return -1;
  }
}

int Node::get_value(int key){

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
