#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
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

void client_qname_config(char *client)
{
	memset(client, 0, sizeof(*client));
	/* create client queue for recv. messages from server */
	sprintf(client, "/sp_ex_client_%d", getpid());
	
}

mqd_t mq_client_config(char *client)
{
	struct mq_attr attr;
	mqd_t ret = 0;
	
	/* attributes setup */
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_MESSAGES;
	attr.mq_msgsize = MAX_MSG_SIZE;
	attr.mq_curmsgs = 0;
	
	ret = mq_open (client, O_RDONLY | O_CREAT, QUEUE_PERMISIONS, &attr);
	if (ret == -1) {
		perror("mq_open (client)");
		exit(-1);
	}
	return ret;
}

mqd_t server_config(void)
{
	mqd_t ret = 0;
	ret = mq_open(SQ_NAME, O_WRONLY);
	if (ret == -1) {
		perror("mq_open (server)");
		exit(-1);
	}
	return ret;
}

void token_request(mqd_t client, mqd_t server, char *client_qname)
{
	char in_buffer[MSG_BUFFER_SIZE];
	char temp_buf[10];
	memset(in_buffer, 0, sizeof(in_buffer));
	memset(temp_buf, 0, sizeof(temp_buf));
	
	printf("Request token_flight (<ENTER>): ");
	while (fgets(temp_buf, 2, stdin)) {
		/* send message to server */
		if (mq_send(server, client_qname, strlen(client_qname) + 1, PRIORITY) == -1) {
			perror("mq_send request server");
			continue;
		}
		
		/* receive response from server */
		if (mq_receive(client, in_buffer, MSG_BUFFER_SIZE, PRIORITY) == -1) {
			perror("mq_receive (client)");
			exit(-1);
		}

		/* display token received from server */
		printf("Client: Token flight number received from server: %s\n\n", in_buffer);
		printf("Request for a token flight (Press <Enter>): ");
	}
}

void msg_queue_close(mqd_t client)
{
	if (mq_close(client) == -1) {
		perror("mq_close (client)");
		exit(-1);
	}
}

void msg_queue_unlink(char *qname)
{
	if (mq_unlink(qname) == -1) {
		perror("mq_unlink (client)");
		exit(-1);
	}
}

int main ()
{
	mqd_t qd_server = 0, qd_client = 0;    //queue descriptors
	char client_qname[64];
	
	client_qname_config(client_qname);
	qd_client = mq_client_config(client_qname);
	qd_server = server_config();
	token_request(qd_client, qd_server, client_qname);
	
	msg_queue_close(qd_client);
	msg_queue_unlink(client_qname);
	printf("Client: disconnected... \n");
	return 0;
}
