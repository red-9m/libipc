# libipc ver. 0.0.2a
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
