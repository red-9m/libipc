#include <stdio.h>
#include "ipc.h"

int main()
{
    printf("IPC server example - ready for messages\n");

    int ch = mk_channel("ex", ctServer);
    int i;
    for (i = 0; i < 6000; i++)
        printf("%d) [%s]\n", i, read_message(ch));
    rm_channel(ch);
    return 0;
}
