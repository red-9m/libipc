#include <stdio.h>
#include <ipc.h>
#include <unistd.h>

int main()
{
    printf("\n[Server] - Started\n");

    int ch = ipc_create_channel("./ex", ipcSock, ipcMessage, ipcNonblock);
    int i;
    for (i = 0; i < 50; i++)
    {
        printf("[Server] - %d [%s]\n", i, ipc_read_message(ch));
        sleep(3);
        ipc_write_message(ch, "responce");
    }
    ipc_close_channel(ch);

    return 0;
}
