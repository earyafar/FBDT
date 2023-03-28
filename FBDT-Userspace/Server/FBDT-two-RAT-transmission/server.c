/* Forward and Backward Data Transmission Code
 * Author: Suresh Srinivasan
 * Email: suresh2@pdx.edu
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/if.h>
#include <time.h>
#include <sys/time.h>






#define PORT 8080


#define BUFFER_SIZE 1024*1024
//#define BUFFER_SIZE 1024*50
#define SEND_BUFFER_SIZE 1024
#define ENABLE_BACKWARD_TRANSMISSION
#define ENABLE_FORWARD_TRANSMISSION



void* reverse_memcpy(
void *dest, const void *src, size_t len)
{

    char *d = dest;
    const char *s = src+len-1;
    while(len--)
        *d++ = *s--;
    return dest;
}


typedef struct data_ack {
    int ack;
    int rev_ack;
}fbdt_ack;

int main(int argc, char const* argv[])
{
	int sockwifi=0, valread, client_fd1, client_fd2, sockwigi=0;
	struct sockaddr_in serv_addr1, serv_addr2;
	char* hellowifi = "Hello from wificlient";
	char* hellowigi = "Hello from wigiclient";
	char buffer[1024] = { 0 };
	struct ifreq ifr1, ifr2;
	int ret1, ret2;
	int val, val1;
	int *fwd_ptr, *rev_ptr, offset=0;
	char buff[BUFFER_SIZE];
	char send_buff[SEND_BUFFER_SIZE];
	fbdt_ack send_ack, rev_ack;
	int ref;

	for(val=0;val<BUFFER_SIZE-4;val=val+4)
        memcpy(&buff[val],(int *) &val, sizeof(int));

/*	for(val=0;val<BUFFER_SIZE-4;val=val+4){
		memcpy((int *) &val1, &buff[val], sizeof(int));
		printf("buffer val:%d\n", val1);fflush(NULL);
	}
	return 0;*/




	if ((sockwifi = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	if ((sockwigi = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr1.sin_family = AF_INET;
	serv_addr1.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "192.168.10.245", &serv_addr1.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	serv_addr2.sin_family = AF_INET;
	serv_addr2.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "192.168.1.4", &serv_addr2.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}


	memset(&ifr1, 0, sizeof(ifr1));
	snprintf(ifr1.ifr_name, sizeof(ifr1.ifr_name), "enp4s0");
	if (setsockopt(sockwifi, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr1, sizeof(ifr1)) < 0) {
		printf("Unable to bind to interface:%s",ifr1.ifr_name);
		return -1;
	}

	memset(&ifr2, 0, sizeof(ifr2));
	snprintf(ifr2.ifr_name, sizeof(ifr2.ifr_name), "enp5s0");
	if (setsockopt(sockwigi, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr2, sizeof(ifr2)) < 0) {
		printf("Unable to bind to interface:%s",ifr2.ifr_name);
		return -1;
	}


	if ((client_fd1
		= connect(sockwifi, (struct sockaddr*)&serv_addr1,
				sizeof(serv_addr1)))
		< 0) {
		printf("\nWiFi Connection Failed \n");
		return -1;
	}

	if ((client_fd2
		= connect(sockwigi, (struct sockaddr*)&serv_addr2,
				sizeof(serv_addr2)))
		< 0) {
		printf("\nWigi Connection Failed \n");
		return -1;
	}



	while(1) {
	fwd_ptr=(int *)&buff[0];
    rev_ptr = (int *) (&buff[BUFFER_SIZE-SEND_BUFFER_SIZE-8]);
	offset=0;
//	printf("\nfwd_ptr:%p and rev_ptr:%p", fwd_ptr, rev_ptr);fflush(NULL);

    while(fwd_ptr<=rev_ptr && fwd_ptr!=NULL && rev_ptr!=NULL) {

#ifdef ENABLE_FORWARD_TRANSMISSION
		//send forward data transmission.....
		if(fwd_ptr+SEND_BUFFER_SIZE == NULL)
			break;
		memcpy(send_buff, fwd_ptr, SEND_BUFFER_SIZE);
		ret1=send(sockwifi, send_buff, SEND_BUFFER_SIZE, MSG_DONTWAIT);
//		printf("ret1:%d", ret1);
		if(ret1 > 0){
			fwd_ptr=fwd_ptr+(ret1/4);
			//printf("\nwifi:fwd_pt:%p and ret:%d", fwd_ptr, ret1);fflush(NULL);
			//for(int i=0;i<ret1;i=i+4){
//				memcpy(&ref, (int *)&send_buff[0], sizeof(int));
//				printf("\nwifi:val:%d and ret:%d nextval:%d", ref, ret1, ref+ret1);fflush(NULL);
			//}
		}
		
		
		//ret1=send(sockwifi, hellowifi, strlen(hellowifi), 0);
		//printf("Hello message sent\n and %dbytes sent", ret1);
		//valread = read(sockwifi, buffer, 1024);
		//printf("%s\n", buffer);
#endif

		//send reverse data transmission.....
		if(rev_ptr-SEND_BUFFER_SIZE == NULL)
			break;
#ifdef ENABLE_BACKWARD_TRANSMISSION		
//		memcpy(send_buff, (rev_ptr+(offset/4)), SEND_BUFFER_SIZE-offset);
		reverse_memcpy(send_buff, rev_ptr, SEND_BUFFER_SIZE-offset);
		ret2=send(sockwigi, &send_buff[offset], SEND_BUFFER_SIZE-offset, MSG_DONTWAIT);
		//ret2=send(sockwigi, send_buff, SEND_BUFFER_SIZE, 0);
//		printf("ret2:%d", ret2);
		if(ret2 > 0) {
			rev_ptr = rev_ptr-((ret2)/4);
			

/*
			if(ret2 == SEND_BUFFER_SIZE-offset) {
				rev_ptr = rev_ptr-((ret2+offset)/4);
				offset=0;
			} else {
				offset=offset+ret2;
			}
else if(ret2 == SEND_BUFFER_SIZE-offset) {
				offset=offset+ret2;
				rev_ptr=rev_ptr+(offset/4);
				rev_ptr = rev_ptr-(2* SEND_BUFFER_SIZE/4);
				offset=0;
			}else if(ret2 < SEND_BUFFER_SIZE-offset) {
				offset=offset+ret2;
				rev_ptr=rev_ptr+(offset/4);
			}*/
			//printf("\nwigi:rev_pt:%p and ret:%d", rev_ptr, ret2);fflush(NULL);
//				memcpy(&ref, (int *)&send_buff[offset], sizeof(int));
//				printf("\n\twigi:val:%d and ret:%d nextval:%d", ref, ret2, ref-ret2);fflush(NULL);
		}


		//ret2=send(sockwigi, hellowigi, strlen(hellowigi), 0);
		//printf("Hello message sent\n and %dbytes sent", ret2);
		//valread = read(sockwigi, buffer, 1024);
		//printf("%s\n", buffer);
#endif
	}
	}

	// closing the connected socket
	close(client_fd1);
	close(client_fd2);
	return 0;
}

