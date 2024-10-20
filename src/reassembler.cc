#include "reassembler.hh"

using namespace std;

bool Reassembler::is_overlapping( uint64_t index, const string& data )
{
  return output_.writer().bytes_pushed() > index && output_.writer().bytes_pushed() < index + data.length();
}

bool Reassembler::is_duplicate( uint64_t index, const string& data )
{
  return output_.writer().bytes_pushed() >= index + data.length();
}

bool Reassembler::is_out_of_order( uint64_t index )
{
  return index > output_.writer().bytes_pushed();
}

bool Reassembler::is_in_order( uint64_t index )
{
  return output_.writer().bytes_pushed() == index;
}

void Reassembler::store( uint64_t index, const string& data )
{
  if ( buffer_.find( index ) == buffer_.end() ) {
    bytes_pending_ += data.length();
    buffer_[index] = data;
  } else if ( data.length() > buffer_[index].length() ) {
    bytes_pending_ += ( data.length() - buffer_[index].length() );
    buffer_[index] = data;
  }
}

void Reassembler::selective_commit( uint64_t index, const string& data )
{
  auto num_bytes_to_commit = index + data.length() - output_.writer().bytes_pushed();
  auto data_to_commit = data.substr( data.length() - num_bytes_to_commit, num_bytes_to_commit );

  output_.writer().push( data_to_commit );
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( data.length() == 0 || is_duplicate( first_index, data ) )
    return;

  if ( is_out_of_order ) {
    store( first_index, data );
    return;
  }

  if ( is_overlapping( first_index, data ) ) {
    selective_commit( first_index, data );
  } else {
    output_.writer().push( data );
  }

  // check all stored data to see if we can insert it at the updated position in the stream
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_pending_;
}
