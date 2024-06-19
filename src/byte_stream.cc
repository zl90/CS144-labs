#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), bytes_popped_(0), bytes_pushed_(0), is_closed_(false) {}

bool Writer::is_closed() const
{
  return is_closed_;
}

void Writer::push( string data )
{
  // Your code here.
  (void)data;
  return;
}

void Writer::close()
{
  is_closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - stream_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  return is_closed_ && stream_.empty();
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

string_view Reader::peek() const
{
  auto front = stream_.front();
  string s;
  s += front;
  return string_view(s);
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  (void)len;
}

uint64_t Reader::bytes_buffered() const
{
  return stream_.size();
}
