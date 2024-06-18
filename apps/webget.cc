#include "socket.hh"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <span>
#include <string>

using namespace std;

void get_URL( const string& host, const string& path )
{
  std::unique_ptr<TCPSocket> socket = std::make_unique<TCPSocket>();
  const auto& host_address = Address( host, "http" );
  const auto& local_address = Address( "127.0.0.1", 8080 );

  socket->bind( local_address );
  socket->connect( host_address );

  std::string http_request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n";
  std::vector<std::string> requests = { http_request };

  socket->write( requests );

  std::string http_response = "";
  while ( !socket->eof() ) {
    socket->read( http_response );
    std::cout << http_response;
  }
}

int main( int argc, char* argv[] )
{
  try {
    if ( argc <= 0 ) {
      abort(); // For sticklers: don't try to access argv[0] if argc <= 0.
    }

    auto args = span( argv, argc );

    // The program takes two command-line arguments: the hostname and "path" part of the URL.
    // Print the usage message unless there are these two arguments (plus the program name
    // itself, so arg count = 3 in total).
    if ( argc != 3 ) {
      cerr << "Usage: " << args.front() << " HOST PATH\n";
      cerr << "\tExample: " << args.front() << " stanford.edu /class/cs144\n";
      return EXIT_FAILURE;
    }

    // Get the command-line arguments.
    const string host { args[1] };
    const string path { args[2] };

    // Call the student-written function.
    get_URL( host, path );
  } catch ( const exception& e ) {
    cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
