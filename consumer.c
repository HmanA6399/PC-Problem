#include "lib.h"

// IPC IDs

// IPC Clear handler
void clearIPC(int signum);

int main(int argc, char const *argv[])
{
    signal(SIGINT, clearIPC);
    printf("Consumer has started\n");
    return 0;
}

void clearIPC(int signum) {

}