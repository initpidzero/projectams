/* Author - anuz
* This file is distibuted under GPLv2
*/

#include <stdio.h>
#include "header.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned short int port = 0x9123;

void log_stuff(const char *message, int parameter)
{
	char msg[BUFSIZ];

	memset(msg, 0, sizeof(msg));
//	sprintf(msg, "echo %s %d >> /tmp/error.txt", message, parameter);
//	system(msg);

}

int query_type(const char *buf)
{
	int i;
	for(i = 0; i < 4; i++)
		if(!strcasecmp(buf, req_type[i]))
			return i;

	return -1;
}

int user_provides_a_question(struct sock_s *s, struct sock_in *p)
{
	char buf[BUFSIZ];
	char filename[BUFSIZ];

	int addrlen;

	memset(buf, 0, sizeof(buf));
	if( recvfrom(s->sd , buf, sizeof(buf), 0, (struct sockaddr *)&p->pin, &addrlen) == -1 ) {
		fprintf(stderr,"recvfrom error");
	}
	ap_debug("%s\n",buf);

	if(strlen(buf))
		log_stuff(buf, __LINE__);

	obtain_next_qfile(filename);
	dump_buffer_to_file(buf, filename);

	return 0;

}


int user_provides_an_answer(struct sock_s *s, struct sock_in *p)
{
	char buf[BUFSIZ];
	char filename[BUFSIZ];
	int addrlen = sizeof(p->pin);

	ap_debug("the client provides an answer %d \n",__LINE__);
	memset(buf, 0, sizeof(buf));
	if( recvfrom(s->sd , buf, sizeof(buf), 0, (struct sockaddr *)&p->pin, &addrlen) == -1 ) {
		fprintf(stderr,"recvfrom error");
	}
	strcpy(p->input, buf);
	ap_debug(" possibly the answer from client : %s\n",buf);
	obtain_next_afile(filename);
	dump_buffer_to_file(buf, filename);
	return 0;
}

int user_wants_an_answer(struct sock_s *s, struct sock_in *p)
{
	char buf[BUFSIZ];
	int addrlen = sizeof(p->pin);
	struct question *q;
	q = (struct question *)malloc(sizeof(struct question)); 
	get_question_to_answer(q); /* TODO this is not the correct way, there should be a reliable way to pass the question around */

	while(1) {
		if(check_for_answer(q) == 0) {
			create_abuf(q,buf);
			if ( sendto(s->sd, buf , strlen(buf) + 1, 0, (struct sockaddr *)&p->pin, addrlen) == -1) {
				fprintf(stderr,"%s %d",strerror(errno), __LINE__);
				exit(EXIT_FAILURE);
			}
			break;
		}
		fprintf(stderr,"Waiting for obtaining answer\n");
		sleep(1);
	}
	free_question(q);
	return 0;
}

int user_needs_a_question(struct sock_s *s, struct sock_in *p)
{
	char buf[BUFSIZ];
	int addrlen = sizeof(p->pin);

	struct question *q;
	q = (struct question *)malloc(sizeof(struct question)); 
	get_question_to_answer(q);
	create_qbuf(q, buf);
	if(strlen(buf))
		log_stuff(buf, __LINE__);
	if ( sendto(s->sd, buf , strlen(buf) + 1, 0, (struct sockaddr *)&p->pin, addrlen) == -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}

	free_question(q);
	return 0;

}

/* send a message to the server PORT on machine HOST */
/* obtain question to be asked */
/* req server for a question */		
int client_req_ques(struct sock_cl *s)
{
	int status;
	if ((status  = sendto(s->sd, s->buffer, strlen(s->buffer) + 1, 0, (struct sockaddr *)&s->sin, s->addrlen))== -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	ap_debug("status=%d\n",status);

	/* spew-out the results and bail out of here! */
	memset(s->buffer, 0, sizeof(s->buffer));
	/* wait for a message to come back from the server */
	if (recvfrom(s->sd, s->buffer, BUFSIZ, 0, (struct sockaddr *)&(s->pin), &(s->addrlen)) == -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	dump_buffer_to_file(s->buffer, question_file);
	printf("message from server = %s\n", s->buffer);
	/* question has been created to be displayed to the user */
	/* now change the buffer to send answer with save answer*/
	memset(s->buffer, 0, sizeof(s->buffer));
	strcpy(s->buffer, req_type[SAVE_ANSWER]); /* switch to answering mode */

	return 0;
}

/* display answer to the the user after it is obtained */
/* server would send the answer back correspondin to the question, 
 * the client must create an answer file corresponding to the answer */
int client_req_ans(struct sock_cl *s)
{
	int status;
	if ((status  = sendto(s->sd, s->buffer, strlen(s->buffer), 0, (struct sockaddr *)&s->sin, s->addrlen))== -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	memset(s->buffer, 0, sizeof(s->buffer));

	if (recvfrom(s->sd, s->buffer, BUFSIZ, 0, (struct sockaddr *)&s->pin, &s->addrlen) == -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	dump_buffer_to_file(s->buffer, answer_file);
	printf("message from server = %s\n", s->buffer);
	return 0;
}

int client_save_ans(struct sock_cl *s, char *dir)
{
	int status;
	if ((status  = sendto(s->sd, dir, strlen(dir) + 1, 0, (struct sockaddr *)&s->sin, s->addrlen))== -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(1);
	}

	/* the user created the answer now transport it to server */
	if ((status  = sendto(s->sd, s->buffer, strlen(s->buffer)+1, 0, (struct sockaddr *)&s->sin, s->addrlen))== -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	return 0;
}

int client_save_ques(struct sock_cl *s, char *dir)
{
	int status;
	s->addrlen = sizeof(s->sin);
	/* the user created the question now transport it to server */
	if ((status  = sendto(s->sd, dir, strlen(dir) + 1, 0, (struct sockaddr *)&s->sin, s->addrlen))== -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(1);
	}

	/* the user created the question now transport it to server */
	if ((status  = sendto(s->sd, s->buffer, strlen(s->buffer) + 1, 0, (struct sockaddr *)&s->sin, s->addrlen))== -1) {
		fprintf(stderr,"%s %d",strerror(errno), __LINE__);
		exit(1);
	}
	memset(dir, 0, sizeof(dir));
	strcpy(dir, req_type[REQ_ANSWER]);
	return 0;
}
