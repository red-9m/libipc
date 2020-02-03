#include <stdio.h>
#include <ipc.h>
#include <unistd.h>

int main()
{
    printf("\n[Client] - Started\n");

    int ch = ipc_connect_channel("./ex", ipcSock, ipcMessage, ipcBlock);
    if (ch < 0)
    {
        printf("[Client] - Error, could not connect to the channel `ex` [%d]\n", ch);
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
                printf("[Client] - Error at iter[%d] written[%d]\n", i, written);
                break;
            } else
                printf("[Client] - Msg send[%d]\n", i);
            sleep(1);
            printf("[Client] - Got back [%s]\n", ipc_read_message(ch));
            sleep(1);
        }
        printf("[Client] - Done[%d] times for client\n", i);
        ipc_close_channel(ch);
    }

    return 0;
}
