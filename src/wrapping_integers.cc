#include "wrapping_integers.hh"
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
  uint32_t num_wraps = checkpoint / ( UINT32_MAX );

  if ( num_wraps == 0 ) {
    return n - isn;
  }

  if ( n > isn ) {
    return ( num_wraps * Wrap32::largest_32_bit_integer ) + n - isn;
  }

  return ( num_wraps * Wrap32::largest_32_bit_integer ) - n - isn;
}
