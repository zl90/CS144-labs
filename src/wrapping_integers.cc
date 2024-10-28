#include "wrapping_integers.hh"
#include <cmath>
#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  Wrap32 result( zero_point + (int32_t)( n % Wrap32::largest_32_bit_integer ) );
  return result;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t n = this->raw_value_;
  uint32_t isn = zero_point.raw_value_;

  if ( checkpoint <= UINT32_MAX ) {
    uint64_t a = (uint32_t)( n - isn );
    uint64_t b = n + (uint64_t)Wrap32::largest_32_bit_integer - isn;

    if ( abs( static_cast<int64_t>( checkpoint - a ) ) > abs( static_cast<int64_t>( checkpoint - b ) ) ) {
      return b;
    } else {
      return a;
    }
  }

  return (uint64_t)( checkpoint + min_dist_from_n_to_checkpoint( Wrap32( n ), checkpoint ) - isn );
}

int64_t Wrap32::min_dist_from_n_to_checkpoint( const Wrap32& n, uint64_t checkpoint )
{
  Wrap32 wrapped_checkpoint = Wrap32( checkpoint );
  uint32_t to_add, to_subtract;

  if ( wrapped_checkpoint.raw_value_ > n.raw_value_ ) {
    to_add = n.raw_value_ + Wrap32::largest_32_bit_integer - wrapped_checkpoint.raw_value_;
    to_subtract = wrapped_checkpoint.raw_value_ - n.raw_value_;
  } else {
    to_add = n.raw_value_ - wrapped_checkpoint.raw_value_;
    to_subtract = wrapped_checkpoint.raw_value_ + Wrap32::largest_32_bit_integer - n.raw_value_;
  }

  return to_add < to_subtract ? (int64_t)to_add : (int64_t)to_subtract * -1;
}
