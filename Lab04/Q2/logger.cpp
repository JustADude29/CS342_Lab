#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>


int main(void) {
    char buf[5000];
    int msqid;
    key_t key;
    if ((key = ftok("logger.cpp", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }
    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) { /* connect to the queue */
        perror("msgget 1");
    }
    
    printf("Logger online\n");
    
    for(int xyz = 1;;xyz++) { /* Spock never quits! */
        if (msgrcv(msqid, buf, sizeof(buf), 0, 0) == -1) {
            perror("msgrcv");
            break;
        }
        printf("%s\n", buf);
    }

    printf("Bye!\n");

    return 0;
}
