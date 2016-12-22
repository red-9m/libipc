#ifndef _LIB_IPC_H_
#define _LIB_IPC_H_

enum ChannelType
{
    ctServer,
    ctClient
};

extern int mk_channel(const char* chName, enum ChannelType chType);
extern int rm_channel(int chId);

extern const char* read_message(int chId);
extern int write_message(int chId, const char* msgText);

#endif
