#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "ipc.h"

#define MSG_BUFF_SIZE (64 * 1024)
#define MAX_CHANNELS 64
#define FREE_CHANNEL 0

struct Channel
{
    char* mName;
    int   mCreated;
    int   mFileHdl;
    char* mMsgBuff;
    int   mMsgBuffSize;
    char* mMsgBuffPtr;
    enum  ipc_type mType;
};

static struct Channel g_channels[MAX_CHANNELS];

static int _find_free_channel()
{
    int result = -1;
    int i;
    for (i = 0; i < MAX_CHANNELS; i++)
    {
        if (g_channels[i].mType == FREE_CHANNEL)
        {
            result = i;
            break;
        }
    }
    return result;
}

static const char *_message_from_buff(struct Channel *ch)
{
    char *result = NULL;

    if ((ch->mMsgBuffSize > 0) && (ch->mMsgBuffPtr) && (ch->mMsgBuffPtr < (ch->mMsgBuff + ch->mMsgBuffSize)))
    {
        result = ch->mMsgBuffPtr;
        ch->mMsgBuffPtr = strchr(ch->mMsgBuffPtr, 0);
        if (ch->mMsgBuffPtr)
            ch->mMsgBuffPtr++;
    }

    return result;
}

static int _open_channel(const char* chName, enum ipc_type chType, int chCreate, enum ipc_operation chBlock)
{
    int result = IPC_NOFREE;
    int file_hdl = 0;
    struct Channel *ch = NULL;
    int ch_id;
    int name_len = 0;
    int is_nonblock = 0;

    ch_id = _find_free_channel();
    if (ch_id >= 0)
    {
        ch = &g_channels[ch_id];

        if (chBlock == opNonblock)
            is_nonblock = O_NONBLOCK;

        if (chCreate)
        {
            mkfifo(chName, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            file_hdl = open(chName, O_RDWR | is_nonblock);
        } else
        {
            file_hdl = open(chName, O_WRONLY | is_nonblock);
        }

        if (file_hdl > 0)
        {
            name_len = strlen(chName);
            ch->mName = malloc(name_len);
            if ((chType == tpMessage) && (chCreate == 1))
                ch->mMsgBuff = malloc(MSG_BUFF_SIZE + 64);
            else
                ch->mMsgBuff = NULL;
            strncpy(ch->mName, chName, name_len);
            ch->mCreated = chCreate;
            ch->mFileHdl = file_hdl;
            ch->mType = chType;
            result = ch_id;
        } else
        {
            result = IPC_OPENERR;
        }
    }
    return result;
}

int ipc_create_channel(const char* chName, enum ipc_type chType, enum ipc_operation chBlock)
{
    return _open_channel(chName, chType, 1, chBlock);
}

int ipc_connect_channel(const char* chName, enum ipc_type chType, enum ipc_operation chBlock)
{
    return _open_channel(chName, chType, 0, chBlock);
}

int ipc_close_channel(int chId)
{
    int result = IPC_DSCERR;

    if (chId >= 0)
    {
        result = 0;
        struct Channel *ch = &g_channels[chId];

        if (ch->mType != FREE_CHANNEL)
        {
            close(ch->mFileHdl);
            if (ch->mCreated)
                unlink(ch->mName);
            free(ch->mName);
            free(ch->mMsgBuff);
            ch->mFileHdl = 0;
            ch->mType = FREE_CHANNEL;
        }
        ch->mName = NULL;
        ch->mMsgBuff = NULL;
        ch->mMsgBuffPtr = NULL;
    }

    return result;
}

static int _write_object(int chId, const void* obj, int objSize, enum ipc_type sendType)
{
    int result = IPC_DSCERR;

    if (chId >= 0)
    {
        struct Channel *ch = &g_channels[chId];
        result = IPC_TYPEERR;
        if (ch->mType == sendType)
            result = write(ch->mFileHdl, obj, objSize);
    }

    return result;
}

int ipc_write_message(int chId, const char* msgText)
{
    return _write_object(chId, msgText, strlen(msgText) + 1, tpMessage);
}

int ipc_write_object(int chId, const void* obj, int objSize)
{
    return _write_object(chId, obj, objSize, tpObject);
}

const char* ipc_read_message(int chId)
{
    const char *result = NULL;

    if (chId >= 0)
    {
        struct Channel *ch = &g_channels[chId];

        if (ch->mType == tpMessage)
        {
            result = _message_from_buff(ch);
            if (!result)
            {
                ch->mMsgBuffSize = read(ch->mFileHdl, ch->mMsgBuff, MSG_BUFF_SIZE);
                ch->mMsgBuffPtr = ch->mMsgBuff;
                ch->mMsgBuff[ch->mMsgBuffSize] = '\0';
                result = _message_from_buff(ch);
            }
        }
    }

    return result;
}

int ipc_read_object(int chId, void* obj, int objSize)
{
    int result = IPC_DSCERR;

    if (chId >= 0)
    {
        struct Channel *ch = &g_channels[chId];

        result = IPC_TYPEERR;
        if (ch->mType == tpObject)
        {
            int read_bytes = read(ch->mFileHdl, obj, objSize);

            if (objSize == read_bytes)
                result = 0;
            else
                result = IPC_OBJERR;
        }
    }

    return result;
}
