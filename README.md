# libipc ver. 0.0.4a
Simple lightweight and productive zero-copy IPC library. Written on pure C, based on Unix named pipes. Library is intended to exchange text messages and objects between processes.

Communication is based on unidirectional channels. One side creates a channel and could receive messages from multiple clients through it. Other side connects to a channel and send messages. You also able to create object channel that allow you to send/receive structures and other objects. Structure no need serialization to be send.

# Features
* Fast: 10K messages (~250K of data) written for 0.009s on i5-4250U CPU  
* Lite: 12k .so footprint
* Zero-copy: No overhead over Unix named pipes, no memcpy() / strcpy() performed for your provided object / message buffer  
* No messages are dropped on overflow: if you receive a success from ipc_write_message() you guaranteed that consumer could receive the message by ipc_read_message() at any time

# Configure:
cmake .

# Compile:
make

# Compile samples:
make samples

# Run client/server samples:
> ./sample-server &  
> ./sample-client client1  
> ./sample-client client2

# Run hello sample
> ./sample-hello

# Limitations
* One channel - one communication direction
* Communication within single machine only  
* Maximum number of channels - 64  
* Maximum not consumed data could remain in a channel - 64Kb (if you try to produce more messages, but 64k is not consumed, ipc_write_message() will return an error)  
* Multi-thread - ipc_create_channel(), ipc_connect_channel() and ipc_close_channel() must be protected by mutexes
