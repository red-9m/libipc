#ifndef _LIB_IPC_H_
#define _LIB_IPC_H_

#define IPC_NOFREE  -1 // No free channels available
#define IPC_OPENERR -2 // Error open channel
#define IPC_DSCERR  -3 // Not valid descriptor
#define IPC_OBJERR  -4 // Incorrect object size
#define IPC_TYPEERR -5 // Incorrect channel type

enum ipc_type
{
    tpMessage = 1,
    tpObject = 2
};

// Create new listening channel and open it for read
// Return: non-negative - channel descriptor; negative - error
extern int ipc_create_channel(const char* chName, enum ipc_type chType);

// Open existing channel (created with create_channel() on another side) for write
// Return: non-negative - channel descriptor; negative - error
extern int ipc_connect_channel(const char* chName, enum ipc_type chType);

// Close channel by descriptor
// Return: zero - channel closed; negative - error
extern int ipc_close_channel(int chId);

// Write message to the channel and exit immediately
// Return: non-negative - number of bytes written; negative - error
extern int ipc_write_message(int chId, const char* msgText);

// Read message from the channel. Returns only when got a message
// Return: non-null - message; null - error
extern const char* ipc_read_message(int chId);

// Write object to the channel and exit immediately
// Return: non-negative - number of bytes written; negative - error
extern int ipc_write_object(int chId, const void* obj, int objSize);

// Read object from the channel. Returns only when got a message
// Return: non-negative - ok; negative - error
extern int ipc_read_object(int chId, void* obj, int objSize);

#endif
