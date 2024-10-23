#pragma once

#include "byte_stream.hh"
#include <unordered_map>

class Reassembler
{
public:
  // Construct Reassembler to write into given ByteStream.
  explicit Reassembler( ByteStream&& output ) : output_( std::move( output ) ) {}

  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

  // Access output stream reader
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // Access output stream writer, but const-only (can't write from outside)
  const Writer& writer() const { return output_.writer(); }

private:
  /** @desc Whether the data overlaps bytes that have already been committed to the stream */
  bool is_overlapping( uint64_t index, const std::string& data );

  /** @desc Whether the data has already been committed to the stream */
  bool is_duplicate( uint64_t index, const std::string& data );

  /** @desc Whether the data is unable to be committed to the stream due to missing preceding bytes */
  bool is_out_of_order( uint64_t index );

  /** @desc Whether the data can be committed to the stream */
  bool is_in_order( uint64_t index );

  /** @desc Attempts to buffer the data in the `buffer_`. This will only overwrite an existing entry with the key of
   * `index` if the `data` is longer than the existing entry.
   *
   * Eg: If there is an existing entry at buffer_[3] with the value of `abc`, attempting to store `ab` at that entry
   will fail, but storing `abcd` will succeed. */
  void store( uint64_t index, const std::string& data, bool is_last_substring );

  /** @desc In the event of an overlapping substring, commits to the stream only the bytes that haven't already been
  committed. */
  void partial_insert( uint64_t index, const std::string& data );

  bool attempt_insert( uint64_t index, const std::string& data, bool is_last_substring );

  /** @desc Checks to see if the buffer is currently holding the next valid substring. If so, it commits the next
   * valid substring to the stream. */
  bool attempt_insert_from_buffer();

  /** @desc Closes the stream and clears the buffer. */
  void close();

  /** @desc Merges any substrings in the `buffer_` that overlap with the specified `index`. */
  void merge_overlapping_unassembled_substrings( uint64_t index );

  ByteStream output_; // the Reassembler writes to this ByteStream
  uint64_t bytes_pending_ = 0;
  std::unordered_map<uint64_t, std::pair<std::string, bool>> buffer_
    = {}; // holds pending/out-of-order substrings, as well as their is_last_substring value. Key/value:
          // index/{substring, is_last_substring}.
};
