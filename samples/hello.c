#include <stdio.h>
#include <ipc.h>

struct person
{
    char name[128];
    char address[256];
    int isMarried;
};

static struct person adam = { "Adam Smith", "Current address is unknown", 1 };
static const char* CH_NAME = "ch_hello";

int main()
{
    struct person got_person;

    printf("IPC Hello World\n");

    int ch_consumer = ipc_create_channel(CH_NAME, tpObject);
    int ch_producer = ipc_connect_channel(CH_NAME, tpObject);

    ipc_write_object(ch_producer, &adam, sizeof(struct person));

    ipc_read_object(ch_consumer, &got_person, sizeof(struct person));
    printf("From channel[%s] got person[%s] with address[%s] is married[%d]\n", CH_NAME, got_person.name, got_person.address, got_person.isMarried);

    ipc_close_channel(ch_producer);
    ipc_close_channel(ch_consumer);

    return 0;
}
