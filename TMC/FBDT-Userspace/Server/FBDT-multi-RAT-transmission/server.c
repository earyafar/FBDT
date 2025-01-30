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
#include <errno.h>
#include <string.h>






#define PORT 8080


#define BUFFER_SIZE 1024*1024
//#define BUFFER_SIZE 1024*50
#define SEND_BUFFER_SIZE 1024 * 50
#define ENABLE_BACKWARD_TRANSMISSION
#define ENABLE_FORWARD_TRANSMISSION
#define ENABLE_SECOND_BACKWARD_TRANSMISSION
#define ENABLE_THIRD_BACKWARD_TRANSMISSION



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
	int sockwifi=0, valread, client_fd1, client_fd2,client_fd3, sockwigi=0, socketh=0;
	struct sockaddr_in serv_addr1, serv_addr2, serv_addr3;
	char* hellowifi = "Hello from wificlient";
	char* hellowigi = "Hello from wigiclient";
	char* helloeth = "Hello from wigieth";
	char buffer[1024] = { 0 };
	struct ifreq ifr1, ifr2, ifr3;
	int ret1, ret2, ret3;
	int val, val1;
	int *fwd_ptr, *sec_rev_ptr, *rev_ptr, offset=0;
	int *ref_fwd_ptr, *ref_sec_rev_ptr, *ref_rev_ptr;
	char buff[BUFFER_SIZE];
	char send_buff[SEND_BUFFER_SIZE];
	fbdt_ack send_ack, sec_rev_ack, rev_ack;
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

	if ((socketh = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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

	serv_addr3.sin_family = AF_INET;
	serv_addr3.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "192.168.20.30", &serv_addr3.sin_addr)
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

	memset(&ifr3, 0, sizeof(ifr3));
	snprintf(ifr3.ifr_name, sizeof(ifr3.ifr_name), "enx681ca2121563");
	if (setsockopt(socketh, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr3, sizeof(ifr3)) < 0) {
		printf("Unable to bind to interface:%s",ifr3.ifr_name);
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
	if ((client_fd3
		= connect(socketh, (struct sockaddr*)&serv_addr3,
				sizeof(serv_addr3)))
		< 0) {
		printf("\nEth Connection Failed \n");
		return -1;
	}



	while(1) {
	fwd_ptr=(int *)&buff[0];
	ref_fwd_ptr=(int *)&buff[0];
    sec_rev_ptr = (int *) (&buff[(BUFFER_SIZE/2)-SEND_BUFFER_SIZE-8]);
    ref_sec_rev_ptr = (int *) (&buff[(BUFFER_SIZE/2)-SEND_BUFFER_SIZE-8]);
    rev_ptr = (int *) (&buff[BUFFER_SIZE-SEND_BUFFER_SIZE-8]);
    ref_rev_ptr = (int *) (&buff[BUFFER_SIZE-SEND_BUFFER_SIZE-8]);
	offset=0;

//	printf("\nfwd_ptr:%p sec_rev_ptr:%p and rev_ptr:%p ", fwd_ptr, sec_rev_ptr, rev_ptr);fflush(NULL);
//	printf("\nfwd_ptr:%d sec_rev_ptr:%d and rev_ptr:%d ", *(int*)fwd_ptr, *(int *)sec_rev_ptr+1, *(int *)rev_ptr);fflush(NULL);

//    while(fwd_ptr<=sec_rev_ptr && sec_rev_ptr<=rev_ptr && fwd_ptr!=NULL && sec_rev_ptr!=NULL && rev_ptr!=NULL) {

#ifdef ENABLE_FORWARD_TRANSMISSION
		//send forward data transmission.....
		/*if(fwd_ptr+SEND_BUFFER_SIZE == NULL)
			break;*/ //sept 10th 
		memcpy(send_buff, fwd_ptr, SEND_BUFFER_SIZE);
		ret1=send(sockwifi, send_buff, SEND_BUFFER_SIZE, MSG_DONTWAIT);
//		printf("ret1:%d", ret1);fflush(NULL);
//		printf("wfi ret1:%d and %s", ret1, strerror(errno));fflush(NULL);
		if(ret1 > 0){
			fwd_ptr=fwd_ptr+(ret1/4);
//			printf("\nwifi:fwd_pt:%p and ret:%d", fwd_ptr, ret1);fflush(NULL);
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

#ifdef ENABLE_SECOND_BACKWARD_TRANSMISSION		
//		memcpy(send_buff, (rev_ptr+(offset/4)), SEND_BUFFER_SIZE-offset);
		reverse_memcpy(send_buff, sec_rev_ptr, SEND_BUFFER_SIZE-offset);
		ret2=send(socketh, &send_buff[offset], SEND_BUFFER_SIZE-offset, MSG_DONTWAIT);
		//ret2=send(sockwigi, send_buff, SEND_BUFFER_SIZE, 0);
//		printf("eth ret2:%d and %s", ret2, strerror(errno));fflush(NULL);
		if(ret2 > 0) {
			//sec_rev_ptr = sec_rev_ptr-((ret2)/4);
			sec_rev_ptr = sec_rev_ptr-((ret2)/4);
//			printf("\neth:sec_rev_pt:%p and ret:%d", sec_rev_ptr, ret2);fflush(NULL);
		}
#endif
		//send reverse data transmission.....
		/*if(sec_rev_ptr-SEND_BUFFER_SIZE == NULL)
			break;*/ //sept 10th

#ifdef ENABLE_THIRD_BACKWARD_TRANSMISSION		
//		memcpy(send_buff, (rev_ptr+(offset/4)), SEND_BUFFER_SIZE-offset);
		reverse_memcpy(send_buff, rev_ptr, SEND_BUFFER_SIZE-offset);
		ret3=send(sockwigi, &send_buff[offset], SEND_BUFFER_SIZE-offset, MSG_DONTWAIT);
		//ret2=send(sockwigi, send_buff, SEND_BUFFER_SIZE, 0);
//		printf("ret3:%d", ret3);fflush(NULL);
//		printf("wigi ret3:%d and %s", ret3, strerror(errno));fflush(NULL);
		if(ret3 > 0) {
			rev_ptr = rev_ptr-((ret3)/4);
//			printf("\nwigi:sec_rev_pt:%p and ret:%d", rev_ptr, ret3);fflush(NULL);
			

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
	if(ref_sec_rev_ptr>=rev_ptr) {
//		printf("rev_ptr is > ref_sec pointer...");fflush(NULL);
		rev_ptr = sec_rev_ptr;
		sec_rev_ptr=fwd_ptr+((sec_rev_ptr-fwd_ptr)/2);
		ref_sec_rev_ptr=fwd_ptr+((sec_rev_ptr-fwd_ptr)/2);
	} else if(fwd_ptr >= sec_rev_ptr) {
//		printf("sec_rev_ptr is > fwd_ptr");fflush(NULL);
		fwd_ptr=ref_sec_rev_ptr;
		ref_fwd_ptr=ref_sec_rev_ptr;
		sec_rev_ptr=fwd_ptr+((rev_ptr-fwd_ptr)/2);
		ref_sec_rev_ptr=fwd_ptr+((rev_ptr-fwd_ptr)/2);
//		printf("fwd:%p sec:%p and rev:%p", fwd_ptr, sec_rev_ptr, rev_ptr);fflush(NULL);
	}

//	}
	}

	// closing the connected socket
	close(client_fd1);
	close(client_fd2);
	close(client_fd3);
	return 0;
}

