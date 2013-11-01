/* Author - anuz
 * This file is distibuted under GPLv2
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>
#include "header.h"

int main (int argc, char*argv[])
{
	struct question *q = NULL;
	char buffer[BUFSIZ];
	while(1) {
		printf("Hi this program provides the answers to your questions\n");
		printf("Enter \"Q or q\"for question\n");
		fgets(buffer, BUFSIZ,stdin);
		ap_debug("%s",buffer);
		if(!strncasecmp(buffer,"Q",1)) {
			/*TODO  We need a more elaborate way of dealing with this stuff */
			unlink("question.txt");
			unlink("answer.txt");
			/* before allocating this structure it might be a good idea to check and relieve this data structure */
			if(q != NULL)
				free_question(q);
			q = (struct question *)malloc(sizeof(struct question));
			printf("Please type your question and press enter\n");
			fgets(buffer, BUFSIZ, stdin);
			q->question = (char *)malloc(strlen(buffer) + 1);
			strip_n_copy(q->question, buffer);
			ap_debug("your question was: \n%s\n",buffer);
			printf("Please type your first preference\n");
			fgets(buffer, BUFSIZ, stdin);
			q->pref1 = (char *)malloc(strlen(buffer) + 1);
			strip_n_copy(q->pref1, buffer);
			ap_debug("your preference was: \n%s\n",buffer);
			printf("Please type your second preference\n");
			fgets(buffer, BUFSIZ, stdin);
			q->pref2 = (char *)malloc(strlen(buffer) + 1);
			strip_n_copy(q->pref2, buffer);
			ap_debug("your second preference was: \n%s\n",buffer);
			/* storing the question and the data structure */
			q->flag = 0;
			q->id = NULL;
			q->did = NULL;
			q->a = NULL;
			create_question_file(q);
			create_req_file(SAVE_QUESTION);
			/* do we want to consistently keep nagging the user for answering */	
			while(1) {
				sleep(1);
				if (check_for_answer(q) == 0) {
					/* display answer and destroy question */ 
					if(q->a == NULL) 
						break;
					printf("This is the answer to your question: \n%s\n",q->a->answer);
					printf("Are you satisfied with the answer \n");
					printf("Press Y/y or N/n and press enter \n");
					fgets(buffer, BUFSIZ,stdin);
					if(!strncasecmp(buffer,"Y",1)) {
						/* at this point we can record that question was satisfactorily answered */
						record_yes(q);
						break;
					} else if(!strncasecmp(buffer,"N",1)) {
						/* Nopes, he wants another answer */
						continue;
					} else {
						printf("What the fuck is your problem dude? Just say yes or no\n");
					}

				}
				printf("No answer yet continuing waiting....\n");
			}
			unlink(REQFILE);
			printf("Do you want to exit? \n");
			printf("Press Y/y and press enter to exit\n");
			fgets(buffer, BUFSIZ,stdin);
			if(!strncasecmp(buffer,"Y",1)) {
				break;
			} 
		} else {
			continue;
		}
	} 
	return 0;
}
