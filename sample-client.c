#include <stdio.h>
#include <ipc.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    printf("IPC client sample\n");

    if (argc != 2)
    {
        printf("Usage: %s <client-name>\n", argv[0]);
        return 1;
    }

    int ch = connect_channel("ex");
    if (ch < 0)
    {
        printf("Error - could not connect to the channel `ex`\n");
    } else
    {
        int i = 0;
        int written = 0;
        for (i = 0; i < 5000; i++)
        {
            char buf[1024];
            sprintf(buf, "===client[%s]test[%d]===", argv[1], i);
            written = write_message(ch, buf);
            if ( written < 0)
            {
                printf("Error - at iter[%d] written[%d]\n", i, written);
                break;
            }
        }
        printf("Done[%d] times for client [%s]\n", i, argv[1]);
        close_channel(ch);
    }

    return 0;
}
