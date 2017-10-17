#include "node.h"
#include <string>
using namespace std;

int main(int argc, char *argv[])
{
  int port = atoi(argv[1]);
  string paddress = argv[2];
  int pport = atoi(argv[3]);
  node::Node n(port, paddress, pport);
  n.run();
  return 0;
}
