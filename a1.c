#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


struct job
{
    pid_t pid;
    char* name;
};

struct job jobs[10];


int getcmd(char *prompt, char *args[], int *background)
{
    int length, i = 0;
    char *token, *loc;
    char *line = NULL;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);
    if (length <= 0)
    {
        exit(-1);
    }

    // Check if background is specified..
    if ((loc = index(line, '&')) != NULL)
    {
        *background = 1;
        *loc = ' ';
    }
    else
    {
        *background = 0;
    }

    while ((token = strsep(&line, " \t\n")) != NULL)
    {
        for (int j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }
    free(line);
    return i;
}


int main(void)
{
    char *buffer;
    size_t size = 1024;
    buffer = (char *)malloc((size_t)size);

    int bg;
    while(1)
    {
        char *args[20] = { NULL };
        bg = 0;
        int cnt = getcmd("\n>> ", args, &bg);
        /* the steps can be..:
        (1) fork a child process using fork()
        (2) the child process will invoke execvp()
        (3) if background is not specified, the parent will wait,
        otherwise parent starts the next command... */

        /*
        for (int i = 0; args[i] != NULL; i++) {
            printf("%d", i);
            printf("Argument %d: %s\n", i, args[i]);
        }        
        */

        pid_t child_pid;
        child_pid = fork();

        if (child_pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (child_pid == 0)
        {
            // This is the child process
        }
        else
        {
            // This is the parent process
            if (strcmp(args[0], "echo") == 0)
            {
                for (int i = 1; args[i] != NULL; i++)
                    printf("%s ", args[i]);
                printf("\n");
                fflush(stdout);
            }
            else if (strcmp(args[0], "cd") == 0)
            {
                chdir(args[1]);
            }
            else if (strcmp(args[0], "pwd") == 0)
            {
                getcwd(buffer, size);
                printf("%s", buffer);
            }
            else if (strcmp(args[0], "exit") == 0)
            {
                exit(0);
            }
            else if (strcmp(args[0], "jobs") == 0)
            {                
                for (int i = 1; i<10; i++)
                {
                    printf("job %d is %s\n", i, jobs[i].name);
                }
            }
            wait(NULL);
        }
    }
}