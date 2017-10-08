/**************************************************************************
*	Node class that holds and runs/returns properties around
**************************************************************************/
#include <ctime>
#include <iostream>
#include <string>
#include <exception>
#include "node.h"

using namespace std;

namespace node {
  //TODO(goldhaber) Add in option to not connect and be solitary node
Node::Node(int port=3001, string partner_address = "127.0.0.1", int partner_port=3000)
  : _partner_address(partner_address), _partner_port(partner_port), _port(port) {
  //create client socket to reach out to partner node (if exists)
  client_socket.create();
  server_socket.create();

  if (client_socket.connect(partner_address, partner_port)) {
    //gets the key space it needs to operate in
  }
  else {
    cout << "Could not bind to client port. Setting keyspace as all." << '\n';
    set_keyspace(0);
  }
  //Bind to the port provided
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

  //receive and store data
}

Node::~Node() {
 //call disconnect function
 //reallocate storage
}

int Node::run(){
 // start up a server
 // runs until it receives (tk) a command to turn off
 // Handles all put get calls

  bool run_server = true;
  while(run_server) {
    string data;
    server_socket.accept(accept_socket);
    accept_socket.recv(data);
    //Currently echoes data received. Parse for commands.
    cout << data << '\n';
  }
  return 0;
}

int Node::disconnect(){
 // call partner node send disconnect message
}

int Node::add(string address){

}

int Node::put_value(string val){
  int hash;
  for (size_t i = 0; i < string.size(); i++) {
    hash += ord(string[i])
  }
  hash = hash % 100;
  if ((hash >= key_space[0]) || (hash <= key_space[1])) {
    storage[hash] = val;
    return hash;
  }
  else {
    //CHECK PEER LIST
    return;
  }
}

int Node::get_value(int key){

}

int Node::set_keyspace(int nodes = 0){
  if (nodes == 0){
    key_space[0] = 0;
    key_space[1] = 100;
  }
  cout << "Key Space: " << key_space[0] << ':' << key_space[1] << '\n';
 // if connected to network, get the keyspace of the connected node
 // send message to that node that it now is only responsible for start -> mid-1
 // set self keyspace to mid -> end
}

string Node::get_address(){
// return current address
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


int main()
{
   node::Node n;
   n.print_partner();
   n.run();
   return 0;
}
