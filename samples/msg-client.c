#include <stdio.h>
#include <ipc.h>
#include <unistd.h>

int main()
{
    printf("IPC client sample\n");

    int ch = ipc_connect_channel("./ex", trSock, tpMessage, opBlock);
    if (ch < 0)
    {
        printf("Error - could not connect to the channel `ex` [%d]\n", ch);
    } else
    {
        int i = 0;
        int written = 0;
        for (i = 0; i < 5; i++)
        {
            char buf[1024];
            snprintf(buf, 1024, "===client=test[%d]===", i);
            written = ipc_write_message(ch, buf);
            if ( written < 0)
            {
                printf("Error - at iter[%d] written[%d]\n", i, written);
                break;
            } else
                printf("Msg send[%d]\n", i);
            sleep(1);
            printf("Got back [%s]\n", ipc_read_message(ch));
            sleep(1);
        }
        printf("Done[%d] times for client\n", i);
        ipc_close_channel(ch);
    }

    return 0;
}
