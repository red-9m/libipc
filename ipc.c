#include <fcntl.h>       // fcntl()
#include <unistd.h>      // read() / write()
#include <sys/stat.h>    // mkfifo()
#include <string.h>      // strncpy() / strlen()
#include <stdlib.h>      // malloc() / free()
#include <sys/socket.h>  // socket() / bind() / listen() / connect() / send()
#include <sys/un.h>      // sockaddr_un
#include <errno.h>       // errno

#include "ipc.h"


// === Private defines =========================================================

#define MSG_BUFF_SIZE (64 * 1024)
#define MAX_CHANNELS 128
#define MAX_LISTENERS 1


// === Internal channel structure ==============================================

struct ipc_channel
{
    char* mName;
    int mCreated;
    int mFileHdl;
    int mListener;
    char* mMsgBuff;
    int mMsgBuffSize;
    char* mMsgBuffPtr;
    enum ipc_msg mMsgType;
    enum ipc_transport mTransport;
    enum ipc_block mBlock;
};

static struct ipc_channel g_channels[MAX_CHANNELS];


// === Private functions =======================================================

static int _find_free_channel()
{
    int result = -1;
    int i;
    for (i = 0; i < MAX_CHANNELS; i++)
    {
        if (g_channels[i].mMsgType == ipcFree)
        {
            result = i;
            break;
        }
    }

    return result;
}

static const char *_message_from_buff(struct ipc_channel *ch)
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

static void _sock_set_block_mode(int fd, enum ipc_block blockMode)
{
    int flags = fcntl(fd, F_GETFL, 0);
    flags = (blockMode == ipcBlock) ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    fcntl(fd, F_SETFL, flags);
}

static int _read_transport(struct ipc_channel *ch, void *buffer, size_t size)
{
    int result = IPC_SOCKERR;

    if ((ch->mTransport == ipcSock) && (ch->mFileHdl == -1))
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

static int _open_sock(const char* chName, enum ipc_msg msgType, int createMode, enum ipc_block blockMode)
{
    int ch_id, fd, name_len;
    struct sockaddr_un addr;
    struct ipc_channel *ch = NULL;
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

            if (createMode)
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
                _sock_set_block_mode(fd, blockMode);

                ch->mName = malloc(name_len);
                strncpy(ch->mName, chName, name_len);
                ch->mMsgBuff = malloc(MSG_BUFF_SIZE + 64);
                ch->mCreated = createMode;
                if (createMode)
                    ch->mFileHdl = -1;
                else
                    ch->mFileHdl = fd;
                ch->mListener = fd;
                ch->mMsgType = msgType;
                ch->mTransport = ipcSock;
                ch->mBlock = blockMode;
                result = ch_id;
            } else
                result = -errno;
        } else
            result = -errno;
    }

    return result;
}

static int _open_fifo(const char* chName, enum ipc_msg msgType, int createMode, enum ipc_block blockMode)
{
    int ch_id;
    int flags = 0;
    int result = IPC_NOFREE;
    struct ipc_channel *ch = NULL;
    int name_len = 0;
    int is_nonblock = 0;

    ch_id = _find_free_channel();
    if (ch_id >= 0)
    {
        ch = &g_channels[ch_id];

        if (blockMode == ipcNonblock)
            is_nonblock = O_NONBLOCK;

        if (createMode)
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
            if ((msgType == ipcMessage) && (createMode))
                ch->mMsgBuff = malloc(MSG_BUFF_SIZE + 64);
            else
                ch->mMsgBuff = NULL;
            strncpy(ch->mName, chName, name_len);
            ch->mCreated = createMode;
            ch->mFileHdl = result;
            ch->mListener = -1;
            ch->mMsgType = msgType;
            ch->mTransport = ipcFifo;
            ch->mBlock = blockMode;
            result = ch_id;
        } else
            result = -errno;
    }

    return result;
}

static int _write_object(int chId, const void* obj, int objSize, enum ipc_msg msgType)
{
    int result = IPC_DSCERR;

    if (chId >= 0)
    {
        struct ipc_channel *ch = &g_channels[chId];
        result = IPC_TYPEERR;

        if (ch->mMsgType == msgType)
        {
            result = IPC_SOCKERR;
            if (ch->mFileHdl != -1)
            {
                if (ch->mTransport == ipcSock)
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


// === Public functions ========================================================

int ipc_create_channel(const char* chName, enum ipc_transport transportType, enum ipc_msg msgType, enum ipc_block blockMode)
{
    const int CREATE_MODE = 1;

    if (transportType == ipcFifo)
        return _open_fifo(chName, msgType, CREATE_MODE, blockMode);
    else
        return _open_sock(chName, msgType, CREATE_MODE, blockMode);
}

int ipc_connect_channel(const char* chName, enum ipc_transport transportType, enum ipc_msg msgType, enum ipc_block blockMode)
{
    const int OPEN_MODE = 0;

    if (transportType == ipcFifo)
        return _open_fifo(chName, msgType, OPEN_MODE, blockMode);
    else
        return _open_sock(chName, msgType, OPEN_MODE, blockMode);
}

int ipc_close_channel(int chId)
{
    int result = IPC_DSCERR;

    if (chId >= 0)
    {
        result = 0;
        struct ipc_channel *ch = &g_channels[chId];

        if (ch->mMsgType != ipcFree)
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
            ch->mMsgType = ipcFree;
        }
        ch->mName = NULL;
        ch->mMsgBuff = NULL;
        ch->mMsgBuffPtr = NULL;
    }

    return result;
}

int ipc_write_message(int chId, const char* msgText)
{
    return _write_object(chId, msgText, strlen(msgText) + 1, ipcMessage);
}

const char* ipc_read_message(int chId)
{
    const char *result = NULL;

    if (chId >= 0)
    {
        struct ipc_channel *ch = &g_channels[chId];

        if (ch->mMsgType == ipcMessage)
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

int ipc_write_object(int chId, const void* obj, int objSize)
{
    return _write_object(chId, obj, objSize, ipcObject);
}

int ipc_read_object(int chId, void* obj, int objSize)
{
    int result = IPC_DSCERR;

    if (chId >= 0)
    {
        struct ipc_channel *ch = &g_channels[chId];

        result = IPC_TYPEERR;
        if (ch->mMsgType == ipcObject)
        {
            result = _read_transport(ch, obj, objSize);

            if ((result > 0) && (objSize != result))
                result = IPC_OBJERR;
        }
    }

    return result;
}
