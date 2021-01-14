#include "lib.h"
#include "buffer.h"
#include "notification.h"

// IPC Clear handler
void clearIPC(int signum);

int main(int argc, char const *argv[])
{   
    // Bind signal handlers
    signal(SIGINT, clearIPC);
    signal(SIGSEGV, clearIPC);
    printf("Consumer has started with pid %d\n", getpid());
    
    // Init IPC
    initBuffer();
    initNotificationQueue();
    int buf_sem_id = getBufSemId();

    int x;  // Carrys read value 
    bool wasFull = false;

    while (1)
    {
        down(buf_sem_id);
        if (isBufEmpty()) {
            printf("Waiting for producer\n");
            up(buf_sem_id);
            waitForProducer();
            down(buf_sem_id);
            printf("Producer is here ! Consuming ...\n");
        }

        // Consume a value
        wasFull = isBufFull();
        x = consume();
        if (x != -1) printf("I'm consuming %d\n", x);

        // Release and notify if full
        if (wasFull) notifyProducer();
        up(buf_sem_id);
        sleep(1);
    }
}

void clearIPC(int signum) {
    destroyBuf();
    destroyNotificationQueue();
    exit((signum == SIGINT || signum == SIGSEGV) ? -1 : 0);
}