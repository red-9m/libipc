#include <stdio.h>
#include <ipc.h>
#include <unistd.h>

int main()
{
    printf("IPC server sample\n");

    int ch = ipc_create_channel("./ex", trSock, tpMessage, opNonblock);
    int i;
    for (i = 0; i < 6000; i++)
    {
        printf("%d) [%s]\n", i, ipc_read_message(ch));
        sleep(3);
        ipc_write_message(ch, "responce");
    }
    ipc_close_channel(ch);

    return 0;
}
