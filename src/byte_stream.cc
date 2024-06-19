#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), bytes_popped_(0), bytes_pushed_(0) {}

bool Writer::is_closed() const
{
  // Your code here.
  return {};
}

void Writer::push( string data )
{
  // Your code here.
  (void)data;
  return;
}

void Writer::close()
{
  // Your code here.
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return {};
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return {};
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

string_view Reader::peek() const
{
  // Your code here.
  return {};
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  (void)len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return {};
}
