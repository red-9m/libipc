# libipc ver. 0.0.8
Simple lightweight and productive zero-copy IPC library. Written on pure C, based on Unix Named Pipes and Unix Domain Sockets. Library is intended to exchange text messages and structures/objects between processes

# Transport
* Unix Named pipes: One-to-many unidirectional channel. One side creates a channel and could receive messages from multiple clients through it. Other side connects to a channel and send messages. You also able to create object channel that allow you to send/receive structures and other objects
* Unix Domain Sockets: One-to-one biderectional channel

# Features
* Fast: 10K messages (~250K of data) written for 0.009s on i5-4250U CPU
* Lite: 12k .so size
* Zero-copy: No overhead over Unix transport, no memcpy() / strcpy() performed for your provided object / message buffer
* No messages are dropped on overflow: if you receive a success from ipc_write_message() you guaranteed that consumer could receive the message by ipc_read_message()

# Configure as shared library:
cmake .

# Configure as static library:
cmake . -DBUILD_SHARED_LIBS=OFF

# Compile library:
cmake --build .

# Compile library and samples:
cmake --build . --target samples

# Run client/server samples:
> ./sample-server &  
> ./sample-client client1

# Run hello sample
> ./sample-hello

# Limitations
* Communication within single machine only
* Maximum number of channels - 64
* Maximum not consumed data could remain in a channel - 64Kb (if you try to produce more messages, but 64k is not consumed, ipc_write_message() will return an error)
* Multi-thread - ipc_create_channel(), ipc_connect_channel() and ipc_close_channel() must be protected by mutexes
