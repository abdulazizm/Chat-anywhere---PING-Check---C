// 1, recv_message func runs in separate thread that prints the received message if any
// 2, main parent thread will be waiting for user input
// 3, if user is typing -> message will receive -> user no need to type message again -> data can be sent correctly
// 4, User1 and User2 are the same program but with reversed send and recv ports - can have this as input from user but not now
// 5, I have tested the app by running two process (First and Second) in the same PC but with two different IP.
// 6, I used cygwin environment installed in windows 8 and compiled with gcc
// 7, I have also attached a video with this souce code that may help you to rebuild if needed.
// 8, sending and receiving messages are mutually exclusive and also running in separate threads
// 9, also works with inputs with spaces

//Updates:
//Check User Provided Input for IPv4 address format

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
 
#define SEND_PORT	 6666 
#define RECV_PORT	 7777 
#define BUFFER_MAX_LEN 1024 
#define DELIM "." //Delimeter to be separated for validating IPv4 address

char buffer[BUFFER_MAX_LEN], FirstUserIP[]="000.000.000.000",SecondUserIP[]="000.000.000.000";
pthread_mutex_t atomicLock;

//err handler
void die(char *s){
	perror(s);
	exit(1);
}

int checkNumber(char *chr) 
{ 
	while (*chr) { 
		if ((*chr >= 48 && *chr <= 57) || *chr == 10) 
			++chr; 
		else {
			printf("%c -Invalid character entered\n", *chr);
			return 0;
		} 
	} 
	return 1; 
} 

int checkValidIP(char *chr) 
{ 
	int i, num, dots = 0; 
	char *ptr; 

	if (chr == NULL) {
		printf("Please enter valid IPv4 address\n");
		return 0;
	} 

	ptr = strtok(chr, DELIM); 

	if (ptr == NULL) {
		printf("Please enter valid IPv4 address\n");
		return 0; 
	}

	while (ptr) { 

		if (!checkNumber(ptr)) {
			printf("No letters allowed. Only Numbers with dots().\n");
			return 0; 
		}
		num = atoi(ptr); 

		if (num >= 0 && num <= 255) { 
			ptr = strtok(NULL, DELIM); 
			if (ptr != NULL) ++dots; 
		} else
			return 0; 
	} 

	if (dots != 3) {
		printf("Please enter valid IPv4 address. Dots(xxx.xxx.xxx.xxx) missing\n");
		return 0;
	} 
	return 1; 
}



//runs in separate thread - binds RECV_PORT and waits for message
void recv_message(){
	int sockfd, len, n;  
	struct sockaddr_in firstaddr, secondaddr; 
	
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
		die("socket creation failed"); 			
			
	memset(&firstaddr, 0, sizeof(firstaddr)); 
	memset(&secondaddr, 0, sizeof(secondaddr)); 
	
	secondaddr.sin_family = AF_INET; // ipv4 
	secondaddr.sin_addr.s_addr = inet_addr(SecondUserIP);
	secondaddr.sin_port = htons(RECV_PORT); 
	
	uint8_t octet[4];
	for(int i=0;i<4;i++)
		octet[i]=secondaddr.sin_addr.s_addr >> (i*8);
	printf("\nSecondUserIP: %d.%d.%d.%d\n",octet[0],octet[1],octet[2],octet[3]);
	
	//bind the socket with the given ip address
	if ( bind(sockfd, (const struct sockaddr *)&secondaddr, sizeof(secondaddr)) == -1 )  
		die("bind failed"); 	

	len = sizeof(firstaddr);
	
	while(1) {
		fflush(stdout);
		if((n = recvfrom(sockfd, (char *)buffer, BUFFER_MAX_LEN, 0, ( struct sockaddr *) &firstaddr, &len))== -1) {
			die("recvfrom");
		}
		pthread_mutex_lock(&atomicLock);
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

	char checkIP[]="000.000.000.000";
	//get source/host IPv4 address
	while(1) {
		printf("\nEnter Your IP address(source/host) for binding: ");
		fgets(SecondUserIP,sizeof(SecondUserIP),stdin);
	
		memcpy(checkIP,SecondUserIP,16);
		if(checkValidIP(checkIP)) break;
	}
	//get destination/guest IPv4 address
	while(1){
		printf("\nEnter Remote IP address(destination/guest) for binding: ");
		fgets(FirstUserIP,sizeof(FirstUserIP),stdin);

		memcpy(checkIP,FirstUserIP,16);
		if(checkValidIP(checkIP)) break;
	
	}
	//do it now, so that will receive message while processing for sending
	pthread_t recv_thread;
	
	if (pthread_create(&recv_thread, NULL, recv_message, NULL)) {
            printf("ERROR from pthread_create()\n");
            exit(-1);
        }

	int sockfd; 
	struct sockaddr_in firstaddr; 

	// Creating socket file descriptor 
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
		die("socket creation failed"); 

	memset(&firstaddr, 0, sizeof(firstaddr)); 

	firstaddr.sin_family = AF_INET; //ipv4
	firstaddr.sin_port = htons(SEND_PORT); 
	firstaddr.sin_addr.s_addr = inet_addr(FirstUserIP);

	uint8_t octet[4];
	for(int i=0;i<4;i++)
		octet[i]=firstaddr.sin_addr.s_addr >> (i*8);
	printf("\nFirstUserIP: %d.%d.%d.%d\n",octet[0],octet[1],octet[2],octet[3]);	
	
	while(1) {
		printf("\n***********\nEnter new message : ");
		fgets(buffer,BUFFER_MAX_LEN,stdin);
		pthread_mutex_lock(&atomicLock);
		if((sendto(sockfd, (const char *)buffer, strlen(buffer), 0, (const struct sockaddr *) &firstaddr, sizeof(firstaddr))) ==-1)
			die("sendto");
		printf("\nMessage(%s) Sent Successfully.\n***********\n",buffer); 	
		pthread_mutex_unlock(&atomicLock);
	}
		
	close(sockfd);
	return 0; 
} 
