#include <stdio.h>
#include <ipc.h>

int main()
{
    printf("IPC server sample\n");

    int ch = create_channel("ex");
    int i;
    for (i = 0; i < 6000; i++)
        printf("%d) [%s]\n", i, read_message(ch));
    close_channel(ch);

    return 0;
}
