
/*
** client.c -- local build for AD MOHANRAJ
*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "8090" // the port client will be connecting to

#define MAXDATASIZE 250 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

bool completeTransmition(char *buf, int length);

int main() {
    /*STEPS
     * struct addrinfo - prep the socket address structures for subsequent use.
     * call getaddrinfo() to fill the struct for you
     * struct sockaddr holds socket address information for many types of sockets. (check function get_in_addr)
     * sockaddr_in is for internet protocol (hence the IN), can be casted to sockaddr
     * ex:
struct sockaddr_in {
  short int          sin_family;  // Address family, AF_INET
  unsigned short int sin_port;    // Port number
  struct in_addr     sin_addr;    // Internet address
  unsigned char      sin_zero[8]; // Same size as struct sockaddr (pads it for casting)
};
     * So if you have declared ina to be of type struct sockaddr_in, then ina.sin_addr.s_addr references the 4-byte IP address (in Network Byte Order)(BIG ENDIAN)
     *
     * inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr)) converts ip adress to val in struct sockaddr_in sa;
     *
     * What do we do with res? We iterate through it. Example:
for(p = res;p != NULL; p = p->ai_next) {
 void *addr;
 char *ipver;
 // get the pointer to the address itself,
 // different fields in IPv4 and IPv6:
 if (p->ai_family == AF_INET) { // IPv4
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
    addr = &(ipv4->sin_addr);
    ipver = "IPv4";}
 else { // IPv6
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
    addr = &(ipv6->sin6_addr);
    ipver = "IPv6";}
 // convert the IP to a string and print it:
 inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
 printf("  %s: %s\n", ipver, ipstr);
    }

     *Part 2 - get actual info w/ socket
     *call to getaddrinfo(), and feed them into socket() directly
     *int s;
struct addrinfo hints, *res;
s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
     *socket() simply returns to you a socket descriptor that you can use in later system calls
     *
     * part3 - bind() (SERVER SIDE)
     * associate a socket with a port on the machine(kernel listens for it)
     * int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
     * sockfd is the socket file descriptor returned by socket(). my_addr is a pointer to a struct sockaddr that contains information about your address, namely, port and IP address. addrlen is the length in bytes of that address.
     *
    * EXAMPLE CODE: SERVER SIDE
     *
struct addrinfo hints, *res;
int sockfd;

// first, load up address structs with getaddrinfo():

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

getaddrinfo(NULL, "3490", &hints, &res);

// make a socket:

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// bind it to the port we passed in to getaddrinfo():

bind(sockfd, res->ai_addr, res->ai_addrlen);

     Part 4 - CONNECT
connect(sockfd, res->ai_addr, res->ai_addrlen);
     information to fill connect comes from getaddrinfo()
     -> returns -1 on error if it fails
1
     Part 5 - LISTEN() AND ACCEPT()
     int listen(int sockfd, int backlog);
        backlog is the number of connections allowed on the incoming queue. incoming connections are going to wait in this queue until you accept() them (see below) and this is the limit on how many can queue up.
ORDER:
getaddrinfo();
socket();
bind();
listen();
accept()?
     someone far far away will try to connect() to your machine on a port that you are listen()ing on.
     Their connection will be queued up waiting to be accept()ed.
     You call accept() and you tell it to get the pending connection.
     It'll return to you a brand new socket file descriptor to use for this single connection!
     The original one is still listening for more new connections,
        and the newly created one is finally ready to send() and recv()

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
     sockfd is the listen()ing socket descriptor.
     addr will usually be a pointer to a local struct sockaddr_storage.
     This is where the information about the incoming connection will go.
     addrlen is a local integer variable that should be set to sizeof(struct sockaddr_storage) before its address is passed to accept(). accept() will not put more than that many bytes into addr.
     example call:
     new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
     use the socket descriptor new_fd for all send() and recv()

     PART 7 - SEND() AND RECV()
int send(int sockfd, const void *msg, int len, int flags);
     sockfd is the socket descriptor you want to send data to
     msg is a pointer to the data you want to send, and len is the length of that data in bytes.

     send may not send all the data (it is limited) so you must send the rest if it fails.
    * */
    const char *argv, *message;
    // std::string a = "73.202.216.109";
    std::string a = "73.223.185.43";
    std::string m = "thomas is super thicc";
    argv = a.c_str();
    //  message = m.c_str();

    int sockfd, numbytes;
    char buf[MAXDATASIZE];

    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    //   if (argc != 2) {
    //     fprintf(stderr,"usage: client hostname\n");
    //   exit(1);
    //}

    memset(&hints, 0, sizeof hints); //clear the struct for socket info
    hints.ai_family = AF_UNSPEC; //dont care about ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM; //tcp sock stream
    //sets up the struct addrinfo with IP, PORT,
    if ((rv = getaddrinfo(argv, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        //if socket type is null, error and drop through
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        //if we connect and there's an error, close the socket and drop through
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            close(sockfd);
            continue;
        }

        break;
    }
    //last case,p = NULL, client could not find any info from getaddrinfo
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    //  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    //      perror("recv");
    //      exit(1);
    //  }

    //  buf[numbytes] = '\0';

    while (true) {
        numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
        if (numbytes == -1)
            break;
        buf[numbytes] = '\0';
        printf("%s", buf);
        if (!completeTransmition(buf, numbytes))
            continue;
        //else {
        //    printf("transmition was incomplete. There may be errors. Please report this to me along with what was sent. Closing..");
        //    close(sockfd);
        //    return -1;
        // }
        std::cin >> m;
        if ((send(sockfd, m.c_str(), m.length(), 0) == -1))
            break;
    }
    return 0;
}

bool completeTransmition(char *buf, int length) {
    return (buf[length - 1] == '\004' or buf[length - 2] == '\004');
}