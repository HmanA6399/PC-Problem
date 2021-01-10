#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

//======= begin DEFINES =======================
#define KEYSALT 6532
#define MAX_MSG_SZ 256
#define MSQ_PERMS 0666
#define SHMPERMS 0666
#define SEMPERMS 0666

//======= e n d DEFINES =======================

//======= begin msq functions =================

// MSG struct
typedef struct msg_t {
    long mtype;
    char mtext[MAX_MSG_SZ];
} msg_t;

/**
 * @brief Wrapper for msgget
 * 
 * @param key Key of the requested msq
 * @param create Put IPC_CREAT if you want to create it for the first time, 0 otherwise
 * @return ID of the created msq
 * @note Exits on failure !
*/
int getMsqID(int key, int create) {
    int msgq_id = msgget(key, MSQ_PERMS | create);
    if (msgq_id == -1) {
        char errtxt[256];
        sprintf(errtxt, "Error in creation, key : %d", key);
        perror(errtxt);
        exit(-1);
    }
    return msgq_id;
}

/**
 * @brief Create Message of type msg_t
 * 
 * @param type Type field of the message
 * @param txt Contents of the message
 * @return  msg_t instance created
*/
msg_t createMessage(int type, char txt[]) {
    msg_t msg;
    msg.mtype = type;
    strcpy(msg.mtext, txt);
    return msg;
}

/**
 * @brief Send created message to the msq with the given ID
 * 
 * @param message Message to send of type msg_t
 * @param msgq_id ID of the message queue
 * @note BLOCKING
 * @note Exits on failure !
*/
void sendMessage(msg_t message, int msgq_id) {
    int sent_msg_status = msgsnd(msgq_id, &message, sizeof (message.mtext), !IPC_NOWAIT);
    if (sent_msg_status == -1) {
        perror("Error in sending to queue");
        exit(-1);
    }
}

/**
 * @brief Recieve a msg_t instance from msq with the given ID
 * 
 * @param msgq_id 
 * @param typ 
 * @returns The recieved message of type msg_t
 * @note BLOCKING
 * @note Exits on failure !
 */
msg_t recieveMessage(int msgq_id, long typ) {
    msg_t recieved_message;
    int msgrcv_status = msgrcv(msgq_id, &recieved_message, MAX_MSG_SZ, typ, !IPC_NOWAIT);

    if (msgrcv_status == -1) {
        perror("Error in message recieve");
        exit(-1);
    }

    return recieved_message;
}
//======= begin msq functions =================

//======== begin Shared memory methods ========

/**
 * @brief Get Shm ID
 * 
 * @param key key of the requested shmid 
 * @param create Pass IPC_CREAT if you want it to be created if not found, 0 otherwise
 * @return shmid of the requested key
 * @note Exits on failure in case of creation
 */
int getShmID(key_t key, int create) {
    int shm_id = shmget(key, sizeof(int), SHMPERMS | create);
    if (shm_id == -1) {
        if (create) {
            perror("Error in starting shared memory initialization");
            exit(-1);
        }
        return -1;
    }

    return shm_id;
}

/**
 * @brief Attach and Get the Shm Addr of the given shmid
 * 
 * @param shmid shmid to be attached to
 * @return shmaddr at the beginning of the shm block
 */
int* getShmAddr(int shmid) {
    return (int*) shmat(shmid, (void*) 0, 0);
}

/**
 * @brief Detach shmaddr
 * 
 * @param shm_addr shmaddr to be released
 * @note Exits on filure !
 */
void releaseShmAddr(int* shm_addr) {
    if(shmdt(shm_addr) == -1) {
        perror("Error in shmdt");
        exit(-1);
    }
}

/**
 * @brief Delete shm IPC Resource of given ID
 * 
 * @param shm_id shmid to be deleted
 * @note Exits on failure !
 */
void deleteShm(int shm_id) {
    if (shmctl(shm_id, IPC_RMID, (struct shmid_ds*) 0) == -1) {
        perror("Shared Memory couldn't be deleted");
        exit(-1);
    }
	printf("Shared Memory was deleted.\n");
}
//======== e n d Shared memory methods ======

//======== begin Semaphore set methods ======
/** @brief DS for semctl */
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

/**
 * @brief Get Binary Sem set, init all sims to 0.
 * 
 * @param key Key of the requested sem_id to create
 * @param create Pass IPC_CREAT if you want it to be created if not found, 0 otherwise
 * @return semid 
 * @note Exits on failure
*/
int getSem(int key, int create) {
    int sem_id = semget(key, 1, SEMPERMS | create);

    if (sem_id == -1) {
        perror("Error in semget");
        exit(-1);
    }

    union semun initer;
    initer.val = 0;
    if (semctl(sem_id, 0, SETVAL, initer) == -1) {
        perror("Error in semget");
        exit(-1);
    }

    return sem_id;
}

/**
 * @brief Attempt to aquire a symaphore
 * 
 * @param sem_set_id identifier for the used symaphore set
 * @return result of semaphore operation
*/
int down(int sem_set_id) {
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = IPC_NOWAIT;

    return semop(sem_set_id, &p_op, 1);
}

/**
 * @brief Release a symaphore
 * 
 * @param sem_set_id identifier for the used symaphore set
 * @return result of semaphore operation
*/
int up(int sem_set_id) {
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = IPC_NOWAIT;

    // TODO :: Make it blocking if necessary
    return semop(sem_set_id, &v_op, 1);
}

/**
 * Clear Sem resource (cancelation point)
 * 
 * @param sem_id id for the sem set to be deleted
 * @note Exits on failure !
*/
void deleteSemSet(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
		perror("Sem set could not be deleted.");
		exit(-1);
	}

	printf("Sem set was deleted.\n");
}