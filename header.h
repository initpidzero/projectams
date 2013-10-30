#ifndef __HEADER_H__
#define __HEADER_H__

#ifdef DEBUG
#define ap_debug(a,b)  printf(a,b)
#else
#define ap_debug(a,b)  
#endif

#include <sys/socket.h>
#include <netinet/in.h>

extern char *question_file;
extern char *answer_file;
extern char qtags[][BUFSIZ];
extern char atags[][BUFSIZ];
extern char req_type[][15]; 
extern unsigned short int port;

struct uni_buf {
	char buffer[BUFSIZ];
	int flag;
	struct uni_buf *next; /* possibility of a link list */
	struct uni_buf *previous; /* possibility of a double link list */
};

struct file_list {
	char *qfile;
	char *afile;
	int latest;
	struct file_list *next;
	/* we may need a lock as well */
	};

enum request_type {
	REQ_QUESTION,
	REQ_ANSWER,
	SAVE_QUESTION,
	SAVE_ANSWER
};

struct file_list *list;

#define REQFILE "request.txt"
struct question {
	char *question;
	char *pref1;
	char *pref2;
	unsigned int flag; /* if question has been answered or not */
	unsigned char *id;
	char *did;
	FILE *fp;
	struct answer *a;
};


struct answer {
	char *answer;
	FILE *fp;
	struct question *q; /* this data structure should hold the question */
};

struct sock_s {
	unsigned short int port;
	int sd;
	struct sockaddr_in sin;
}; /* server struct */

struct sock_in {
	int sd;
	char input[BUFSIZ];
	struct sockaddr_in pin;
}; /* client struct */

struct sock_cl {
	int sd;
	struct sockaddr_in sin; /* outgoing socket */
	struct sockaddr_in pin; /* incoming socket */
	int addrlen;
	unsigned short int port;
	char buffer[BUFSIZ];
};
/* PROTOTYPE BITCHES */
int create_question_file(struct question *q);
int record_yes(struct question *q);
int populate_ques_ds(char *filename, struct question *q);
#endif /* this is the end of __HEADER_H__ */
