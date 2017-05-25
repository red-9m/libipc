#ifndef _LIB_IPC_H_
#define _LIB_IPC_H_

// Create new listening channel and open it for read. Return: non-negative - channel descriptor; negative - error
extern int create_channel(const char* chName);

// Open existing channel (created with create_channel() on another side) for write. Return: non-negative - channel descriptor; negative - error
extern int connect_channel(const char* chName);

// Close channel descriptor. Return: zero - descriptor closed; negative - error
extern int close_channel(int chId);

// Read message from the channel. Returns only when got a message. Return: non-null - message; null - error
extern const char* read_message(int chId);

// Write message to the channel and exit immediately. Return: non-negative - number of bytes written; negative - error
extern int write_message(int chId, const char* msgText);

#endif
