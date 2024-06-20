### Task 1:

Created basic connection to web server (using `telnet`).

### Task 2:

Listen on a port for a basic tcp connection (`netcat` and `telnet`) (communicate over local network). Had no idea it was so easy to send text over a local network like this!

### Task 3:

**Start by reading the public interfaces inside `socket.hh` and `file_descriptor.hh`**

- These are a C++ wrappers for the C standard library networking library methods: `<sys/socket.h>`.
- `FileDescriptor` class header. Note: a socket is technically a file descriptor (like cout or a regular file stream handle).
  - Contains `FDWrapper` class: used as a handle to a kernel file descriptor.
  - I guess file descriptors are managed by the kernel? -> Yes. File descriptors are stored in an index (per process) and point to data structures that contain metadata about each open file. See https://stackoverflow.com/questions/5256599/what-are-file-descriptors-explained-in-simple-terms.
  - `close()` wrapper function calls the `close` unix system call (`man2 close`).
  - Interesting that we store the `EOF` state of the file descriptor in our own program, is this not queryable? Or is the `EOF` state of a file descriptor completely separate for each program reading it? I guess two programs can read the same file.
  - We store the amount of times the file descriptor has been read and written to.
- Learn about `bind` (`man2 bind`).
- Learned a little about the socket lifecycle. Socket is created with socket command, it exists in a name space (address family) but has no address assigned to it. bind() assigns an address to the socket file descriptor. This operation is called "assigning a name to a socket". Normally you need to assign a local address to a socket using bind() before a SOCK_STREAM socket can receive connections.
- Learned about binding sockets to devices (devices are NICs or virtual network interfaces like `eth0`, `wlan0`, `lb`, etc).
- Learned about `connect` (man2 connect). -> Connect a socket to a socket address, typically on another machine, and it must be set up as a server.
- Interesting classes in the `socket.hh` file:
  - `PacketSocket`: wrapper for packet sockets - used to receive or send raw packets at the device driver (OSI Layer 2) level.
  - `LocalStreamSocket` and `LocalDatagramSocket`: wrapper for Unix-domain sockets - used for inter-process communication (exchanging data between processes executing on the same host operating system) (uses address family `AF_UNIX`).
  - Obviously `UDPSocket` and `TCPSocket`: wrappers for UDP and TCP transport layer sockets.
- Learned the difference between the `AF_UNIX` and `AF_INET` address families (Unix domain sockets being used for IPC and Internet sockets being used for inter-device communication).
- Great post by Stefano Sanfilippo:
  > If you want to communicate with a remote host, then you will probably need an INET socket. The difference is that an INET socket is bound to an IP address-port tuple, while a UNIX socket is "bound" to a special file on your filesystem. Generally, only processes running on the same machine can communicate through the latter. So, why would one use a UNIX socket? Exactly for the reason above: communication between processes on the same host, being a lightweight alternative to an INET socket via loopback. In fact, INET sockets sit at the top of a full TCP/IP stack, with traffic congestion algorithms, backoffs and the like to handle. A UNIX socket doesn't have to deal with any of those problems, since everything is designed to be local to the machine, so its code is much simpler and the communication is faster. Granted, you will probably notice the difference only under heavy load, e.g. when reverse proxying an application server (Node.js, Tornado...) behind Nginx etc.

### Task 4: writing `webget`

- Need to implement the `get_URL()` function inside `webget.cc` file.
- But what do we need to do this?
  - Need a `TCPSocket`.
  - Need to bind a socket and connect the socket to the address given.
    - Do I need to bind to a NIC?
    - Do I need to somehow get the host/path as an IP Address? -> Yes. Use the constructor of the `Address` class. It can resolve an IP address from a hostname and service name.
      - What is a service name?? Inspecting the `Address` constructor implementation I find this comment: ` //! \param[in] service name (from ``/etc/services``, e.g., "http" is port 80) `.
      - `ls`ing this directory reveals: a file containing a bunch of service names and their port mappings. All defined by a company called the [Internet Assigned Numbers Authority](https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml). Cool.
      - So basically we need to use the `http` service name here -> that's the port I would expect to find a web page.
  - How do I get a local address to bind the socket to?
    - Construct `Address` object from a local address string eg: "127.0.0.1" and a port number? I chose port 8080.
    - Not sure what the sockaddr is -> will come back to this.
    - Turns out I didn't need to bind the port since binding is only for servers to listen/accept on a port, not for clients.
  - Now that I've binded the socket to the local address I need to connect to the host address.
    - Use the `connect()` function with the host address.
  - Need to send a GET request using HTTP/1.1.
    - Now that I'm connected, how do I send data? Can I just write to a socket as if it was an output stream? No -> use the `write` function defined in the `FileDescriptor` class. Remember a socket is a file descriptor at its core.
    - Need to send a string containing my HTTP request.
  - Need to read ALL data returned from the host (use a loop).
    - Use the `read` method from `Socket`.
    - Use the `eof` method from `Socket` to keep reading up until the stream ends.
  - First compilation: builds, but doesn't run. `connect: Invalid argument`.
    - Realised what the problem was: the client `TCPSocket` should not be `bind`ing to a local address. Binding is used mainly for servers to listen/accept new connections.

### Task 5: writing an in-memory reliable byte stream

- The Transmission Control Protocol is arguably the most prevalent computer program on the planet.
- Need to implement the methods in `byte_stream.cc`.
- Looks like everything should be done through the `ByteStream` interface, including access to the `Reader` and `Writer` classes.
- Some member variables are missing:
  - Need a `bytes_popped` and `bytes_pushed` to keep track of how many cumulative bytes have been pushed into the stream and popped from the stream.
  - Need a buffer of size `capacity_` stored on the stream itself. This should be implemented as a `std::queue` -> don't need to initialise the queue with a size, just limit it's size later.
  - Need booleans to store when the stream has been "closed" (signal from the `Writer`) and "finished" (signal from the `Reader` where the stream is closed and fully popped).
  - Need to store `reader_` and `writer_` data members too. Should these be `unique_ptr`s?
    - Also need to think about constructors, destructors and assignment operators. To keep things simple I don't want them to be movable or copyable.
    - Destructors: these have nothing to destruct. But the `ByteStream` class will. If I use `unique_ptr`s I shouldn't need to worry about this actually.
- How should error handling be done? The implementation suggests it should be handled.
- We are given a handy `read()` helper function that lets us peek and pop from the stream into a string. It takes a "length" argument. This will obviously come in handy inside the `Reader` class.
- Also need to figure out how to only write the appropriate amount of bytes to the stream (ie: just enough to fill the stream, but not so much that it exceeds its capacity).
  - Problem is: what do we do with the leftover data once we've pushed all we can to the queue? Obviously we need to wait until more space in the queue is free, but doing that means we have to store the remaining data in another buffer.
  - Re-reading the spec: it looks handling this is not the job of the stream. The consumer of the `ByteStream` is the one who needs to do Flow Control. From what I can see, we should signal an error if the stream gets overloaded or is read from too many times.
- How will the `writer_` and `reader_` members access the queue? Will they need to made a `friend` of the `ByteStream` class? -> This is handled in `byte_stream_helpers.cc` -> the `ByteStream` instance is just downcasted to a `Reader` or `Writer` type, so it will always have access to the queue.
- `peek()` method: why does it require a `string_view` and how do I convert a `char` to a `string_view`? -> Solution: I learned that a typical packet payload is ~100 bytes, enough to store an average line of HTML code, therefore I can store `string`s inside my queue, then easily convert that to a `string_view` in the `peek()` method. -> Realised this was a mistake because it won't easily let me calculate `bytes_buffered()`. `char`s it is.
- `pop()` function is interesting because it seems pointless since we have the `read()` helper function, all the hard work is done for us. I just need to wrap the `read()` function by the looks of it. Not sure if there's a requirement to do anything with the `out` string like outputting it?
- Decided to add guard clauses to the `push()` and `pop()` methods.
