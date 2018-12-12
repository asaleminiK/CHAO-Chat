//
// Created by ameen on 11/23/18.
//

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
#include <sys/types.h>
#include <signal.h>

#include <arpa/inet.h>

#define PORT "8090" // the port client will be connecting to

#define MAXDATASIZE 250 // max number of bytes we can get at once
/*packet header defined by 2 bytes:
 * __%%%%%%%%%%%%%%%%%%%%
 * i:logIn
 *  u:username
 *  p:password
 * o:logOut
 *  u:username
 * c:Create Account
 *  u:username
 *  p:password
 * t :talk
 *
 * END PACKET byte defined by "x"
 */

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

bool completeTransmition(char *buf, int length);

bool closeSocket(char * buf, int numbytes);

int main() {
    const char *argv, *message;
    // std::string a = "73.202.216.109";
    std::string a = "73.202.216.109";
    std::string m("");
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
    //Prompt for login and insert LU for server side (Login Username)
    printf("Login: ");
    std::cin>>m;
    std::cin.ignore();
    m.insert(0,1,'l');
    m.insert(1,1,'u');
    //send returns -1 upon failure. We quit the program if such happens (socket was closed)
    if ((send(sockfd, m.c_str(), m.length(), 0) == -1))
        return 0;
    //main loop for chatting
    while (true) {
        numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
        if (numbytes == -1)
            break;
        //null terminate string with the number of bytes we recieved
        buf[numbytes] = '\0';
        //the server may send one byte, representing nothing (empty quotes flush the input buffer)
        //thus, we make sure that the output string is shifted by 2, since the first two bytes
        //identify the type of incoming packet
        if(numbytes != 1) {
            std::cout << buf + 2;
        }
        fflush(stdout);
        if (buf[numbytes-1] == 'x'){
            close(sockfd);
            break;
        }

        //else {
        //    printf("transmition was incomplete. There may be errors. Please report this to me along with what was sent. Closing..");
        //    close(sockfd);
        //    return -1;
        // }
        //Prompt for password if asked to with Login Password
        if(buf[0] == 'l' && buf[1] == 'p') {
            std::cout<< "Password: ";
            getline(std::cin, m);
            m.insert(0, "lp");
            //Send password to server with lp back
            if ((send(sockfd, m.c_str(), m.length(), 0) == -1))
                break;
        }
        //If the server does not find an account, it will ask for the password to create a new account instead
        else if(buf[0] == 'c') {
            std::string temppassword;
            std::cout<< "Account was not found. Would you like to create one?(y/n)";
            getline(std::cin, temppassword);
            if(temppassword[0] == 'n'){
                close(sockfd);
                break;
            }
            std::cout<< "Enter password:";
            getline(std::cin, m);
            std::cout<< "Re-enter password:";
            getline(std::cin,temppassword);
            if(m == temppassword) {
                m.insert(0, "cp");
                send(sockfd, m.c_str(), m.length(), 0);
            }
            else{
                std::cout << "Passwords did not match, exiting..";
                close(sockfd);
                break;
            }


        }
        /* getline() blocks program execution until an input is received.
         * We fork it and let the program continue execution
         *
         */
        else{
            int forknum = fork();
            if(forknum == 0) {
               // std::cout<<"\n-------------------------------------------\n:";
                std::cout<<"\n:";
                m = std::string("");
                getline(std::cin, m);
                m.insert(0, "t ");
                if ((send(sockfd, m.c_str(), m.length(), 0) == -1))
                    break;
                exit(0);
            }
        }

    }
    return 0;
}

//Packages used to be terminated by \004 character
//This function is deprecated as the server no longer terminates sockets

bool completeTransmition(char *buf, int length) {
    return (buf[length - 1] == '\004' or buf[length - 2] == '\004');
}