/* Author - anuz
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "header.h"

int main (int argc, char *argv[])
{
	char filename[BUFSIZ];
	char buffer[BUFSIZ];
	struct question *q;
	if(argc > 1)
		sprintf(filename,"%s",argv[1]);
	else
		sprintf(filename, question_file);

	/*TODO  We need a more elaborate way of dealing with this stuff */
	unlink("question.txt");
	unlink("answer.txt");

	create_req_file(REQ_QUESTION);
	while(1) {
		printf("In order to answer a question press \"Y or y\" and enter\n"); 
		fgets(buffer, BUFSIZ,stdin);
		if(buffer == NULL)
			continue;
		if(!strncasecmp(buffer,"Y",1)) {
			if( access( filename, F_OK ) == -1 ) {
				fprintf(stderr, "No question available at this time, Waiting for question\n");
				continue;
			}
			q = (struct question *)malloc(sizeof(struct question)); 
			//if(get_question_to_answer(q) != 0) { /* we wan question to be answered by local file at this moment */
			if(populate_ques_ds(filename,q) != 0){
				fprintf(stderr, "throw all kinds of bitch fit\n");
				sleep(1);
				free_question(q);
				continue;
			}
			printf("Please answer the following question and press enter to submit\n");	
			printf("%s \n", q->question);	
			fgets(buffer, BUFSIZ, stdin);
			store_answer(q, buffer);
			free_question(q);
			break;
		}
		sleep(1);
	}

	unlink(REQFILE);
	ap_debug("%s\n",__FILE__);
	return 0;
}
