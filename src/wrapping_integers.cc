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
  ///////////////////////// 4294967296 wraps to zero!!!
  ///////////////////////// 4294967295 is the last byte before the wrap.
  ///////////////////////// UINT32_max = 4294967295
  ///////////////////////// 1UL << 32 = 4294967296

  uint32_t n = this->raw_value_;
  uint32_t isn = zero_point.raw_value_;
  uint32_t num_wraps = checkpoint / ( UINT32_MAX );

  if ( num_wraps == 0 ) {
    return n - isn;
  }

  // if ( n > isn ) {
  //   return ( num_wraps * Wrap32::largest_32_bit_integer ) + n - isn;
  // }

  return (uint64_t)( ( num_wraps * INT32_MAX ) + min_dist_from_n_to_checkpoint( Wrap32( n ), Wrap32( checkpoint ) )
                     - isn );
}

int64_t Wrap32::min_dist_from_n_to_checkpoint( const Wrap32& n, const Wrap32& checkpoint )
{
  uint32_t a = n.raw_value_ - checkpoint.raw_value_;
  uint32_t b = checkpoint.raw_value_ - n.raw_value_;

  if ( a < b ) {
    return (int64_t)a;
  } else {
    return (int64_t)b * -1;
  }
}
