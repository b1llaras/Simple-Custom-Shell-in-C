#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define BUFFER_SIZE 100
#define MAX_ARGS 100

//---------FUNCS---------//
void input_commands(char buffer[]);
int count_pipes(char buffer[]);
void execute_cmd(char buffer[], char const *argv[]);
void run_piped_commands(char buffer[]);

//---------VARS---------//
char buffer[BUFFER_SIZE];
char* args[MAX_ARGS];
char* token; char* token1; char* token2;
pid_t pid;

int main(int argc, char const *argv[])
{
    while (1)
    {
        printf("my_shell$ ");
        input_commands(buffer);

        int count = count_pipes(buffer);

        if (count == 0)
        {
            execute_cmd(buffer,argv);
        }

        else
        {
            run_piped_commands(buffer);
        }

    }

}

void input_commands(char buffer[]){
    fgets(buffer, BUFFER_SIZE, stdin);
    int length = strlen(buffer);


    buffer[length-1] = '\0';

    if (strcmp(buffer, "exit") == 0)
    {
        printf("exited\n");
        exit(0);
    }
}


void execute_cmd(char buffer[], char const *argv[]){
    pid = fork();

    if (pid == -1)//fork error
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }

    else if(pid == 0){//child process

        token = strtok(buffer, " \n");

        int i;
        while (token != NULL)
        {
            args[i] = token;
            i++;
            token = strtok(NULL, " \n");
        }args[i] = '\0';

        execvp(args[0], args);
        perror("child error");
        exit(EXIT_FAILURE);
    }

    else if(pid > 0){//parent process
        int status;

        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

}

int count_pipes(char buffer[]){
    int pipes_found = 0;
    int length = strlen(buffer);
    int i;

    for (i = 0; i < length; i++) //check for '|'
    {
        if(buffer[i] == '|'){
            pipes_found++;
        }
    }
    return pipes_found;
}


void run_piped_commands(char buffer[]){
    int i = 0, j = 0;

    token1 = strtok(buffer, "|");
    token2 = strtok(NULL, "|");

    char* args1[64];
    char* args2[64];

    char* token1_1; char* token2_1;
    token1_1 = strtok(token1, " \n");

//token1 to arguments
    while (token1_1 != NULL)
    {
        args1[i++] = token1_1;
        token1_1 = strtok(NULL, " \n");

    }args1[i] = '\0';

//token2 to arguments
    token2_1 = strtok(token2, " \n");
    while (token2_1 != NULL)
    {
        args2[j++] = token2_1;
        token2_1 = strtok(NULL, " \n");
    }args2[j] = '\0';

    pid_t pid;
    int pipefds[2];
    int pid1, pid2;

    if (pipe(pipefds) == -1)
    {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    pid1 = fork();

    if (pid1 == -1)
    {
        perror("fork 1 error");
        exit(EXIT_FAILURE);
    }

    else if (pid1 == 0) //child1
    {
        printf("in child 1: %s, %ld\n", args1[0], strlen(args1[0]));
        close(pipefds[1]);
        dup2(pipefds[1], 1);
        close(pipefds[0]);

        execvp(args1[0], args1);
        perror("child 1 error");
        exit(EXIT_FAILURE);
    }

    else if(pid1 > 0){
        int status;

        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    pid2 = fork();

    if (pid2 == -1)
    {
        perror("fork 2 error");
        exit(EXIT_FAILURE);
    }

    else if (pid2 == 0) //child2
    {
        printf("in child 2: %s, %ld\n", args2[0], strlen(args2[0]));
        close(pipefds[0]);
        dup2(pipefds[0], 0);
        close(pipefds[1]);
        
        execvp(args2[0], args2);
        perror("child 2 error");
        exit(EXIT_FAILURE);
    }

    else if(pid2>0){
        int status;

        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    close(pipefds[0]);
    close(pipefds[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


