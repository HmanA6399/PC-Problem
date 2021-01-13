#include "lib.h"
#include "buffer.h"

// IPC Clear handler
void clearIPC(int signum);

int main(int argc, char const *argv[])
{
    signal(SIGINT, clearIPC);
    signal(SIGSEGV, clearIPC);
    printf("Consumer has started\n");

    initBuffer();
    int buf_sem_id = getBufSemId();

    while (1)
    {
        down(buf_sem_id);
        if (!isBufEmpty()) {
            int x = consume();
            printf("I'm consuming %d\n", x);
        } else {
            printf("Buffer is empty\n");
        }
        up(buf_sem_id);
        sleep(2);
    }
}

void clearIPC(int signum) {
    destroyBuf();
}