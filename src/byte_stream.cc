#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity )
  , bytes_popped_( 0 )
  , bytes_pushed_( 0 )
  , is_closed_( false )
  , stream_()
  , peek_buffer_( "" )
{}

uint64_t ByteStream::total_capacity() const
{
  return capacity_;
}

bool Writer::is_closed() const
{
  return is_closed_;
}

void Writer::push( string data )
{
  if ( is_closed_ ) {
    return;
  }

  for ( size_t i = 0; i < data.size(); i++ ) {
    if ( available_capacity() > 0 ) {
      stream_.push( data[i] );
      bytes_pushed_++;
    }
  }

  peek_buffer_ = stream_.front();
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
  return string_view( peek_buffer_ );
}

void Reader::pop( uint64_t len )
{
  if ( bytes_buffered() < len ) {
    set_error();
    return;
  }

  if ( is_finished() ) {
    return;
  }

  for ( uint64_t i = 0; i < len; i++ ) {
    stream_.pop();
  }

  bytes_popped_ += len;

  if ( stream_.empty() ) {
    peek_buffer_ = "";
  } else {
    peek_buffer_ = stream_.front();
  }
}

uint64_t Reader::bytes_buffered() const
{
  return stream_.size();
}
