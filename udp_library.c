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

	struct question *q;
	q = (struct question *)malloc(sizeof(struct question)); 
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

	return 0;

}
