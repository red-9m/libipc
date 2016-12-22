#include <stdio.h>
#include "ipc.h"
#include <unistd.h>

int main(int argc, char** argv)
{
    (void)argc;
    printf("IPC client example\n");

    int ch = mk_channel("ex", ctClient);
    if (ch < 0)
    {
        printf("Error - incorrect channel name\n");
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
        printf("Done for [%d] clients\n", i);
    }

    rm_channel(ch);
    return 0;
}
