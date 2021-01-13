#include "lib.h"
#include "buffer.h"

// IPC Clear handler
void clearIPC(int signum);


int main(int argc, char const *argv[])
{
    signal(SIGINT, clearIPC);
    signal(SIGSEGV, clearIPC);
    printf("Hi. I'm the producer\n");

    // Init IPC
    initBuffer();
    int buf_sem_id = getBufSemId();

    int val;
    
    while (1) {
        down(buf_sem_id);
        if (!isBufFull()) {
            val = rand() % 10000;
            printf("I'm producing %d\n", val);
            produce(val);
        } else {
            printf("Buffer is full !!\n");
        }
        up(buf_sem_id);
        sleep(1);
    }
}

void clearIPC(int signum) {
    destroyBuf();
    exit(0);    
}