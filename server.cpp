/**************************************************************************
*	Simple C++ server.
**************************************************************************/
#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "node.h"

using boost::asio::ip::tcp;

std::string make_daytime_string() {
  std::time_t now = std::time(0);
  return ctime(&now);
}

int main()
{
  try
  {
    //create io_service
    boost::asio::io_service io_service;
    //tcp socket
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 13));
    for (;;) {
      tcp::socket socket(io_service);
      acceptor.accept(socket);
      std::string message = make_daytime_string();

      boost::system::error_code ignored_error;
      std::string s = socket.remote_endpoint().address().to_string();
      std::cout << s << '\n';
      boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
    }
   }
   catch (std::exception& e)
   {
     std::cerr << e.what() << std::endl;
   }

   return 0;
}
