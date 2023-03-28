/* Forward and Backward Data Transmission Code
 * Author: Suresh Srinivasan
 * Email: suresh2@pdx.edu
 */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/if.h>
#include <time.h>
#include <sys/time.h>



#define BUFFER_SIZE 1024*1024
#define RX_BUFFER_SIZE 1024
//#define EXP_RX_SIZE 900000
#define EXP_RX_SIZE 1047000
//#define EXP_RX_SIZE 1024*50

#define PORT 8080

typedef struct data_ack {
    int ack;
    int rev_ack;
}fbdt_ack;

void* reverse_memcpy(
void *dest, const void *src, size_t len)
{

    char *d = dest;
    const char *s = src+len-1;
    while(len--)
        *d++ = *s--;
    return dest;
}





int main(int argc, char const* argv[])
{
	int wifiserver_fd, wigiserver_fd, ethserver_fd, wifisocket, wigisocket, ethsocket, valread;
	struct sockaddr_in address1, address2, address3;
	int opt = 1;
	int addrlen = sizeof(address1);
	char buffer[1024] = { 0 };
	struct ifreq ifr1, ifr2, ifr3;
	char* hello = "Hello from server";
	char buff[BUFFER_SIZE];
    char rx_buff[RX_BUFFER_SIZE];
    fbdt_ack send_ack, sec_rev_ack, rev_ack;
	int *fwd_ptr, *sec_rev_ptr, *rev_ptr, *ref_fwd_ptr, fwd_count=0, rev_count=0;
	long int val;
	int ref;
	time_t start_time, end_time;
    struct timeval tv;
	double totalbytes, bytes, wifibytes, wigibytes, ethbytes, throughput;
	int offset=0, rxsize=RX_BUFFER_SIZE, seqno;
	FILE *fpt;



	// Creating socket file descriptor
	if ((wifiserver_fd = socket(AF_INET, SOCK_STREAM, 0))
		== 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Creating socket file descriptor
	if ((wigiserver_fd = socket(AF_INET, SOCK_STREAM, 0))
		== 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	// Creating socket file descriptor
	if ((ethserver_fd = socket(AF_INET, SOCK_STREAM, 0))
		== 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
/*	if(setsockopt(wifiserver_fd, SOL_SOCKET, SO_RCVLOWAT, &rxsize, sizeof(rxsize)) <0) {
		perror(":wifisock");

	}
	if(setsockopt(wigiserver_fd, SOL_SOCKET, SO_RCVLOWAT, &rxsize, sizeof(rxsize)) < 0) {
		perror(":wifisock");
	}*/

	// Forcefully attaching socket to the port 8080
	if (setsockopt(wifiserver_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address1.sin_family = AF_INET;
	address1.sin_addr.s_addr = INADDR_ANY;
	address1.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(wifiserver_fd, (struct sockaddr*)&address1,
			sizeof(address1))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	memset(&ifr1, 0, sizeof(ifr1));
    snprintf(ifr1.ifr_name, sizeof(ifr1.ifr_name), "wlp1s0");
    if (setsockopt(wifiserver_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr1, sizeof(ifr1)) < 0) {
        printf("Unable to bind to interface:%s",ifr1.ifr_name);
        return -1;
    }
	

	// Forcefully attaching socket to the port 8080
	if (setsockopt(wigiserver_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address2.sin_family = AF_INET;
	address2.sin_addr.s_addr = INADDR_ANY;
	address2.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(wigiserver_fd, (struct sockaddr*)&address2,
			sizeof(address2))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	memset(&ifr2, 0, sizeof(ifr2));
    snprintf(ifr2.ifr_name, sizeof(ifr2.ifr_name), "wlp2s0");
    if (setsockopt(wigiserver_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr2, sizeof(ifr2)) < 0) {
        printf("Unable to bind to interface:%s",ifr1.ifr_name);
        return -1;
    }
// Forcefully attaching socket to the port 8080
    if (setsockopt(ethserver_fd, SOL_SOCKET,
                SO_REUSEADDR | SO_REUSEPORT, &opt,
                sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address3.sin_family = AF_INET;
    address3.sin_addr.s_addr = INADDR_ANY;
    address3.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(ethserver_fd, (struct sockaddr*)&address3,
            sizeof(address3))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    memset(&ifr3, 0, sizeof(ifr3));
    snprintf(ifr3.ifr_name, sizeof(ifr3.ifr_name), "enp0s31f6");
    if (setsockopt(ethserver_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr3, sizeof(ifr3)) < 0) {
        printf("Unable to bind to interface:%s",ifr3.ifr_name);
        return -1;
    }


	printf("Listening to wifi connection....");fflush(NULL);

	if (listen(wifiserver_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	printf("Listening to wigi connection....");fflush(NULL);

	if (listen(wigiserver_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Listening to ethernet connection....");fflush(NULL);

	if (listen(ethserver_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	printf("Waiting for wifi to connect....");fflush(NULL);
		if ((wifisocket
			= accept(wifiserver_fd, (struct sockaddr*)&address1,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
	printf("Waiting for wigi to connect....");fflush(NULL);

		if ((wigisocket
			= accept(wigiserver_fd, (struct sockaddr*)&address2,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
	printf("Waiting for eth to connect....");fflush(NULL);

		if ((ethsocket
			= accept(ethserver_fd, (struct sockaddr*)&address3,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

	printf("Rxing data from the client.....");fflush(NULL);

	gettimeofday(&tv, NULL);
    start_time = tv.tv_sec;

	fpt = fopen(argv[1], "w+");
	fprintf(fpt,"Throughput, wifiratio, wigiratio\n");



	while(1) {
	memset(rx_buff,0,RX_BUFFER_SIZE);
	memset(buff,0,BUFFER_SIZE);

	fwd_ptr=(int *)&buff[0];
	ref_fwd_ptr=(int *)&buff[0];
	sec_rev_ptr=(int *)(&buff[(BUFFER_SIZE/2)-8]);
	rev_ptr=(int *)(&buff[BUFFER_SIZE-8]);
	//rev_ptr=(int *)(&buff[BUFFER_SIZE-RX_BUFFER_SIZE-8]);

	fwd_count=0;
	rev_count=0;

	wifibytes=0;
	wigibytes=0;

//	printf("\nfwd_ptr:%p and rev_ptr:%p and diff:%ld", fwd_ptr, rev_ptr, rev_ptr-fwd_ptr);fflush(NULL);


	

	while(1) {


		valread = recv(wifisocket, rx_buff, RX_BUFFER_SIZE, MSG_DONTWAIT);
		if(valread > 0 ) {
			memcpy(&seqno, (int *)&rx_buff[0], sizeof(int));
			if(fwd_ptr!= (ref_fwd_ptr+(seqno/4))) {
				fwd_ptr=ref_fwd_ptr+(seqno/4);
			}
			memcpy(fwd_ptr,rx_buff, valread);
			fwd_ptr=fwd_ptr+(valread/4);
			
			//printf("\nwifiValue Rx'ed:%d and totalbytes:%.4f",valread, totalbytes);fflush(NULL);
			//printf("\nwifi:fwd_ptr:%p and rev_ptr:%p", fwd_ptr, rev_ptr);fflush(NULL);
			totalbytes=totalbytes+valread;
			bytes=bytes+valread;
			fwd_count=fwd_count+valread;
			wifibytes=wifibytes+valread;

//			if(totalbytes>1040000)
			if(totalbytes>EXP_RX_SIZE)
				break;

		}

       valread = recv(ethsocket, rx_buff, RX_BUFFER_SIZE, MSG_DONTWAIT);
        //valread = recv(wigisocket, rx_buff, RX_BUFFER_SIZE, 0);
        if(valread > 0 ) {
			reverse_memcpy(&seqno, (int *)&rx_buff[valread/4], sizeof(int));
            if(sec_rev_ptr!= (ref_fwd_ptr+(seqno/4))) {
                if(seqno>=0 && seqno<=BUFFER_SIZE) //This patch is temp fix, sometimes there is junk seq no.....need to fix this issue....
                    sec_rev_ptr=ref_fwd_ptr+(seqno/4);
                //printf("\nrev_ptr:%ld and seqno:%d", (rev_ptr-ref_fwd_ptr)*4, seqno);fflush(NULL);
                //reverse_memcpy(&seqno, (int *)&rx_buff[0], sizeof(int));
                //printf("\nrev_ptr:%ld and last seqno:%d", (rev_ptr-ref_fwd_ptr)*4, seqno);fflush(NULL);
            }

            /*memcpy(&seqno, (int *)&rx_buff[0], sizeof(int));
            if(sec_rev_ptr!= (ref_fwd_ptr+(seqno/4))) {
                sec_rev_ptr=ref_fwd_ptr+(seqno/4);
            }*/
            sec_rev_ptr = sec_rev_ptr - valread/4;
            reverse_memcpy(sec_rev_ptr,rx_buff, valread);
            ethbytes=ethbytes+valread;
            bytes=bytes+valread;

            //printf("\nethValue Rx'ed:%d and totalbytes:%.3f",valread, totalbytes);fflush(NULL);
//          memcpy(&ref, (int *)&rx_buff[val], sizeof(int));
//          printf("\nbytes rxed %d and top value:%d and next:%d", valread, ref, ref-valread);fflush(NULL);

/*          if(valread == RX_BUFFER_SIZE) {
                rev_ptr=rev_ptr-(valread/4);
                memcpy(rev_ptr,rx_buff, valread);
                printf("\nwigiValue Rx'ed:%d and totalbytes:%.3f",valread, totalbytes);fflush(NULL);
                offset=0;
            } else if(valread == RX_BUFFER_SIZE -offset) {
                offset=offset+valread;
                rev_ptr=rev_ptr+(offset/4);
                memcpy(rev_ptr,rx_buff, valread);
                printf("\nwigiValue Rx'ed:%d and totalbytes:%.3f",valread, totalbytes);fflush(NULL);
                rev_ptr=rev_ptr+(2*RX_BUFFER_SIZE/4);


            } else if(valread< RX_BUFFER_SIZE) {
                offset=offset+valread;
                rev_ptr=rev_ptr+(offset/4);
                memcpy(rev_ptr,rx_buff, valread);
                printf("\nwigiValue Rx'ed:%d and totalbytes:%.3f",valread, totalbytes);fflush(NULL);
            }*/
            //printf("\nwigi:fwd_ptr:%p and rev_ptr:%p", fwd_ptr, rev_ptr);fflush(NULL);
            totalbytes=totalbytes+valread;

            rev_count=rev_count+valread;
//          if(totalbytes>1040000)
            if(totalbytes>EXP_RX_SIZE)
                break;

        }


		valread = recv(wigisocket, rx_buff, RX_BUFFER_SIZE, MSG_DONTWAIT);
		//valread = recv(wigisocket, rx_buff, RX_BUFFER_SIZE, 0);
		if(valread > 0 ) {
			reverse_memcpy(&seqno, (int *)&rx_buff[valread/4], sizeof(int));
			if(rev_ptr!= (ref_fwd_ptr+(seqno/4))) {
				if(seqno>=0 && seqno<=BUFFER_SIZE) //This patch is temp fix, sometimes there is junk seq no.....need to fix this issue....
					rev_ptr=ref_fwd_ptr+(seqno/4);
				//printf("\nrev_ptr:%ld and seqno:%d", (rev_ptr-ref_fwd_ptr)*4, seqno);fflush(NULL);
				//reverse_memcpy(&seqno, (int *)&rx_buff[0], sizeof(int));
				//printf("\nrev_ptr:%ld and last seqno:%d", (rev_ptr-ref_fwd_ptr)*4, seqno);fflush(NULL);
			}
			rev_ptr = rev_ptr - valread/4;
			reverse_memcpy(rev_ptr,rx_buff, valread);
			wigibytes=wigibytes+valread;
			bytes=bytes+valread;
			
			//printf("\nwigiValue Rx'ed:%d and totalbytes:%.3f",valread, totalbytes);fflush(NULL);
//			memcpy(&ref, (int *)&rx_buff[val], sizeof(int));
//			printf("\nbytes rxed %d and top value:%d and next:%d", valread, ref, ref-valread);fflush(NULL);

/*			if(valread == RX_BUFFER_SIZE) {
				rev_ptr=rev_ptr-(valread/4);
				memcpy(rev_ptr,rx_buff, valread);
				printf("\nwigiValue Rx'ed:%d and totalbytes:%.3f",valread, totalbytes);fflush(NULL);
				offset=0;
			} else if(valread == RX_BUFFER_SIZE -offset) {
				offset=offset+valread;
				rev_ptr=rev_ptr+(offset/4);
				memcpy(rev_ptr,rx_buff, valread);
				printf("\nwigiValue Rx'ed:%d and totalbytes:%.3f",valread, totalbytes);fflush(NULL);
				rev_ptr=rev_ptr+(2*RX_BUFFER_SIZE/4);


		    } else if(valread< RX_BUFFER_SIZE) {
				offset=offset+valread;
				rev_ptr=rev_ptr+(offset/4);
				memcpy(rev_ptr,rx_buff, valread);
				printf("\nwigiValue Rx'ed:%d and totalbytes:%.3f",valread, totalbytes);fflush(NULL);
			}*/
			//printf("\nwigi:fwd_ptr:%p and rev_ptr:%p", fwd_ptr, rev_ptr);fflush(NULL);
			totalbytes=totalbytes+valread;

			rev_count=rev_count+valread;
//			if(totalbytes>1040000)
			if(totalbytes>EXP_RX_SIZE)
				break;

		}
/*		if(fwd_ptr >= sec_rev_ptr && sec_rev_ptr >= rev_ptr)
			break;*/

		gettimeofday(&tv, NULL);
        end_time = tv.tv_sec;
        if((end_time - start_time) >= 1) {
			throughput=(bytes*8)/(1000*1000);
            printf("\n%fMBits/sec Received....", throughput);fflush(NULL);
			//printf("\nwifiratio:%.1f%% and wigiratio:%.1f%%", (wifibytes/totalbytes)*100, (wigibytes/totalbytes)*100);
            fflush(NULL);
			fprintf(fpt,"%.1f,%.1f,%.1f\n", throughput, (wifibytes/totalbytes)*100, (wigibytes/totalbytes)*100);
            gettimeofday(&tv, NULL);
            start_time = tv.tv_sec;
            bytes=0;
        }

	}
		//	printf("\nValue Rx'e from fwd data transmission:%d, from revers data transmission:%d and totalbytes:%.3f",fwd_count, 
         //              rev_count, totalbytes);fflush(NULL);
//			printf("\nTotalbytes:%.4f wifibytes:%.1f and wigibtes:%.1f", totalbytes, wifibytes, wigibytes);fflush(NULL);
	//		printf("\nwifiratio:%.1f%% and wigiratio:%.1f%%", (wifibytes/totalbytes)*100, (wigibytes/totalbytes)*100);

		//bytes=bytes+totalbytes;
//		totalbytes=0;


		gettimeofday(&tv, NULL);
        end_time = tv.tv_sec;
        if((end_time - start_time) >= 1) {
            printf("\n%fMBits/sec Received....", (bytes*8)/(1000*1000));fflush(NULL);
            fflush(NULL);
            gettimeofday(&tv, NULL);
            start_time = tv.tv_sec;
            bytes=0;
        }
		break;


	}

	fclose(fpt);


//	for(val=0;val<1040000;val=val+4) {
//	for(val=0;val<EXP_RX_SIZE;val=val+4) {
	for(val=0;val<totalbytes;val=val+4) {
		memcpy(&ref, (int *)&buff[val], sizeof(int));
		printf("\nvalue at %ld is %d", val, ref);fflush(NULL);
		if(ref!=val) {
			printf("Error in FBDT.....");fflush(NULL);
			//break;
		}
	}



	//valread = read(new_socket, buffer, 1024);
	//send(new_socket, hello, strlen(hello), 0);
	//printf("%s\n", buffer);fflush(NULL);

//	send(new_socket, hello, strlen(hello), 0);
//	printf("Hello message sent\n");

// closing the connected socket
	close(wifisocket);
	close(wigisocket);
// closing the listening socket
	shutdown(wifiserver_fd, SHUT_RDWR);
	return 0;
}

