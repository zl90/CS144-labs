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

void Reassembler::store( uint64_t index, const string& data, bool is_last_substring )
{
  if ( buffer_.find( index ) == buffer_.end() ) {
    bytes_pending_ += data.length();
    buffer_[index].first = data;
    buffer_[index].second = is_last_substring;
  } else if ( data.length() > buffer_[index].first.length() ) {
    bytes_pending_ += ( data.length() - buffer_[index].first.length() );
    buffer_[index].first = data;
    buffer_[index].second = is_last_substring;
  }
}

void Reassembler::selective_commit( uint64_t index, const string& data )
{
  auto num_bytes_to_commit = index + data.length() - output_.writer().bytes_pushed();
  auto data_to_commit = data.substr( data.length() - num_bytes_to_commit, num_bytes_to_commit );

  output_.writer().push( data_to_commit );
}

bool Reassembler::attempt_insert( uint64_t index, const string& data, bool is_last_substring )
{
  if ( is_out_of_order( index ) ) {
    store( index, data, is_last_substring );
    return false;
  }

  if ( is_overlapping( index, data ) ) {
    selective_commit( index, data );
  } else {
    output_.writer().push( data );
  }

  if ( is_last_substring ) {
    buffer_.clear();
    output_.writer().close();
  }

  return true;
}

bool Reassembler::attempt_insert_next_substring()
{
  auto next_index = output_.writer().bytes_pushed();

  if ( buffer_.find( next_index ) != buffer_.end() ) {
    // found exact match in buffer, can commit to stream
    auto data = buffer_[next_index].first;
    output_.writer().push( data );
    bytes_pending_ -= data.length();
    buffer_.erase( next_index );
    return true;
  } else {
    // search for overlapping substrings in buffer
    for ( const auto& [curr_index, value] : buffer_ ) {
      if ( curr_index < next_index ) { // no need to search indexes >= to the next index
        auto data = value.first;
        auto is_last_substring = value.second;

        // discard this substring if it is a duplicate
        if ( is_duplicate( curr_index, data ) ) {
          bytes_pending_ -= data.length();
          buffer_.erase( curr_index );
          return false;
        }

        bool is_inserted = attempt_insert( curr_index, data, is_last_substring );
        if ( is_inserted ) {
          bytes_pending_ -= output_.writer().bytes_pushed() - next_index;
          buffer_.erase( curr_index );
          return true;
        }
      }
    }
  }

  return false;
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( is_duplicate( first_index, data ) )
    return;

  bool is_inserted = attempt_insert( first_index, data, is_last_substring );

  if ( is_inserted ) {
    bool is_next_substring_inserted = attempt_insert_next_substring();
    while ( is_next_substring_inserted ) {
      is_next_substring_inserted = attempt_insert_next_substring();
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_pending_;
}
