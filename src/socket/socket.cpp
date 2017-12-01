// Implementation of the Socket class.


#include "socket.h"
#include "string.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>


Socket::Socket() : m_sock ( -1 ) {
  //Start by setting the setting as not available
  //Sets the first num bytes of the block of memory pointed by ptr to the specified value (interpreted as an unsigned char).
  memset ( &m_addr, 0, sizeof ( m_addr ) );
}

Socket::~Socket() {
  //on destruction close the socket
  if ( is_valid() )
  ::close ( m_sock );
}

bool Socket::create() {
  // Create a streaming TCP socket. Setting to 0 automatically chooses the proper protocol.
  m_sock = socket ( AF_INET, SOCK_DGRAM, 0 );

  if ( ! is_valid() )
  return false;
  // TIME_WAIT - argh
  int on = 1;

  //Checks if setsockopt has successfully set up the socket
  /*int setsockopt(int socket, int level, int option_name,
  const void *option_value, socklen_t option_len); */
  //The level argument specifies the protocol level at which the option resides. To set options at the socket level, specify the level argument as SOL_SOCKET
  //SO_REUSEADDR allows reuse of local addresses
  if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof (on) ) == -1 )
  return false;

  return true;
}



bool Socket::bind ( const int port ) {
  if ( ! is_valid() )
  {
    return false;
  }

  //Setting up the ports and addresses
  m_addr.sin_family = AF_INET;
  //Binds it to all local interfaces (all the ips of the machine)
  m_addr.sin_addr.s_addr = INADDR_ANY;
  // The htons() function converts the unsigned integer hostlong from host byte order to network byte order
  m_addr.sin_port = htons ( port );

  int bind_return = ::bind ( m_sock,
    ( struct sockaddr * ) &m_addr,
    sizeof ( m_addr ) );

    if ( bind_return == -1 )
    {
      return false;
    }

    return true;
  }


  bool Socket::listen() const
  {
    if ( ! is_valid() )
    {
      return false;
    }

    int listen_return = ::listen ( m_sock, MAXCONNECTIONS );


    if ( listen_return == -1 )
    {
      return false;
    }

    return true;
  }

  // /The accept() function extracts the first connection on the queue of pending connections
  // creates a new socket with the same socket type protocol and address family as the specified socket, and allocates a new file descriptor for that socket.
  Socket& Socket::accept ( Socket& new_socket ) const
  {
    int addr_length = sizeof ( m_addr );
    new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

    //  if ( new_socket.m_sock <= 0 )
    //    return -1;
    //  else
    return new_socket;
  }


  bool Socket::send ( const std::string s ) const
  {
    int status = ::send ( m_sock, s.c_str(), s.size(), MSG_NOSIGNAL );
    if ( status == -1 )
    {
      return false;
    }
    else
    {
      return true;
    }
  }


  int Socket::recv ( std::string& s ) const
  {
    char buf [ MAXRECV + 1 ];

    s = "";

    memset ( buf, 0, MAXRECV + 1 );

    //this fills the buffer with everything that has been received on the socket
    int status = ::recv ( m_sock, buf, MAXRECV, 0 );
    if ( status == -1 )
    {
      std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
      return 0;
    }
    else if ( status == 0 )
    {
      return 0;
    }
    else
    {
      s = buf;
      return status;
    }
  }



  bool Socket::connect ( const std::string host, const int port )
  {
    if ( ! is_valid() ) return false;

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons ( port );

    int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

    if ( errno == EAFNOSUPPORT ) return false;

    status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

    if ( status == 0 )
    return true;
    else
    return false;
  }

  bool Socket::close()
  {
    if ( is_valid() )
    ::close ( m_sock );
  }

  // Manipulates file flag to set as blocking or non-blocking
  void Socket::set_non_blocking ( const bool b )
  {

    int opts;

    opts = fcntl ( m_sock,
      F_GETFL );

      if ( opts < 0 )
      {
        return;
      }

      if ( b )
      opts = ( opts | O_NONBLOCK );
      else
      opts = ( opts & ~O_NONBLOCK );

      fcntl ( m_sock,
        F_SETFL,opts );

      }

      bool Socket::is_closed(){
        char buf [ MAXRECV + 1 ];
        if (::recv(m_sock, buf, MAXRECV, MSG_PEEK | MSG_DONTWAIT) == 0) {
          return true;
        }
        return false;
      }
