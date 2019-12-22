// 1, recv_message func runs in separate thread that prints the received message if any
// 2, main parent thread will be waiting for user input
// 3, if user is typing -> message will receive -> user no need to type message again -> data can be sent correctly
// 4, User1 and User2 are the same program but with reversed send and recv ports - can have this as input from user but not now
// 5, I have tested the app by running two process (First and Second) in the same PC but with two different IP.
// 6, I used cygwin environment installed in windows 8 and compiled with gcc
// 7, I have also attached a video with this souce code that may help you to rebuild if needed.
// 8, sending and receiving messages are mutually exclusive and also running in separate threads
// 9, also works with inputs with spaces

//Video Shown with: 
//192.168.1.5 - First
//192.168.1.10 - Second

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <pthread.h> 
 
#define SEND_PORT	 5555 
#define RECV_PORT	 6666 
#define BUFFER_MAX_LEN 1024 

char buffer[BUFFER_MAX_LEN], FirstUserIP[15],SecondUserIP[15];
pthread_mutex_t atomicLock;

//err handler
void die(char *s){
	perror(s);
	exit(1);
}

//runs in separate thread - binds RECV_PORT and waits for message
void recv_message(){
	int sockfd, len, n;  
	struct sockaddr_in firstaddr, secondaddr; 
	
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
		die("socket creation failed"); 			
			
	memset(&firstaddr, 0, sizeof(firstaddr)); 
	memset(&secondaddr, 0, sizeof(secondaddr)); 
	
	firstaddr.sin_family = AF_INET; // ipv4 
	firstaddr.sin_addr.s_addr = inet_addr(FirstUserIP);
	firstaddr.sin_port = htons(RECV_PORT); 
	
	uint8_t octet[4];
	for(int i=0;i<4;i++)
		octet[i]=firstaddr.sin_addr.s_addr >> (i*8);
	printf("\nFirstUserIP: %d.%d.%d.%d\n",octet[0],octet[1],octet[2],octet[3]);
		
		
	//bind the socket with the given ip address
	if ( bind(sockfd, (const struct sockaddr *)&firstaddr, sizeof(firstaddr)) == -1 )  
		die("bind failed"); 	

	len = sizeof(secondaddr);
	
	while(1) {
		pthread_mutex_lock(&atomicLock);
		fflush(stdout);
		if((n = recvfrom(sockfd, (char *)buffer, BUFFER_MAX_LEN, 0, ( struct sockaddr *) &secondaddr, &len))== -1) {
			die("recvfrom");
		}
		buffer[n] = '\0'; // trailing
		printf("\n--------\n Message Received Successfully: %s\n----------\n", buffer);
		memset(&buffer, 0, sizeof(buffer)); // clear buffer before getting input
		printf("\n***********\nEnter new message : ");		
		pthread_mutex_unlock(&atomicLock);
	}

	close(sockfd);
	return; 
}
	
// Main Func to get user input and send it to the other system
int main() { 

	memset(&FirstUserIP, 0, sizeof(FirstUserIP));
	memset(&SecondUserIP, 0, sizeof(SecondUserIP));
	
	printf("\nEnter Your IP address(source/host) for binding: ");
	fgets(FirstUserIP,sizeof(FirstUserIP),stdin);
	
	printf("\nEnter Remote IP address(destrination/guest) for binding: ");
	fgets(SecondUserIP,sizeof(SecondUserIP),stdin);
	
	
	//do it now, so that will receive message while processing for sending (usually slow as it involves user input)
	pthread_t recv_thread;
	pthread_create(&recv_thread, NULL, recv_message, NULL); 
	
	int sockfd; 
	struct sockaddr_in secondaddr; 

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
		die("socket creation failed"); 

	memset(&secondaddr, 0, sizeof(secondaddr)); 

	secondaddr.sin_family = AF_INET; 
	secondaddr.sin_port = htons(SEND_PORT); 
	secondaddr.sin_addr.s_addr = inet_addr(SecondUserIP); 
	
	uint8_t octet[4];
	for(int i=0;i<4;i++)
		octet[i]=secondaddr.sin_addr.s_addr >> (i*8);
	printf("\SecondUserIP: %d.%d.%d.%d\n",octet[0],octet[1],octet[2],octet[3]);
	
	while(1) {
		printf("\n***********\nEnter new message : ");
		pthread_mutex_lock(&atomicLock);
		fgets(buffer,BUFFER_MAX_LEN,stdin);
		if((sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *) &secondaddr, sizeof(secondaddr))) ==-1)
			die("sendto");
		printf("\nMessage(%s) Sent Successfully.\n***********\n",buffer); 	
		pthread_mutex_unlock(&atomicLock);
	}
		
	close(sockfd); 
	return 0; 
} 
