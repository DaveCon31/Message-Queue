#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SQ_NAME	"/sp_ex_server"

enum { ON  = 1, 
       Q_PERMS = 0660, 
       MAX_MESSAGES = 10, 
       MAX_MSG_SIZE = 1024,
       PRIORITY = 0,
       MSG_BUFFER_SIZE = MAX_MESSAGES + MAX_MSG_SIZE };

int main ()
{
	struct mq_attr attr;
	mqd_t qd_server, qd_client;	//q descriptors
	
	char arr_row[6] = {'A', 'B', 'C', 'D', 'E', 'F'};
	int arr_col[10] = {1, 2, 3, 4, 5 ,6 ,7 ,8, 9, 10};
	int counter_row = 0;
	long token_flight_number = 0;	//token flight number for client
	int checker = 0;
	char in_buffer[MSG_BUFFER_SIZE];
	char out_buffer[MSG_BUFFER_SIZE];
	
	//attributes setup
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_MESSAGES;
	attr.mq_msgsize = MAX_MSG_SIZE;
	attr.mq_curmsgs = 0;
	
	printf("Server: Coming Online!\n");
	qd_server = mq_open (SQ_NAME, O_RDONLY | O_CREAT, Q_PERMS, &attr);
	if (qd_server == -1){
		perror ("Server: mq_open (server)");
		exit (1);
	}
	
	while (ON) 
	{
		if(token_flight_number == 10){
			printf("All seats for this flight are full!!! Preparing tokens for next flight...\n");
			token_flight_number = 0;
		}
		
		// get the oldest message with highest priority
		if (mq_receive(qd_server, in_buffer, MSG_BUFFER_SIZE, PRIORITY) == -1){
			perror("Server: mq_receive");
			exit(1);
		}
	
		printf("Server: request message received!\n");
		
		// send reply message to client
		qd_client = mq_open(in_buffer, O_WRONLY);
		if(qd_client == 1){
			perror("Server: Not able to open client queue");
			continue;
		}
		
		sprintf(out_buffer, "%d%c", arr_col[token_flight_number], arr_row[counter_row]);
		checker = mq_send(qd_client, out_buffer, strlen(out_buffer) + 1, PRIORITY);
		if(checker == -1){
			perror("Server: Not able to send message to client");
			continue;
		}
		
		printf("Server: response sent to client, ticket number %d%c\n\n",arr_col[token_flight_number], arr_row[counter_row]);
		
		counter_row ++;	
				
		if(counter_row == 6){
			counter_row = 0;
			token_flight_number++;
		}
		

	
	}
	
	if(mq_close(qd_server) == -1){
		perror("Server: mq_close");
		exit(1);
	}
	
	if(mq_unlink(SQ_NAME) == -1){
		perror("Server: mq_unlink");
		exit(1);
	}
	
	printf("Server: disconnected... \n");
	exit(0);	
	
	
}
