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

/* REPLACE with your server machine name*/
#define HOST        "127.0.0.1"
char *qfilename = "question.txt";
char *afilename = "answer.txt";
static char hostname [BUFSIZ];
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

int client_req_ques(struct sock_in *s)
{
	return 0;
}

int client_req_ans(struct sock_in *s)
{
	return 0;
}

int client_save_ans(struct sock_in *s)
{
	return 0;
}

int client_save_ques(struct sock_in *s)
{
	return 0;
}

int main(int argc, char **argv) 
{
	char dir[BUFSIZ];
	struct sock_cl s;
	pid_t child_id;
	int addrlen;
	int status;
	struct question *q; /* are we going to recycle the same data structure over and over again */	

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

	printf(dir, "%s",argv[1]);
	/* All transaction are started by client, so it is client which will either ask 
	 * 1. server to send the question or 
	 * 2. process the question send by the user.
	 * in case 1. the user has already asked the question and waiting for response
	 * in case 2. user pushes a question. server sends it to another client, who wants to answer the question
	 * server waits for the response from the user, as soon as response is available, the server pushes the question back to first client
	 */
	while(1) {
		switch(query_type(dir))
		{

			case REQ_QUESTION:
				/* req */
				/* send a message to the server PORT on machine HOST */
				/* obtain question to be asked */
				/* req server for a question */		
				if ((status  = sendto(s.sd, dir, strlen(dir), 0, (struct sockaddr *)&(s.sin), addrlen))== -1) {
					fprintf(stderr,"%s %d",strerror(errno), __LINE__);
					exit(1);
				}
				printf("%s,%d\n","status=", status);

				/* spew-out the results and bail out of here! */
				memset(dir, 0, sizeof(dir));
				/* wait for a message to come back from the server */
				if (recvfrom(s.sd, dir, BUFSIZ, 0, (struct sockaddr *)&(s.pin), &addrlen) == -1) {
					fprintf(stderr,"%s %d",strerror(errno), __LINE__);
					exit(1);
				}
				dump_buffer_to_file(dir, qfilename);
				printf("message from server = %s\n", dir);
				/* question has been created to be displayed to the user */
				/* now change the buffer to send answer with save answer*/
				memset(dir, 0, sizeof(dir));
				strcpy(dir, req_type[SAVE_ANSWER]); /* switch to answering mode */
				q = (struct question *)malloc(sizeof(struct question));
				populate_ques_ds(qfilename, q);

				break;
			case REQ_ANSWER:
				/* display answer to the the user after it is obtained */
				/* server would send the answer back correspondin to the question, 
 				* the client must create an answer file corresponding to the answer */
				if ((status  = sendto(s.sd, dir, strlen(dir), 0, (struct sockaddr *)&(s.sin), addrlen))== -1) {
					fprintf(stderr,"%s %d",strerror(errno), __LINE__);
					exit(1);
				}
				memset(dir, 0, sizeof(dir));

				if (recvfrom(s.sd, dir, BUFSIZ, 0, (struct sockaddr *)&(s.pin), &addrlen) == -1) {
					fprintf(stderr,"%s %d",strerror(errno), __LINE__);
					exit(1);
				}
				dump_buffer_to_file(dir, afilename);
				printf("message from server = %s\n", dir);

				break;
			case SAVE_QUESTION:
				/* the user created the question now transport it to server */
				if ((status  = sendto(s.sd, dir, strlen(dir), 0, (struct sockaddr *)&(s.sin), addrlen))== -1) {
					fprintf(stderr,"%s %d",strerror(errno), __LINE__);
					exit(1);
				}

				memset(dir, 0, sizeof(dir));
				/* open the question file and populate question data structure */
				q = (struct question *)malloc(sizeof(struct question));
				populate_ques_ds(qfilename, q);
				create_qbuf(q,dir);
				/* the user created the question now transport it to server */
				if ((status  = sendto(s.sd, dir, strlen(dir), 0, (struct sockaddr *)&(s.sin), addrlen))== -1) {
					fprintf(stderr,"%s %d",strerror(errno), __LINE__);
					exit(1);
				}
				memset(dir, 0, sizeof(dir));
				strcpy(dir, req_type[REQ_ANSWER]);
				break;
			case SAVE_ANSWER:
				if( access( afilename, F_OK ) == -1 ) 
						continue;
				/* the user created the answer now transport it to server */
				if ((status  = sendto(s.sd, dir, strlen(dir), 0, (struct sockaddr *)&(s.sin), addrlen))== -1) {
					fprintf(stderr,"%s %d",strerror(errno), __LINE__);
					exit(1);
				}
				/* obtain answer in the buffer */
				/* open the answer file and fill the buffer */
				memset(dir, 0, sizeof(dir));
				populate_if_match(q, afilename); /* obtain data structure for question */
				create_abuf(q, dir);
				if ((status  = sendto(s.sd, dir, strlen(dir), 0, (struct sockaddr *)&(s.sin), addrlen))== -1) {
					fprintf(stderr,"%s %d",strerror(errno), __LINE__);
					exit(1);
				}
				/* obtain answer in the buffer */
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
