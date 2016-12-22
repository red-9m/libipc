#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
//#include <stdio.h>

#include "ipc.h"

#define MSG_BUFF_SIZE (64 * 1024)
#define MAX_CHANNELS 1024
#define FREE_CHANNEL 0

struct Channel
{
    char *mName;
    enum ChannelType mType;
    int mFileHdl;
    char *mMsgBuff;
    int mMsgBuffSize;
    char *mMsgBuffPtr;
};

static struct Channel g_channels[MAX_CHANNELS];
static int g_first_channel = 1;

static void _clear_channels()
{
    int i;
    for (i = 0; i < MAX_CHANNELS; i++)
    {
        g_channels[i].mFileHdl = FREE_CHANNEL;
        g_channels[i].mMsgBuffPtr = NULL;
    }
}

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

// non-negative - ok; negative - error
int mk_channel(const char* chName, enum ChannelType chType)
{
    int result = -1;
    int file_hdl = 0;
    struct Channel *ch = NULL;
    int ch_id;
    int name_len = 0;

    if (g_first_channel)
    {
        _clear_channels();
        g_first_channel = 0;
    }

    ch_id = _find_free_channel();
    if (ch_id >= 0)
    {
        ch = &g_channels[ch_id];

        if (chType == ctServer)
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
            ch->mType = chType;
            ch->mFileHdl = file_hdl;
            result = ch_id;
        } else
        {
            result = -2;
        }
    }
    return result;
}

int rm_channel(int chId)
{
    int result = 0;
    struct Channel *ch = NULL;

    ch = &g_channels[chId];
    close(ch->mFileHdl);
    if (ch->mType == ctServer)
        unlink(ch->mName);
    free(ch->mName);
    free(ch->mMsgBuff);
    ch->mMsgBuffPtr = NULL;
    ch->mFileHdl = FREE_CHANNEL;

    return result;
}

const char* read_message(int chId)
{
    const char *result = NULL;
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

    return result;
}

int write_message(int chId, const char* msgText)
{
    int written_bytes = 0;
    struct Channel *ch = NULL;

    ch = &g_channels[chId];
    written_bytes = write(ch->mFileHdl, msgText, strlen(msgText) + 1);

    return written_bytes;
}
