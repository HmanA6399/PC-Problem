#pragma once
#include "lib.h"

#define BUF_SIZE 10
#define BUF_SHM_KEY 0x0AAAA
#define BUF_SEM_KEY 0x0ABAE

// Buffer DS
typedef struct Buffer {
    int rd_idx;
    int wr_idx;
    int max_size;
    int count;
    int storage[BUF_SIZE];
} Buffer;

// Process-scoped variable carrying buffer address
int buf_id, buf_sem_id;
Buffer * buf_addr;

void initBuffer() {
    buf_id = getShmID(BUF_SHM_KEY, sizeof(Buffer), IPC_CREAT);
    buf_addr = (Buffer *) getShmAddr(buf_id);
    buf_sem_id = getSem(BUF_SEM_KEY, IPC_CREAT);
    printf("I'm Initted\n");
    
    // If I'm the first one to be attached, set buffer variables
    if (getShmAttachesCount(buf_id) == 1) {
        buf_addr->rd_idx = 0;
        buf_addr->wr_idx = 0;
        buf_addr->count = 0;
        buf_addr->max_size = BUF_SIZE;

        // Init buffer semaphore
        initSem(buf_sem_id, 1);
    }
}

int getBufSemId() {
    return buf_sem_id;
}

void destroyBuf() {
    // Detach buffer
    releaseShmAddr(buf_addr);
    printf("I'm detaching\n");

    // Release the sem if I'm holding it
    if (getSemPID(buf_sem_id) == getpid()) {
        printf("I'm the one holding the semaphore so release !\n");
        up(buf_sem_id);
    }

    // Remove if last process attached
    if (getShmAttachesCount(buf_id) == 0) {
        printf("I'm the last user of buf_shm so I'll delete\n");
        deleteShm(buf_id);
        deleteSemSet(buf_sem_id);
    }
}

void produce(int data) {
    if (buf_addr->count < buf_addr->max_size) {
        buf_addr->storage[buf_addr->wr_idx] = data;
        buf_addr->wr_idx = (buf_addr->wr_idx + 1) % buf_addr->max_size;
        buf_addr->count++;
    } else {
        printf("Maaan :( I'll not produce if the buffer is full\n");
    }
}

int consume() {
    if (buf_addr->count > 0) {
        int data = buf_addr->storage[buf_addr->rd_idx];
        buf_addr->rd_idx = (buf_addr->rd_idx + 1) % buf_addr->max_size;
        buf_addr->count--;
        return data;
    } else {
        printf("Maaan :( I'll not consume if the buffer is empty\n");
        return -1;
    }
}

bool isBufEmpty() {
    return buf_addr->count <= 0;
}

bool isBufFull() {
    return buf_addr->count >= buf_addr->max_size;
}