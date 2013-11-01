#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "header.h"

extern int errno;

static void populate_sock(struct sock_cl *s) 
{

	if ((s->sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(&s->sin, 0, sizeof(s->sin));
	s->sin.sin_family = AF_INET;
	s->sin.sin_addr.s_addr = inet_addr(hostname);
	s->sin.sin_port = htons(s->port);
	s->addrlen = sizeof(s->sin); 

}

int main(int argc, char **argv) 
{
	char dir[BUFSIZ];
	struct sock_cl s;
	pid_t child_id;
	int addrlen;
	int status;
	struct question *q; /* are we going to recycle the same data structure over and over again */	

	if(argc != 3) {
		fprintf(stderr, "USAGE: ./udp_client ip_address port_number\n");
		return -1;
	}

	if(argv[1]) 
		strcpy(hostname, argv[1]);
	else
		strcpy(hostname, HOST);
	if(argv[2]) 
		s.port = atoi(argv[2]);
	else
		s.port = port;

	while(obtain_req_type(dir) != 0){
		fprintf(stderr, "Still no request for client\n");
		sleep(2);
	}	
	populate_sock(&s);
	addrlen = s.addrlen; 

	ap_debug("====>Client side port = %d", port);

	/* All transaction are started by client, so it is client which will either ask 
	 * 1. server to send the question or 
	 * 2. process the question send by the user.
	 * in case 1. the user has already asked the question and waiting for response
	 * in case 2. user pushes a question. server sends it to another client, who wants to answer the question
	 * server waits for the response from the user, as soon as response is available, 
	 * the server pushes the question back to first client
	 */
	ap_debug("%p\n",&s.sin);
	while(1) {
		switch(query_type(dir))
		{
				/*  Begin set 1 reqQ-saveA */
			case REQ_QUESTION:
				strcpy(s.buffer, dir);
				client_req_ques(&s);
				q = (struct question *)malloc(sizeof(struct question));
				populate_ques_ds(question_file, q);
				strcpy(dir, s.buffer);
				break;
				/* set 2 reqA-saveQ End */
			case REQ_ANSWER:
				strcpy(s.buffer, dir);
				client_req_ans(&s);
				strcpy(dir, req_type[REQ_STOP]);
				break;
				/* Begin set 2 saveQ-reqA */
			case SAVE_QUESTION:
				/* CAUTION dir and s.buffer contain different values */
				memset(s.buffer, 0, sizeof(s.buffer));
				/* open the question file and populate question data structure */
				q = (struct question *)malloc(sizeof(struct question));
				populate_ques_ds(question_file, q);
				create_qbuf(q,s.buffer);
				client_save_ques(&s, dir); /* dir is populated inside here */
				free_question(q);
				break;
				/* set 1 saveA-reqQ End */
			case SAVE_ANSWER:
				if( access( answer_file, F_OK ) == -1 ) 
						continue;
				/* obtain answer in the buffer */
				/* open the answer file and fill the buffer */
				memset(s.buffer, 0, sizeof(s.buffer));
				populate_if_match(q, answer_file); /* obtain data structure for question */
				create_abuf(q, s.buffer);
				client_save_ans(&s, dir);
				strcpy(dir, req_type[REQ_STOP]);
				free_question(q);
				break;
			case REQ_STOP:
				/* do any cleanup here? */
				return 0;
			case REQ_LOOP:
				/* do any cleanup here? */
				break;
			default:
			case -1:
				fprintf(stderr, "What the fuck bro?\n");
				exit(EXIT_FAILURE);

		}

	sleep(1);
	}

	return 0;
}
