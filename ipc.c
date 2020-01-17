#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "ipc.h"

#define MSG_BUFF_SIZE (64 * 1024)
#define MAX_CHANNELS 64
#define MAX_LISTENERS 1
#define FREE_CHANNEL 0

struct Channel
{
    char* mName;
    int   mCreated;
    int   mFileHdl;
    int   mListener;
    char* mMsgBuff;
    int   mMsgBuffSize;
    char* mMsgBuffPtr;
    enum  ipc_type mType;
    enum  ipc_transport mTransport;
    enum  ipc_operation mBlock;
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

static void _sock_set_block_mode(int fd, enum ipc_operation chBlock)
{
    int flags = fcntl(fd, F_GETFL, 0);
    flags = (chBlock == opBlock) ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    fcntl(fd, F_SETFL, flags);
}

static int _read_transport(struct Channel *ch, void *buffer, size_t size)
{
    int result = IPC_SOCKERR;

    if ((ch->mTransport == trSock) && (ch->mFileHdl == -1))
    {
        ch->mFileHdl = accept(ch->mListener, NULL, NULL);
        _sock_set_block_mode(ch->mFileHdl, ch->mBlock);
    }

    if (ch->mFileHdl != -1)
    {
        result = read(ch->mFileHdl, buffer, size);

        if (result == 0)
        {
            result = IPC_SOCKERR;
            close(ch->mFileHdl);
            ch->mFileHdl = -1;
        } else if (result == -1)
            result = -errno;
    }

    return result;
}

static int _open_channel_sock(const char* chName, enum ipc_type chType, int chCreate, enum ipc_operation chBlock)
{
    int ch_id, fd, name_len;
    struct sockaddr_un addr;
    struct Channel *ch = NULL;
    int result = IPC_NOFREE;

    ch_id = _find_free_channel();
    if (ch_id >= 0)
    {
        ch = &g_channels[ch_id];
        fd = socket(PF_LOCAL, SOCK_STREAM, 0);

        if (fd != -1)
        {
            name_len = strlen(chName);
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_LOCAL;
            strncpy(addr.sun_path, chName, 107);

            if (chCreate)
            {
                unlink(chName);
                result = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
                if (result == 0)
                    result = listen(fd, MAX_LISTENERS);
            }
            else
                result = connect(fd, (struct sockaddr*)&addr, sizeof(addr));

            if (result == 0)
            {
                _sock_set_block_mode(fd, chBlock);

                ch->mName = malloc(name_len);
                strncpy(ch->mName, chName, name_len);
                ch->mMsgBuff = malloc(MSG_BUFF_SIZE + 64);
                ch->mCreated = chCreate;
                if (chCreate)
                    ch->mFileHdl = -1;
                else
                    ch->mFileHdl = fd;
                ch->mListener = fd;
                ch->mType = chType;
                ch->mTransport = trSock;
                ch->mBlock = chBlock;
                result = ch_id;
            } else
                result = -errno;
        } else
            result = -errno;
    }

    return result;
}

static int _open_channel_fifo(const char* chName, enum ipc_type chType, int chCreate, enum ipc_operation chBlock)
{
    int ch_id;
    int flags = 0;
    int result = IPC_NOFREE;
    struct Channel *ch = NULL;
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
            unlink(chName);
            mkfifo(chName, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            flags = O_RDWR;
        } else
            flags = O_WRONLY;

        result = open(chName, flags | is_nonblock);

        if (result != -1)
        {
            name_len = strlen(chName);
            ch->mName = malloc(name_len);
            if ((chType == tpMessage) && (chCreate == 1))
                ch->mMsgBuff = malloc(MSG_BUFF_SIZE + 64);
            else
                ch->mMsgBuff = NULL;
            strncpy(ch->mName, chName, name_len);
            ch->mCreated = chCreate;
            ch->mFileHdl = result;
            ch->mListener = -1;
            ch->mType = chType;
            ch->mTransport = trFifo;
            ch->mBlock = chBlock;
            result = ch_id;
        } else
            result = -errno;
    }

    return result;
}

int ipc_create_channel(const char* chName, enum ipc_transport chTransport, enum ipc_type chType, enum ipc_operation chBlock)
{
    if (chTransport == trFifo)
        return _open_channel_fifo(chName, chType, 1, chBlock);
    else
        return _open_channel_sock(chName, chType, 1, chBlock);
}

int ipc_connect_channel(const char* chName, enum ipc_transport chTransport, enum ipc_type chType, enum ipc_operation chBlock)
{
    if (chTransport == trFifo)
        return _open_channel_fifo(chName, chType, 0, chBlock);
    else
        return _open_channel_sock(chName, chType, 0, chBlock);
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
            if (ch->mFileHdl != -1)
                close(ch->mFileHdl);
            if (ch->mCreated)
                unlink(ch->mName);
            if (ch->mListener != -1)
                close(ch->mListener);
            free(ch->mName);
            free(ch->mMsgBuff);
            ch->mFileHdl = -1;
            ch->mListener = -1;
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
        {
            result = IPC_SOCKERR;
            if (ch->mFileHdl != -1)
            {
                if (ch->mTransport == trSock)
                    result = send(ch->mFileHdl, obj, objSize, MSG_NOSIGNAL);
                else
                    result = write(ch->mFileHdl, obj, objSize);

                if (result == -1)
                {
                    result = -errno;
                    if (result == -EPIPE)
                    {
                        result = IPC_SOCKERR;
                        close(ch->mFileHdl);
                        ch->mFileHdl = -1;
                    }
                }
            }
        }
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
                ch->mMsgBuffSize = _read_transport(ch, ch->mMsgBuff, MSG_BUFF_SIZE);

                if (ch->mMsgBuffSize > 0)
                {
                    ch->mMsgBuffPtr = ch->mMsgBuff;
                    ch->mMsgBuff[ch->mMsgBuffSize] = '\0';
                    result = _message_from_buff(ch);
                }
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
            result = _read_transport(ch, obj, objSize);

            if ((result > 0) && (objSize != result))
                result = IPC_OBJERR;
        }
    }

    return result;
}
