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
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>






#define BUFFER_SIZE 1024*1024
#define RX_BUFFER_SIZE 1024
//#define EXP_RX_SIZE 900000
//#define EXP_RX_SIZE 1047000 //previous value...
//#define EXP_RX_SIZE 104700000
#define EXP_RX_SIZE 1024*1024
#define MAX_NO_OF_TRIALS 55
//#define MAX_NO_OF_TRIALS 30
//#define EXP_RX_SIZE 1024*50




#define PORT 8080
#define CMD_PORT 8081
//#define WIGIG
#define WIFI

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


int clear_sockets(int wifisocket, int wigisocket) {

	int valreadwigi=100, valreadwifi=100;
    	char rx_buff[RX_BUFFER_SIZE];
	int gosignal=1;
	
	while(valreadwifi > 0 || valreadwigi > 0) {
		valreadwifi = recv(wifisocket, rx_buff, RX_BUFFER_SIZE, MSG_DONTWAIT);
		valreadwigi = recv(wigisocket, rx_buff, RX_BUFFER_SIZE, MSG_DONTWAIT);
	}

	//send clear signal to server






}




int main(int argc, char const* argv[])
{
	int wifiserver_fd, wifiservercmd_fd, wigiserver_fd, wifisocket, wifisocketcmd, wigisocket, valread, valread1, valread2;
	int trialNo=0;
	struct sockaddr_in address1, address2, addresscmd1;
	int opt = 1;
	int addrlen = sizeof(address1);
	char buffer[1024] = { 0 };
	struct ifreq ifr1, ifr2, ifrcmd1;
	char* hello = "Hello from server";
	char buff[BUFFER_SIZE];
    char rx_buff[RX_BUFFER_SIZE];
    char rx_buff_wifi[RX_BUFFER_SIZE];
    char rx_buff_wigig[RX_BUFFER_SIZE];
    fbdt_ack send_ack, rev_ack;
	int *fwd_ptr, *rev_ptr, fwd_count=0, rev_count=0;
	long int val;
	int ref;
	time_t start_time, end_time;
    struct timeval tv;
	double totalbytes=0, bytes=0, wifibytes, wigibytes, throughput, totaltime=0;
	int offset=0, rxsize=RX_BUFFER_SIZE;
	FILE *fpt;
	int filesize, lfilesize, gfilesize, error=10000, enable=1, retmptcp;



	// Creating socket file descriptor
	if ((wifiserver_fd = socket(AF_INET, SOCK_STREAM, 0))
		== 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}


	// Creating socket file descriptor
	if ((wifiservercmd_fd = socket(AF_INET, SOCK_STREAM, 0))
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



/*	if(setsockopt(wifiserver_fd, SOL_SOCKET, SO_RCVLOWAT, &rxsize, sizeof(rxsize)) <0) {
		perror(":wifisock");

	}
	if(setsockopt(wigiserver_fd, SOL_SOCKET, SO_RCVLOWAT, &rxsize, sizeof(rxsize)) < 0) {
		perror(":wifisock");
	}*/

	// Forcefully attaching socket to the port 8080
//#ifdef WIFI
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


// Create socket for CMD port (8081)
	if (setsockopt(wifiservercmd_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	addresscmd1.sin_family = AF_INET;
	addresscmd1.sin_addr.s_addr = INADDR_ANY;
	addresscmd1.sin_port = htons(CMD_PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(wifiservercmd_fd, (struct sockaddr*)&addresscmd1,
			sizeof(address1))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	memset(&ifrcmd1, 0, sizeof(ifr1));
    snprintf(ifrcmd1.ifr_name, sizeof(ifrcmd1.ifr_name), "wlp1s0");
    if (setsockopt(wifiservercmd_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifrcmd1, sizeof(ifrcmd1)) < 0) {
        printf("Unable to bind to interface:%s",ifrcmd1.ifr_name);
        return -1;
    }
//#endif

#ifdef WIGIG

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
#endif

//#ifdef WIFI
	printf("Listening to wifi connection....");fflush(NULL);

	if (listen(wifiserver_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	printf("Listening to wifi-cmd connection....");fflush(NULL);

	if (listen(wifiservercmd_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
//#endif

#ifdef WIGIG


	printf("Listening to wigi connection....");fflush(NULL);

	if (listen(wigiserver_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
#endif

//#ifdef WIFI

	printf("Waiting for wifi to connect....");fflush(NULL);
		if ((wifisocket
			= accept(wifiserver_fd, (struct sockaddr*)&address1,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

	printf("Waiting for wifi cmd to connect....");fflush(NULL);
		if ((wifisocketcmd
			= accept(wifiservercmd_fd, (struct sockaddr*)&address1,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
//#endif

#ifdef WIGIG
	printf("Waiting for wigi to connect....");fflush(NULL);

		if ((wigisocket
			= accept(wigiserver_fd, (struct sockaddr*)&address2,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
#endif

	printf("Rxing data from the client.....");fflush(NULL);

	gettimeofday(&tv, NULL);
    start_time = tv.tv_sec*1000000 + tv.tv_usec;

	fpt = fopen(argv[1], "w+");
	fprintf(fpt,"Trial#,download_time\n");

	int cnt=0;

	while(1) {
	while(trialNo < MAX_NO_OF_TRIALS) {
	while(1) {
	memset(rx_buff,0,RX_BUFFER_SIZE);
	memset(buff,0,BUFFER_SIZE);
	//valread = recv(wifisocketcmd, rx_buff, RX_BUFFER_SIZE, 0);
	//clear_sockets(wifisocket, wigisocket);
	//printf("Waiting for filesize.....");fflush(NULL);
	valread = recv(wifisocketcmd, &filesize, sizeof(int), 0);
	if(valread>0) {
		//printf("\nfiles size %d", lfilesize);fflush(NULL);
		if(filesize==-1) {
			printf("files size %d", gfilesize);fflush(NULL);
			fprintf(fpt,"%d,%d, %.2f\n", trialNo,gfilesize,totaltime);
			gfilesize=filesize;
			totaltime=0;
			cnt=0;
			clear_sockets(wifisocket, wigisocket);
			//exit(1);
			break;
		}
		lfilesize=filesize;
		gfilesize=filesize;
		//error=gfilesize*0.1;
	}

	//printf("\nfile size %d and cnt:%d", filesize, cnt);fflush(NULL);
	cnt++;
	gettimeofday(&tv, NULL);

    	start_time = tv.tv_sec*1000000 + tv.tv_usec;
	totalbytes=0;
	bytes=0;

	fwd_ptr=(int *)&buff[0];
	//rev_ptr=(int *)(&buff[BUFFER_SIZE-8]);
	rev_ptr=(int *)(&buff[filesize-8]);
	//rev_ptr=(int *)(&buff[BUFFER_SIZE-RX_BUFFER_SIZE-8]);

	fwd_count=0;
	rev_count=0;

	wifibytes=0;
	wigibytes=0;

//	printf("\nfwd_ptr:%p and rev_ptr:%p and diff:%ld", fwd_ptr, rev_ptr, rev_ptr-fwd_ptr);fflush(NULL);


	
	while(filesize>0) {
		//printf("ongoing filesize:%d", filesize);fflush(NULL);
        	if(filesize > BUFFER_SIZE) {
                	rev_ptr = (int *) (&buff[BUFFER_SIZE-RX_BUFFER_SIZE-8]);
			fwd_ptr=(int *)&buff[0];
                        filesize=filesize-BUFFER_SIZE;
                } else {
			//if(filesize <= RX_BUFFER_SIZE) {
                       // 	rev_ptr = (int *) (&buff[RX_BUFFER_SIZE-8]);
		//		fwd_ptr=(int *)&buff[0];
		//	} else {
                        	rev_ptr = (int *) (&buff[filesize-8]);
				fwd_ptr=(int *)&buff[0];

		//	}
                	filesize=0;
               }

		while(1) {

#ifdef WIFI
			valread1 = recv(wifisocket, rx_buff_wifi, RX_BUFFER_SIZE, MSG_DONTWAIT);
			if(valread1 > 0 ) {
				memcpy(fwd_ptr,rx_buff_wifi, valread1);
				fwd_ptr=fwd_ptr+(valread1/4);
//				printf("\nwifiValue Rx'ed:%d and totalbytes:%.4f",valread, totalbytes);fflush(NULL);
				//printf("\nwifi:fwd_ptr:%p and rev_ptr:%p", fwd_ptr, rev_ptr);fflush(NULL);
				totalbytes=totalbytes+valread1;
				bytes=bytes+valread1;
				fwd_count=fwd_count+valread1;
				//printf("fwd cnt:%d", fwd_count);fflush(NULL);
				wifibytes=wifibytes+valread1;
				//printf("\nWifibytes:%.1f and totalbytes:%.1f", wifibytes, bytes);	fflush(NULL);

//			if(totalbytes>1040000)
			//if(totalbytes>EXP_RX_SIZE){
				if(fwd_ptr >= rev_ptr) {
            				//gettimeofday(&tv, NULL);
        				//end_time = tv.tv_sec*1000000+tv.tv_usec;
					//printf("\nend time:%ld", end_time);
	    				//printf("\nTime to download %d is %ld", filesize ,(end_time - start_time));fflush(NULL);
					//printf("\nBytes Rx'ed:%f", totalbytes);fflush(NULL);
					//fprintf(fpt,"%d, %ld\n", trialNo, (end_time-start_time));
					//printf("\nstart time:%ld", (end_time-start_time));
					//trialNo++;
					//totalbytes=0;
					//gettimeofday(&tv, NULL);
   				 	//start_time = tv.tv_sec*1000000 + tv.tv_usec;
					break;
				}

			}
#endif

#ifdef WIGIG

			valread2 = recv(wigisocket, rx_buff_wigig, RX_BUFFER_SIZE, MSG_DONTWAIT);
			//valread = recv(wigisocket, rx_buff, RX_BUFFER_SIZE, 0);
			if(valread2 > 0 ) {
				rev_ptr = rev_ptr - valread/4;
				//memcpy(rev_ptr,rx_buff, valread);
				reverse_memcpy(rev_ptr,rx_buff_wigig, valread2);
				wigibytes=wigibytes+valread2;
				bytes=bytes+valread2;
				//printf("\nWigigbytes:%.1f and totalbytes:%.1f", wigibytes, bytes);fflush(NULL);

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
				totalbytes=totalbytes+valread2;

				rev_count=rev_count+valread2;
//			if(totalbytes>1040000)
				//if(totalbytes>EXP_RX_SIZE){
				if(fwd_ptr >= rev_ptr) {
            				//gettimeofday(&tv, NULL);
        				//end_time = tv.tv_sec*1000000+tv.tv_usec;
	    				//printf("\nTime to download %d is %ld", filesize ,(end_time - start_time));fflush(NULL);
					//printf("\nend time:%ld", end_time);
	    			//	printf("\nTime to download %ld is %ld", EXP_RX_SIZE ,(end_time - start_time));fflush(NULL);
					//fprintf(fpt,"%d, %ld\n", trialNo, (end_time-start_time));
					//trialNo++;
					//totalbytes=0;
					//gettimeofday(&tv, NULL);
    					//start_time = tv.tv_sec*1000000 + tv.tv_usec;
					//printf("\nstart time:%ld", start_time);
					break;
				}

			}
#endif

			if(fwd_ptr >= rev_ptr)
				break;

			if(bytes >= (lfilesize - error)) {
			/*printf("lfilesize:%d", lfilesize);fflush(NULL);
			 valread = recv(wifisocketcmd, &filesize, sizeof(int), MSG_DONTWAIT);
			 if(valread > 0) {
        			lfilesize=filesize;
				bytes=0;
				totalbytes=0;
        			if(lfilesize==-1) {
                			printf("files size %d", lfilesize);fflush(NULL);
                			fprintf(fpt,"%d, %.2f\n", trialNo, totaltime);
                			totaltime=0;
                			cnt=0;
					goto nexttrial;
					//exit(1);
			
        			} 
				//exit(1);
				break;
			}*/
			break;
			}


			//gettimeofday(&tv, NULL);
        //end_time = tv.tv_sec;
        	//if((end_time - start_time) >= 1) {
		//	throughput=(bytes*8)/(1000*1000);
            		//printf("\n%fMBits/sec Received....", throughput);fflush(NULL);
			//printf("\nwifiratio:%.1f%% and wigiratio:%.1f%%", (wifibytes/totalbytes)*100, (wigibytes/totalbytes)*100);
			//fprintf(fpt,"%.1f,%.1f,%.1f\n", throughput, (wifibytes/totalbytes)*100, (wigibytes/totalbytes)*100);
            	//	gettimeofday(&tv, NULL);
           // start_time = tv.tv_sec;
            	//	bytes=0;
       		// }

		}
		//	printf("\nValue Rx'e from fwd data transmission:%d, from revers data transmission:%d and totalbytes:%.3f",fwd_count, 
         //              rev_count, totalbytes);fflush(NULL);
//			printf("\nTotalbytes:%.4f wifibytes:%.1f and wigibtes:%.1f", totalbytes, wifibytes, wigibytes);fflush(NULL);
	//		printf("\nwifiratio:%.1f%% and wigiratio:%.1f%%", (wifibytes/totalbytes)*100, (wigibytes/totalbytes)*100);

		//bytes=bytes+totalbytes;
		//totalbytes=0;


	/*gettimeofday(&tv, NULL);
        end_time = tv.tv_sec*1000000+tv.tv_usec;
        if((end_time - start_time) >= 1) {
	    //printf("\nTime to download %d is %ld", EXP_RX_SIZE ,(end_time - start_time));fflush(NULL);
            //printf("\n%fMBits/sec Received....", (bytes*8)/(1000*1000));fflush(NULL);
            fflush(NULL);
            //gettimeofday(&tv, NULL);
            //start_time = tv.tv_sec*1000000+tv.tv_usec;
            bytes=0;
        }*/


		}
		
		gettimeofday(&tv, NULL);
        	end_time = tv.tv_sec*1000000+tv.tv_usec;
		totaltime=totaltime+(end_time-start_time);
	    	//printf("\n[%d]Time to download %d is %ld and Total Time:%f wifibytes:%.1f and wigigbytes:%.1f",cnt, lfilesize ,(end_time - start_time), totaltime, wifibytes, wigibytes);fflush(NULL);

		//fprintf(fpt,"%d, %ld\n", trialNo, (end_time-start_time));
		
	}
nexttrial:	trialNo++;
	printf("\nTrial No....%d", trialNo);fflush(NULL);
	//sleep(1);
	}
	trialNo=0;
	}
	printf("Total Time:%f", totaltime);fflush(NULL);

	fclose(fpt);


//	for(val=0;val<1040000;val=val+4) {
//	for(val=0;val<EXP_RX_SIZE;val=val+4) {
	/*for(val=0;val<totalbytes;val=val+4) {
		memcpy(&ref, (int *)&buff[val], sizeof(int));
		printf("\nvalue at %ld is %d", val, ref);fflush(NULL);
		if(ref!=val) {
			printf("Error in FBDT.....");fflush(NULL);
			//break;
		}
	}*/



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

