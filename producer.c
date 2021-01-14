#include "lib.h"
#include "buffer.h"
#include "notification.h"

// IPC Clear handler
void clearIPC(int signum);

int main(int argc, char const *argv[])
{
    signal(SIGINT, clearIPC);
    signal(SIGSEGV, clearIPC);
    printf("Hi. I'm the producer %d\n", getpid());

    // Init IPC
    initBuffer();
    int buf_sem_id = getBufSemId();
    initNotificationQueue();
    
    int val;    // Carrys produced value    
    bool wasEmpty = false;

    while (1) {
        down(buf_sem_id);
        if (isBufFull()) {
            // Just wait for consumer notification
            printf("Waiting for consumer\n");
            up(buf_sem_id);
            waitForConsumer();
            down(buf_sem_id);
            printf("Consumer is here ! Producing ...\n");
        }
        // produce random value
        val = rand() % 10000;
        printf("I'm producing %d\n", val);
        wasEmpty = isBufEmpty();
        produce(val);

        // Release and notify the consumer if empty
        if (wasEmpty) notifyConsumer();
        up(buf_sem_id);
        sleep(2);
    }
}

void clearIPC(int signum) {
    destroyBuf();
    destroyNotificationQueue();
    exit((signum == SIGINT || signum == SIGSEGV) ? -1 : 0);
}