# libipc ver. 0.0.3a
Simple and productive IPC library written on pure C. Library is based on Unix named pipes. It avoids unnecessary memory copy to provide maximum performance.

Communication is based on unidirectional channels. One side creates a channel and could receive messages from multiple clients through it. Other side connects to a channel and send messages.

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
Maximum number of channels - 64  
Maximum messages in a channel - 64Kb (if you try to produce more messages, but 64k is not consumed, write_message() will return an error)  
Multi-thread - create_channel(), connect_channel() and close_channel() must be protected by mutexes
