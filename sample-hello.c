#include <stdio.h>
#include <ipc.h>

static const char* CH_NAME = "ch_hello";

int main()
{
    printf("IPC Hello World\n");

    int ch_consumer = create_channel(CH_NAME);
    int ch_producer = connect_channel(CH_NAME);

    write_message(ch_producer, "Hello World - IPC!");
    write_message(ch_producer, "Hello World - Named Pipe!");

    printf("Got first message from channel[%s] :%s\n", CH_NAME, read_message(ch_consumer));
    printf("Got second message from channel[%s] :%s\n", CH_NAME, read_message(ch_consumer));

    close_channel(ch_producer);
    close_channel(ch_consumer);

    return 0;
}
