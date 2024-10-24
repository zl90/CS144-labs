# Stanford CS-144 Networking Labs

I'm taking this class to learn how to build my own TCP/IP stack, something I've been interested in for a long time. There's no better way to learn how something works by building it yourself.

I will be following the CS144 labs to build a TCP/IP implementation from "outside-in", starting at the endpoints of communication where user applications read messages from a byte stream, then working deeper into the stack and eventually parsing TCP/IP headers, implementing the ARP protocol, etc.

## Lab 0 - Reliable byte streams

View my video recording here: https://youtu.be/KSQNDu8et-s?si=4TweTYnHm9zyh0wY

For this lab I built a ByteStream object, which is essentially just a FIFO queue that applications can read bytes from. To make sure the applications will be reading from a coherent stream, the bytes will be taken from the TCP packet and reassembled (in-order) in future labs.

## Lab 1 - The stream reassembler

In this lab I built the StreamReassembler object, which takes bytes from the payloads of TCP packets, reassembles them in-order (because they can arrive out of order, or not at all), then writes them to the ByteStream for consumption by applications.

The Reassembler is robust, as it can handle overlapping, missing and duplicate packets.
