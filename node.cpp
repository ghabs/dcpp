/**************************************************************************
*	Node class that holds and runs/returns properties around
**************************************************************************/
#include <ctime>
#include <iostream>
#include <string>
#include "node.h"


using namespace std;

namespace node {
Node::Node(string partner = "test") : _partner(partner) {
  //reach out to partner node (if exists)
  //gets the key space it needs to operate in
  //receive and store data
}

Node::~Node() {
 //call disconnect function
 //reallocate storage
}

int Node::run(string port){
 // start up a server
 // runs until it receives (tk) a command to turn off
 // Handles all put get calls
 try
 {
   //create io_service
   //create tcp socket
   //for (;;) {
     //set socket to accept
     //handles calls to other functions
   //}
  }
  catch (exception& e)
  {
    cerr << e.what() << endl;
  }
  return 0;
}

int Node::disconnect(){
 // call partner node send disconnect message
}

int Node::add(string address){

}

int Node::put_value(string val){

}

int Node::get_value(int key){

}

string Node::get_address(){
// return current address
}

string Node::print_partner(){
  cout << _partner << '\n';
}

string Node::current_time(){
  time_t now = std::time(0);
  return ctime(&now);
}
} // node


int main()
{
   node::Node n("a");
   n.print_partner();
   return 0;
}
