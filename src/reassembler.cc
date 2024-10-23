#include "reassembler.hh"
#include <cmath>

using namespace std;

bool Reassembler::is_overlapping( uint64_t index, const string& data )
{
  return output_.writer().bytes_pushed() > index && output_.writer().bytes_pushed() < index + data.length();
}

bool Reassembler::is_duplicate( uint64_t index, const string& data )
{
  return data.length() > 0 && output_.writer().bytes_pushed() >= index + data.length();
}

bool Reassembler::is_out_of_order( uint64_t index )
{
  return index > output_.writer().bytes_pushed();
}

bool Reassembler::is_in_order( uint64_t index )
{
  return output_.writer().bytes_pushed() == index;
}

struct Substring
{
  uint64_t index;
  string data;
  bool is_last_substring;
};

void Reassembler::merge_overlapping_unassembled_substrings( uint64_t index )
{
  vector<uint64_t> indexes_to_erase;
  vector<Substring> substrings_to_store;
  bool did_merge = false;
  uint64_t smallest_index;

  for ( const auto& [merge_index, merge_info] : buffer_ ) {
    if ( merge_index == index )
      continue;
    auto merge_data = merge_info.first;
    auto curr_start = index;
    auto curr_end = index + buffer_[index].first.length();
    auto merge_start = merge_index;
    auto merge_end = merge_index + merge_data.length();

    // Check general overlap
    if ( curr_start < merge_end && merge_start < curr_end ) {
      did_merge = true;
      smallest_index = min( curr_start, merge_start );
      // Check if one is engulfing the other
      if ( curr_start <= merge_start && curr_end >= merge_end ) {
        indexes_to_erase.push_back( merge_index );
      } else if ( merge_start <= curr_start && merge_end >= curr_end ) {
        indexes_to_erase.push_back( index );
      } else {
        // Handle a data merge
        auto second_smallest_index = max( curr_start, merge_start );
        string new_data = "";

        for ( uint64_t i = 0; i < second_smallest_index - smallest_index; i++ ) {
          new_data += buffer_[smallest_index].first[i];
        }
        for ( uint64_t i = 0; i < buffer_[second_smallest_index].first.length(); i++ ) {
          new_data += buffer_[second_smallest_index].first[i];
        }

        substrings_to_store.push_back(
          { smallest_index, new_data, buffer_[index].second || buffer_[merge_index].second } );
        indexes_to_erase.push_back( index );
        indexes_to_erase.push_back( merge_index );
      }
    }
  }

  for ( const auto& idx : indexes_to_erase ) {
    bytes_pending_ -= buffer_[idx].first.length();
    buffer_.erase( idx );
  }

  for ( const auto& substring : substrings_to_store ) {
    buffer_[substring.index] = { substring.data, substring.is_last_substring };
    bytes_pending_ += substring.data.length();
  }

  if ( did_merge ) {
    merge_overlapping_unassembled_substrings( smallest_index );
  }
}

void Reassembler::store( uint64_t index, const string& data, bool is_last_substring )
{
  auto capacity = output_.total_capacity();

  if ( data.length() == 0 || index >= capacity )
    return;

  // Ensure the data will not overflow the capacity of the stream
  string new_data = data;
  if ( index + data.length() > capacity ) {
    // Silently drop the overflowing bytes
    new_data = data.substr( 0, index + data.length() - capacity );
  }

  if ( buffer_.find( index ) == buffer_.end() ) {
    bytes_pending_ += new_data.length();
    buffer_[index].first = new_data;
    buffer_[index].second = new_data.length() == data.length() ? is_last_substring : false;
  } else if ( new_data.length() > buffer_[index].first.length() ) {
    bytes_pending_ += ( new_data.length() - buffer_[index].first.length() );
    buffer_[index].first = new_data;
    buffer_[index].second = new_data.length() == data.length() ? is_last_substring : false;
  }

  merge_overlapping_unassembled_substrings( index );
}

void Reassembler::close()
{

  buffer_.clear();
  output_.writer().close();
}

void Reassembler::partial_insert( uint64_t index, const string& data )
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
    partial_insert( index, data );
  } else {
    output_.writer().push( data );
  }

  if ( is_last_substring )
    close();

  return true;
}

bool Reassembler::attempt_insert_from_buffer()
{
  auto next_index = output_.writer().bytes_pushed();

  if ( buffer_.find( next_index ) != buffer_.end() ) {
    // found exact match in buffer, can commit to stream
    auto data = buffer_[next_index].first;
    auto is_last_substring = buffer_[next_index].second;
    output_.writer().push( data );
    bytes_pending_ -= data.length();
    buffer_.erase( next_index );
    if ( is_last_substring )
      close();
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
          bytes_pending_ -= output_.writer().bytes_pushed() - curr_index;
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
    bool is_next_substring_inserted = attempt_insert_from_buffer();
    while ( is_next_substring_inserted ) {
      is_next_substring_inserted = attempt_insert_from_buffer();
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_pending_;
}
