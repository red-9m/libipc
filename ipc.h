#ifndef _LIB_IPC_H_
#define _LIB_IPC_H_

extern int create_channel(const char* chName);
extern int connect_channel(const char* chName);
extern int close_channel(int chId);

extern const char* read_message(int chId);
extern int write_message(int chId, const char* msgText);

#endif
