#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SQ_NAME	"/sp_ex_server"
#define QUEUE_PERMISIONS 0660
#define PRIORITY 0

enum msg_attr {
	MAX_MESSAGES = 10,
       MAX_MSG_SIZE = 1024,
       MSG_BUFFER_SIZE = MAX_MESSAGES + MAX_MSG_SIZE
};

mqd_t mq_server_config(void)
{
	struct mq_attr attr;
	mqd_t ret = 0;
	
	/* attributes setup */
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_MESSAGES;
	attr.mq_msgsize = MAX_MSG_SIZE;
	attr.mq_curmsgs = 0;
	
	printf("Server: Coming Online!\n");
	ret = mq_open (SQ_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISIONS, &attr);
	if (ret == -1) {
		perror ("mq_open (server)");
		exit (-1);
	}
	return ret;
}

void token_reply(mqd_t server, mqd_t client)
{
	char arr_row[6] = {'A', 'B', 'C', 'D', 'E', 'F'};
	int arr_col[10] = {1, 2, 3, 4, 5 ,6 ,7 ,8, 9, 10};
	int token_flight_number = 0;    //token flight number for client
	int counter_row = 0;
	int ret = 0;
	char in_buffer[MSG_BUFFER_SIZE];
	char out_buffer[MSG_BUFFER_SIZE];
	memset(in_buffer, 0, sizeof(in_buffer));
	memset(out_buffer, 0, sizeof(out_buffer));

	while (1) {
		if (token_flight_number == 10) {
			printf("All seats for this flight are full!!! Preparing tokens for next flight...\n");
			token_flight_number = 0;
		}

		/* get the oldest message with highest priority */
		if (mq_receive(server, in_buffer, MSG_BUFFER_SIZE, PRIORITY) == -1) {
			perror("mq_receive (server)");
			exit(-1);
		}
		printf("Server: request message received!\n");

		/* send reply message to client */
		client = mq_open(in_buffer, O_WRONLY);
		if (client == -1) {
			perror("mq_open (client)");
			continue;
		}
		
		sprintf(out_buffer, "%d%c", arr_col[token_flight_number], arr_row[counter_row]);
		ret = mq_send(client, out_buffer, strlen(out_buffer) + 1, PRIORITY);
		if (ret == -1) {
			perror("mq_send (server)");
			continue;
		}

		printf("Server: response sent to client, ticket number %d%c\n\n",arr_col[token_flight_number], arr_row[counter_row]);
		counter_row ++;		
		if (counter_row == 6) {
			counter_row = 0;
			token_flight_number++;
		}
	}
}

void msg_queue_close(mqd_t server)
{
	if (mq_close(server) == -1) {
		perror("mq_close (server)");
		exit(-1);
	}
}

void msg_queue_unlink(void)
{
	if (mq_unlink(SQ_NAME) == -1) {
		perror("mq_unlink (server)");
		exit(-1);
	}
}

int main ()
{
	mqd_t qd_server = 0, qd_client = 0;    //queue descriptors

	qd_server = mq_server_config();
	token_reply(qd_server, qd_client);

	msg_queue_close(qd_server);
	msg_queue_unlink();
	printf("Server: disconnected... \n");
	return 0;		
}
