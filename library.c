/* Author - anuz
* This file is distibuted under GPLv2
* contains all the routines needed to create files 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>
#include "header.h"
/* 0 will always represent sucess 
 * and 1 and other number will represent failure 
 * negative number should only be used to represent system failure 
 */

char *question_file = "question.txt";
char *answer_file = "answer.txt";
char qtags[][BUFSIZ] = {"q","id", "p1","p2", "f", "did"};
char atags[][BUFSIZ] = {"q","id", "a", "f", "did"};
char req_type[][15] = {"req_question","req_answer", "save_question", "save_answer"}; 

/* if we even need this stuff */
void error_exit(char *str)
{

	fprintf(stderr,"%s", str);
	exit(EXIT_FAILURE);
}
/* somehow this is going to get all very magical */

int obtain_next_afile(char *filename)
{
	if(1) /* this need some kind of check for existence of file name somewhere */
		strcpy(filename, "/media/disk-1/anuz/projects/project1/answer.txt");
	else
		return 1;
	return 0;
}

/* somehow this should know if there is a file or not */
int obtain_next_qfile(char *filename)
{
	if(1) /* unix file check */
		strcpy(filename, "/media/disk-1/anuz/projects/project1/question.txt");
	else	
		return 1;

	return 0;
}

/* remove new line character from the end of the src string  and copy to dest */
/* TODO add support for removal of all kinds of white spaces before and after */
int strip_n_copy(char *dest, const char *src)
{
	int len = strlen(src);
	char *tmp; 
	tmp = src + len;
	while(src[len] != '\n') {
		len--;
	}
	
	ap_debug("what is at len %c", src[len]);
	strncpy(dest, src, len);
	dest[len] = '\0';
	return 0;
}

/* TODO at this point only one question and one answer is being used 
 * This procedure expect the question structure be allocated 
 * */
int get_question_to_answer(struct question *q)
{
	char filename[BUFSIZ];
	if (obtain_next_qfile(filename) == 0) {
		return populate_ques_ds(filename, q);
	}	
	return 1;
}

/* TODO write this routine completly */
int free_question(struct question *q)
{
	if(q == NULL)
		return -1;
	if(q->a) 
		free_answer(q->a);

	if(q->question)
		free(q->question);
	q->question = NULL;

	if(q->pref1)
		free(q->pref1);
	q->pref1 = NULL;

	if(q->pref2)
		free(q->pref2);
	q->pref2 = NULL;

	if(q->id)
		free(q->id);
	q->id = NULL;

	if(q->did)
		free(q->did);
	q->did = NULL;

	q->flag = 0;
	q->fp = NULL;

	free(q);
	q = NULL;

	return 0;
}

int obtain_req_type(char *buffer)
{
	FILE *fp = fopen("request.txt","r");
	char temp[15];
	if(fp == NULL) {
		fprintf(stderr, "No request available to send\n");
		return -1;
	}

	fgets(temp,sizeof(temp),fp); 
	strcpy(buffer,temp);
	fclose(fp);

	return 0;
}

int create_req_file(int req)
{

	FILE *fp = fopen("request.txt","w");
	if(fp == NULL) {
		fprintf(stderr, "Oddly enough, the file cannot be created\n");
		return -1;
	};
	fprintf(fp,"%s",req_type[req]);
	fclose(fp);

	return 0;
}

int check_for_answer(struct question *q)
{
	char filename[BUFSIZ];	
	int status;
	status = obtain_next_afile(filename);
	if(status == 1)
		return 1;
	return populate_if_match(q, filename); /* obtain data structure for question */

}

/* answers will always be linked to the question data structure */
int free_answer(struct answer *a)
{
	if(a->q) 
		a->q = NULL; /* freeing question is not the responsibility of this method */
	free(a->answer);
	a->answer = NULL;
	free(a);
	a = NULL;

}

/* free the memory after usage */
int get_device_id(char *did)
{
	strcpy(did, "Android1"); /* TODO this is to be obtained from device */
	return 0;
}

/* this assuming the name of the file remains fixed */
int create_answer_file(struct question *q)
{

	FILE *fp = fopen(answer_file, "w"); /* to be removed with get_answer_filename() */
	char flag[2];
	sprintf(flag,"%d",q->flag);
	if(fp == NULL) {
		fprintf(stderr, "Cannot create question file");
		return -1;
	}
	add_tag(fp,  atags[0], q->question); 
	add_tag(fp,  atags[1], q->id); 
	add_tag(fp,  atags[2], q->a->answer); 
	add_tag(fp,  atags[3], flag); 
	add_tag(fp,  atags[4], q->did); 

	fclose(fp);
	return 0;
}

/* add tags to the file */
int add_tag(FILE *fp, char *tag, char *string)
{
	char * buffer = (char *)malloc(strlen(string) + 10);

	sprintf(buffer,"<%s>%s</%s>\n",tag,string,tag);
	fprintf(fp,buffer);
	free(buffer);
	buffer = NULL;
	return 0;
}

/* free the memory after usage */
int get_sha1(char *hash, const char *message)
{

	unsigned char md[21];
	int i;
	unsigned char *sha1 = SHA1(message, strlen(message), md);
	ap_debug("md = %x ", md[0]);
	ap_debug("sha1 = %x \n",sha1[0]);
	for(i = 0; i < strlen(md); i++) {
		char temp[2];
		sprintf(temp,"%x",md[i]);
		strcat(hash,temp);
	}

	return 0;
}

/* question data structure is also populated for rest of the values 
 * question and preferences are already supplied, id and did are populated here 
 * also since this is the first time question file is created answer ds is incomplete
 */
int create_question_file(struct question *q)
{
	unsigned char id[41] = "";
	char did[10];
	FILE *fp = fopen(question_file, "w");
	char flag[2];
	sprintf(flag,"%d",q->flag);

	if(fp == NULL) {
		fprintf(stderr, "Cannot create question file");
		return 1;
	}

	if(q->did == NULL){	
		get_device_id(did);
		q->did = (char *)malloc(strlen(did)+1);
		strcpy(q->did, did);
	}

	if(q->id == NULL){	
		get_sha1(id, q->question);
		q->id = (unsigned char *)malloc(strlen(id)+1);
		strcpy(q->id, id);
	}

	add_tag(fp,  qtags[0], q->question); 
	add_tag(fp,  qtags[1], q->id); 
	add_tag(fp,  qtags[2], q->pref1); 
	add_tag(fp,  qtags[3], q->pref2); 
	add_tag(fp,  qtags[4], flag); 
	add_tag(fp,  qtags[5], q->did); 

	fclose(fp);	
	//free_question(q);
	return 0;
}

int record_yes(struct question *q)
{
	if(q != NULL) {
		q->flag = 1;
		create_question_file(q);
	}
	/* we also need to make sure that question file is also update */
	else 
		return -1;

	return 0;
}

int store_answer(struct question *q, const char *answer)
{
	ap_debug("your answer\n%s\n",answer);
	q->a = (struct answer *)malloc(sizeof(struct answer));
	q->a->answer = (char *)malloc(sizeof(strlen(answer) +1));
	strip_n_copy(q->a->answer, answer);
	return create_answer_file(q);

}

int ask_another(char *buffer)
{
	struct question *q;
	printf("While we find answer to your question, would you like to answer a question\n");
	printf("Press \"Y or y\" or \"N on n\" to continue waiting \n");
	fgets(buffer, BUFSIZ,stdin);
	if(buffer == NULL)
		return -1;
	if(!strncasecmp(buffer,"Y",1)) {
		q = (struct question *)malloc(sizeof(struct question)); /* the next routine requires the question to be allocated */
		if(get_question_to_answer(q) == 0){
			printf("Please answer the following question and press enter to submit\n");	
			printf("%s \n", q->question);	
			fgets(buffer, BUFSIZ, stdin);
			store_answer(q, buffer);
			return 0;
		} else {
			return -1;
		}
	}
	if(!strncasecmp(buffer,"N",1))  
		return 1;
}

int dump_buffer_to_file(const char *buf, const char *filename)
{
	FILE *fp = fopen(filename,"w");
	if(fp == NULL) {
		fprintf(stderr, "The file cannot be created\n");
		return -1;
	}
	fprintf(fp,"%s",buf);
	fclose(fp);
	return 0;
}

int create_qbuf(struct question *q, char *buf)
{

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "<q>%s</q>\n<id>%s</id>\n<p1>%s</p1>\n<p2>%s</p2>\n<f>%d</f>\n<did>%s</did>\n",
			q->question,q->id,q->pref1,q->pref2,q->flag,q->did);
	return 0;
}

int create_abuf(struct question *q, char *buf)
{
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "<q>%s</q>\n<id>%s</id>\n<a>%s</a>\n<did>%s</did>\n<f>%d</f>\n",
			q->question,q->id,q->a->answer,q->did,q->flag);
	return 0;
}

/* extract relevant string from the tag */
int tagify(const char *tag, const char *input, char *output)
{
	char t_b[BUFSIZ];
	char t_e[BUFSIZ];
	unsigned int len = strlen(input);
	char *begin = NULL;
	char *end = NULL;
	unsigned int tag_size;

	sprintf(t_b,"<%s>",tag);	
	tag_size = strlen(t_b);
	sprintf(t_e,"</%s>",tag);	
	begin = strstr(input,t_b);

	if(begin != NULL)
		ap_debug("first = %s\n", begin+tag_size);
	else
		return 1;

	end = strstr(input,t_e);
	if(end != NULL)
		ap_debug("last = %s\n", end);

	len = (char *)end - (char *)(begin + tag_size);
	strncpy(output, begin+tag_size, len);
	*(output + len) = '\0';
	ap_debug("string len = %d\n",len);
	ap_debug("output = %s\n",output);
	return 0;
}

/* check if question and answer are matching 
 * First even if the file name is matching
 * and if sha1 ids are matching or not
 * populate answer structure
 * */
int populate_if_match(struct question *q, const char *filename)
{
	char buffer[BUFSIZ];
	char obuffer[BUFSIZ];
	FILE *fp;
	char *answer;
	if(q == NULL)
		return 1;
	fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "there is no fucking way\n");
		return 1;
	}
	while(fgets(buffer, BUFSIZ, fp)) {
		if(tagify(atags[1],buffer, obuffer) == 0) 
			if(strcmp(obuffer, q->id) != 0) {
				return 1;
			} else {
				ap_debug("buffer = %s",obuffer);
				ap_debug("id = %s",q->id);
			}

		if(tagify(atags[2],buffer, obuffer) == 0)
			answer = obuffer;
	}

	if(q->a == NULL)
		q->a = (struct answer *)malloc(sizeof(struct answer));	

	ap_debug("do we have an answer here %s\n", answer);
	q->a->answer = (char *)malloc(strlen(answer) +1);
	strcpy(q->a->answer, answer);
	q->a->q = q; /* redundant probably */
	return 0;
}

/* this will open the file and obtain the relevant data structure */
int populate_ques_ds(char *filename, struct question *q)
{
	char buffer[BUFSIZ];
	char obuffer[BUFSIZ];
	FILE *fp;
	int total_tags = sizeof(qtags)/BUFSIZ;

	fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "there is no fucking way\n");
		return -1;
	}

	while(fgets(buffer, BUFSIZ, fp)){
		if(feof(fp) != 0) 
			break;
		int i;
		ap_debug("%s\n", buffer);
		for(i = 0; i < total_tags; i++) {
			if(tagify(qtags[i],buffer, obuffer) == 0)
				switch(i) {
					case 0:
						q->question = (char *)malloc(strlen(obuffer)+1);
						strcpy(q->question, obuffer);
						ap_debug("question = %s\n",q->question);
						break;
					case 1:
						q->id = (unsigned char *)malloc(strlen(obuffer)+1);
						strcpy(q->id, obuffer);
						ap_debug("question = %s\n",q->id);
						break;
					case 2:
						q->pref1 = (char *)malloc(strlen(obuffer)+1);
						strcpy(q->pref1, obuffer);
						ap_debug("question = %s\n",q->pref1);
						break;
					case 3:
						q->pref2 = (char *)malloc(strlen(obuffer)+1);
						strcpy(q->pref2, obuffer);
						break;
					case 4:
						sscanf(obuffer, "%d",&(q->flag));
						/* flag */
						break;
					case 5:
						q->did = (char *)malloc(strlen(obuffer)+1);
						strcpy(q->did, obuffer);
						/* device id */
						break;
				}
		}		
	}
	fclose(fp);
	return 0;
}
