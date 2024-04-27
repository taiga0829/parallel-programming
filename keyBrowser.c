#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MSG_SIZE 128

struct msgbuf {
    long mtype;
    char mtext[MSG_SIZE];
};

int main() {
    key_t key = 0x0112ca9f; // Replace this with the key of the message queue you want to access
    int msqid;
    struct msgbuf buf;

    if ((msqid = msgget(key, 0666)) == -1) {
        perror("msgget");
        return 1;
    }

    printf("Message Queue ID: %d\n", msqid);

    while (1) {
        if (msgrcv(msqid, &buf, MSG_SIZE, 0, IPC_NOWAIT) == -1) {
            // No more messages
            break;
        }
        printf("Received Message: %s\n", buf.mtext);
    }

    return 0;
}
