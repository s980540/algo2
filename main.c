#include "main.h"
#include "thread.h"

foo_ts foo = {.mutex = (PTHREAD_MUTEX_INITIALIZER), .val = 0};

int main(int argc, char *argv[])
{
    printf("thread\n");
    srand(time(NULL));

    thread_init(argc, argv);
    thread_delete();

    return 0;
}