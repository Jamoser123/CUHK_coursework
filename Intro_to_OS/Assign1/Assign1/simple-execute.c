#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
/*
Frederico Aberle 1155210337
Janic Moser	1155210428
*/



char *arg1[30]; //arguments for first pipe
char *arg2[30]; //arguments for second pipe
char *arg3[30]; //arguments for third pipe

int shell_execute(char ** args, int argc)
{
	int wait_return, status;

	int p1[2], p2[2], p3[2]; //pipes
	int child0, child1, child2, child3;
	int pipes = 0; //#pipes
	int argCount = 0;

	if (strcmp(args[0], "EXIT") == 0)
		return -1; 

	for (int i = 0; i < argc; i++){
		if (args[i] == NULL){
			if (pipes == 1){
				arg1[argCount + 1] = NULL;
			}
			if (pipes == 2){
				arg2[argCount + 1] = NULL;
			}
			if (pipes == 3){
				arg3[argCount + 1] = NULL;
			}
		}
		else if (strcmp(args[i], "|") == 0){
			if (pipes == 0){
				args[i] = NULL;
			}
			if (pipes == 1){
				arg1[argCount + 1] = NULL;
			}
			if (pipes == 2){
				arg2[argCount + 1] = NULL;
			}
			pipes++;
			argCount = 0;
		}
		else{
			if (pipes == 1){
				arg1[argCount] = args[i];
			}
			if (pipes == 2){
				arg2[argCount] = args[i];
			}
			if (pipes == 3){
				arg3[argCount] = args[i];
			}
			argCount++;
		}
	}

	if((child0 = fork()) < 0)
	{
		printf("fork() Child 0error \n");
	}
	else if (child0 == 0)
	{
		if (pipes == 0){
			if (execvp(args[0], args) < 0)
			{	 
				printf("execvp() Child 0 error \n");
				exit(-1);
			}
		}
		if (pipes == 1)
		{ 
			pipe(p1);
			if ((child1 = fork()) < 0)
			{
				printf("fork() Child 1 error \n");
			}
			else if (child1 == 0)
			{
				close(1);
				dup(p1[1]); 

				close(p1[0]); close(p1[1]);
				if (execvp(args[0], args) < 0)
				{
					printf("execvp() Child 1 error\n");
					exit(-1);
				}
			}
			else
			{	close(0);
				dup(p1[0]);

				close(p1[0]); close(p1[1]);
				if (execvp(arg1[0], arg1) < 0)
				{
					printf("execvp() Child 0 error\n");
					exit(-1);
				}
			}

		}
		else if (pipes == 2)
		{ 
			pipe(p1);
			pipe(p2);

			if ((child2 = fork()) < 0)
			{
				printf("fork() Child 2 error\n");
			}
			else if (child2 == 0)
			{	
				close(1);
				dup(p1[1]);

				close(p1[0]); close(p1[1]);
				close(p2[0]);close(p2[1]);
				if (execvp(args[0], args) < 0)
				{
					printf("execvp() Child 2 error\n");
				}
			}
			else if ((child1 = fork()) < 0)
			{
				printf("fork() Child 1 error\n");
			}
			else if (child1 == 0)
			{ 	
				close(0);
				dup(p1[0]);
				close(1);
				dup(p2[1]);

				close(p1[0]); close(p1[1]);
				close(p2[0]); close(p2[1]);
				if (execvp(arg1[0], arg1) < 0)
				{
					printf("execvp() Child 1 error\n");
				}
			}
			else
			{   
				close(0);
				dup(p2[0]); 

				close(p1[0]);close(p1[1]);
				close(p2[0]); close(p2[1]);
				if (execvp(arg2[0], arg2) < 0)
				{
					printf("execvp() Child 0 error\n");
				}
			}
		}
		else if (pipes == 3)
		{ 
			pipe(p1);
			pipe(p2);
			pipe(p3);

			if ((child3 = fork()) < 0)
			{ 
				printf("fork() Child 3 error\n");
			}
			else if (child3 == 0)
			{	
				close(1);
				dup(p1[1]); 

				close(p1[0]); close(p1[1]);
				close(p2[0]); close(p2[1]);
				close(p3[0]); close(p3[1]);
				if (execvp(args[0], args) < 0)
				{
					printf("execvp() Child 3 error\n");
				}
			}
			else if ((child2 = fork()) < 0)
			{
				printf("fork() Child 2 error\n");
			}
			else if (child2 == 0)
			{	
				close(0);
				dup(p1[0]);
				close(1);
				dup(p2[1]); 

				close(p1[0]); close(p1[1]);
				close(p2[0]); close(p2[1]);
				close(p3[0]); close(p3[1]);

				if (execvp(arg1[0], arg1) < 0)
				{
					printf("execvp() Child 2 error\n");
				}
			}
			else if ((child1 = fork()) < 0)
			{
				printf("fork() Child 1 error\n");
			}
			else if (child1 == 0)
			{   
				close(0);
				dup(p2[0]);
				close(1);
				dup(p3[1]); 

				close(p1[0]); close(p1[1]);
				close(p2[0]); close(p2[1]);
				close(p3[0]); close(p3[1]);
				if (execvp(arg2[0], arg2) < 0)
				{
					printf("execvp() Child 1 error\n");
				}
			}
			else
			{	
				close(0);
				dup(p3[0]);

				close(p1[0]); close(p1[1]);
				close(p2[0]); close(p2[1]);
				close(p3[0]); close(p3[1]);
				if (execvp(arg3[0], arg3) < 0)
				{
					printf("execvp() Child 0 error\n");
				}
			}
		}
		else if (pipes > 3)
		{
			printf("Too many pipes\n");
			exit(-1);
		}
	}
	else
	{
		if ((wait_return = wait(&status)) < 0)
			printf("wait() error \n"); 
	}
			
	return 0;
}
