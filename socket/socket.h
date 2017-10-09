// Definition of the Socket class

#ifndef Socket_class
#define Socket_class


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>


const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 500;

class Socket
{
 public:
  Socket();
  virtual ~Socket();
  // Server initialization
  bool create();
  bool bind ( const int port );
  bool listen() const;
  bool accept ( Socket& ) const;
  // Client initialization
  bool connect ( const std::string host, const int port );
  // Returns true false if the string has sent
  bool close ();
  bool send ( const std::string ) const;
  // Receive data from the socket
  int recv ( std::string& ) const;
  void set_non_blocking ( const bool );
  // Returns if m_sock is available
  bool is_valid() const { return m_sock != -1; }
 private:
   // close the socket
  int m_sock;
  /*
  struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET, AF_INET6
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
  };*/
  sockaddr_in m_addr;
};


#endif
