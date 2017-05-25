#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
//#include <stdio.h>

#include "ipc.h"

#define MSG_BUFF_SIZE (64 * 1024)
#define MAX_CHANNELS 64
#define FREE_CHANNEL 0

struct Channel
{
    char *mName;
    int mCreated;
    int mFileHdl;
    char *mMsgBuff;
    int mMsgBuffSize;
    char *mMsgBuffPtr;
};

static struct Channel g_channels[MAX_CHANNELS];

static int _find_free_channel()
{
    int result = -1;
    int i;
    for (i = 0; i < MAX_CHANNELS; i++)
    {
        if (g_channels[i].mFileHdl == FREE_CHANNEL)
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

static int _open_channel(const char* chName, int chCreate)
{
    int result = -1;
    int file_hdl = 0;
    struct Channel *ch = NULL;
    int ch_id;
    int name_len = 0;

    ch_id = _find_free_channel();
    if (ch_id >= 0)
    {
        ch = &g_channels[ch_id];

        if (chCreate)
        {
            mkfifo(chName, S_IRWXU);
            file_hdl = open(chName, O_RDWR);
        } else
        {
            file_hdl = open(chName, O_WRONLY | O_NONBLOCK);
        }

        if (file_hdl > 0)
        {
            name_len = strlen(chName);
            ch->mName = malloc(name_len);
            ch->mMsgBuff = malloc(MSG_BUFF_SIZE + 64);
            strncpy(ch->mName, chName, name_len);
            ch->mCreated = chCreate;
            ch->mFileHdl = file_hdl;
            result = ch_id;
        } else
        {
            result = -2;
        }
    }
    return result;
}

int create_channel(const char* chName)
{
    return _open_channel(chName, 1);
}

int connect_channel(const char* chName)
{
    return _open_channel(chName, 0);
}

int close_channel(int chId)
{
    int result = -1;

    if (chId >= 0)
    {
        struct Channel *ch = NULL;

        result = 0;
        ch = &g_channels[chId];
        if (ch->mFileHdl != FREE_CHANNEL)
        {
            close(ch->mFileHdl);
            if (ch->mCreated)
                unlink(ch->mName);
            free(ch->mName);
            free(ch->mMsgBuff);
            ch->mFileHdl = FREE_CHANNEL;
        }
        ch->mName = NULL;
        ch->mMsgBuff = NULL;
        ch->mMsgBuffPtr = NULL;
    }

    return result;
}

const char* read_message(int chId)
{
    const char *result = NULL;

    if (chId >= 0)
    {
        struct Channel *ch = NULL;

        ch = &g_channels[chId];

        result = _message_from_buff(ch);
        if (!result)
        {
            ch->mMsgBuffSize = read(ch->mFileHdl, ch->mMsgBuff, MSG_BUFF_SIZE);
            ch->mMsgBuffPtr = ch->mMsgBuff;
            ch->mMsgBuff[ch->mMsgBuffSize] = '\0';
            result = _message_from_buff(ch);
        }
    }

    return result;
}

int write_message(int chId, const char* msgText)
{
    int result = -1;
    if (chId >= 0)
    {
        struct Channel *ch = NULL;

        ch = &g_channels[chId];
        result = write(ch->mFileHdl, msgText, strlen(msgText) + 1);
    }

    return result;
}
