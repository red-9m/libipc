#include <stdio.h>
#include <ipc.h>

int main()
{
    printf("IPC server sample\n");

    int ch = ipc_create_channel("ex", tpMessage, opBlock);
    int i;
    for (i = 0; i < 6000; i++)
        printf("%d) [%s]\n", i, ipc_read_message(ch));
    ipc_close_channel(ch);

    return 0;
}
