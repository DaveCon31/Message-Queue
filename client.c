#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
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
	mqd_t qd_server, qd_client;	//queue descriptors
	
	char client_qname[64];
	char in_buffer[MSG_BUFFER_SIZE];
	char temp_buf[10];
	
	//attributes setup
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_MESSAGES;
	attr.mq_msgsize = MAX_MSG_SIZE;
	attr.mq_curmsgs = 0;
	
	//create client queue for recv. messages from server
	sprintf(client_qname, "/sp_ex_client_%d", getpid());

	qd_client = mq_open (client_qname, O_RDONLY | O_CREAT, Q_PERMS, &attr);
	if(qd_client == -1){
		perror("Client: mq_open (client)");
		exit(1);
	}
	
	qd_server = mq_open(SQ_NAME, O_WRONLY);
	if(qd_server == -1){
		perror("Client: mq_open (server)");
		exit(1);
	}
	
	printf("Request token_flight (<ENTER>): ");
	
	while (fgets(temp_buf, 2, stdin)) 
	{
		//send message to server
		if(mq_send(qd_server, client_qname, strlen(client_qname) + 1, PRIORITY) == -1)
		{
			perror("Client: Not able to send request to server");
			continue;
		}
		
		//receive response from server
		if(mq_receive(qd_client, in_buffer, MSG_BUFFER_SIZE, PRIORITY) == -1)
		{
			perror("Client: mq_receive");
			exit(1);
		}
		
		//display token received from server
		printf("Client: Token flight number received from server: %s\n\n", in_buffer);
		
		printf("Request for a token flight (Press <Enter>): ");
	}
	
	if(mq_close(qd_client) == -1) 
	{
		perror("Client: mq_close");
		exit(1);
	}
	
	if(mq_unlink(client_qname) == -1)
	{
		perror("Client: mq_unlink");
		exit(1);
	}
		
	printf("Client: disconnected... \n");
	
	exit(0);
	
}
	
	
	
	
