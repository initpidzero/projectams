/* Author - anuz
* This file is distibuted under GPLv2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include "header.h"

extern int errno;

/* be careful between input sock and output sock */
void get_the_buffer(struct sock_s *s, struct sock_in *p)
{
	char buf[BUFSIZ];
	int addrlen = sizeof(p->pin);
	pid_t pid;

	memset(buf, 0, sizeof(buf));
	if( recvfrom(s->sd , buf, sizeof(buf), 0, (struct sockaddr *)&p->pin, &addrlen) == -1 ) {
		printf("recvfrom error");
	}
	strcpy(p->input, buf);
	ap_debug("%s\n",buf);
	if(strlen(buf))
		log_stuff(buf, __LINE__);

	switch(query_type(buf)) {
		case REQ_QUESTION:
			user_needs_a_question(s,p);
			break;
		case REQ_ANSWER:
			if ((pid = fork()) == 0) {
				ap_debug("child process = %d  \n ",getpid());
				user_wants_an_answer(s,p);
				exit(EXIT_SUCCESS);
			} else if (pid < 0) {
				fprintf(stderr,"fork error %s\n", strerror(errno));
			} else {
				ap_debug("parent process = %d\n ",getpid());
			}
			break;
		case SAVE_QUESTION:
			user_provides_a_question(s,p);
			break;
		case SAVE_ANSWER:
			user_provides_an_answer(s,p);
			break;
		default:
		case -1:
			fprintf(stderr, "What the fuck bro?\n");
			exit(EXIT_FAILURE);

	}

}

static void populate_sock(struct sock_s *s)
{
	if ((s->sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	s->port = port;

	memset(&s->sin, 0, sizeof(s->sin));
	s->sin.sin_family = AF_INET;
	s->sin.sin_addr.s_addr = INADDR_ANY;
	s->sin.sin_port = htons(s->port);

	if (bind(s->sd, (struct sockaddr *)&s->sin, sizeof(s->sin)) == -1) {
		perror("bind");
		close(s->sd);
		exit(EXIT_FAILURE);
	}
}

int create_server()
{
	struct sock_s s;
	struct sock_in p;
	int status;
	populate_sock(&s);
	while(1) {
		get_the_buffer(&s, &p);
		sleep(2);
		waitpid(-1, &status, WNOHANG);
		if(WIFEXITED(status))
			printf("child exited normally\n");
	}
}

void deamonify(void)
{
	pid_t sid;
	int fd;

	sid = setsid(); /* get a new process group. */
	chdir("/");
	umask(0);
	/*close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	fd = open("/dev/null", O_RDWR);*//* stdin */
	//(void) dup(fd); /* stdout */
	//(void) dup(fd); /* stderr */
	ap_debug ("again process = %d\n ",getpid());
	create_server();
}

int create_child()
{

	pid_t pid;
	if ((pid = fork()) == 0) {
		ap_debug("child process = %d  \n ",getpid());
		deamonify();
	} else if (pid < 0) {
		fprintf(stderr,"fork error %s\n", strerror(errno));
	} else {
		ap_debug("parent process = %d\n ",getpid());
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int main (int argc, char*argv[])
{
	if(argv[1]) 
		port = atoi(argv[1]);
	create_child();
	return 0;
}
