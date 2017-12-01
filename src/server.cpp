#include "node.h"
#include <string>
#include <pthread.h>
#include <ctime>

using namespace std;

int main(int argc, char *argv[])
{
  //TODO: change
  srand(time(NULL));
  int port = atoi(argv[1]);
  string paddress = argv[2];
  int pport = atoi(argv[3]);
  int chord_id = atoi(argv[4]);
  node::Node n(port, paddress, pport, chord_id);
  n.run();
  //TODO, create several nodes from the get go to simulate network;
  return 0;
}
