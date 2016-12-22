#include <stdio.h>
#include "ipc.h"

int main()
{
    printf("IPC Hello World\n");

    int ch_consumer = mk_channel("ex", ctServer);
    int ch_producer = mk_channel("ex", ctClient);

    write_message(ch_producer, "Hello World - IPC!");
    write_message(ch_producer, "Hello World - Named Pipe!");

    printf("Got first message from channel :%s\n", read_message(ch_consumer));
    printf("Got second message from channel :%s\n", read_message(ch_consumer));

    rm_channel(ch_producer);
    rm_channel(ch_consumer);

    return 0;
}
