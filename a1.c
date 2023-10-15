#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>


typedef struct Job
{
    char *name;         // Process name
    pid_t pid;          // Process ID
};


struct Job jobs[20];


void addjob(pid_t pid, char *name) {
    struct Job *job = malloc(sizeof(struct Job));
    job->pid = pid;
    job->name = strdup(name);

    int i = 0;
    while (i < 20) {
        if (jobs[i].name == NULL) {
            jobs[i] = *job;
            break;
        }
        i++;
    }
    free(job);
}


void removeJob(pid_t pid)
{
    for (int i = 0; i < 20; i++)
    {
        if (jobs[i].pid == pid)
        {
            free(jobs[i].name);
            jobs[i].name = NULL;
            jobs[i].pid = 0;
            for (int j = i; j < 19; j++)
            {
                jobs[j] = jobs[j + 1];
            }
            break;
        }
    }
}


struct Job *getJob(int jobID)
{
    for (int i = 0; i < 20; i++)
    {
        if (jobs[i].pid == jobID)
        {
            return &jobs[i];
        }
    }
    return NULL; // Job with the specified ID not found
}



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
        free(line);
        exit(-1);
    }

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

    args[i] = NULL;

    free(line);
    return i;
}


int main(void)
{
    int original_stdout;
    int redir_flag = 0;
    char *args[20];
    int bg;
    while(1)
    {
        for (int i = 0; i < 20; i++)
        {
            args[i] = NULL;
        }

        bg = 0;
        int cnt = getcmd("\n>> ", args, &bg);

        for (int i = 0; i < cnt; i++)
        {
            if (strcmp(args[i], ">") == 0)
            {
                original_stdout = dup(STDOUT_FILENO); // Store the original standard output
                redir_flag = 1;
                printf("redirection found\n");
                int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd < 0)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                if (dup2(fd, STDOUT_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(fd);

                args[i] = NULL;
                args[i + 1] = NULL;
                cnt -= 2;
                break;
            }
        }


        if (strcmp(args[0], "echo") == 0)
        {
            for (int i = 1; args[i] != NULL; i++)
                printf("%s ", args[i]);
            printf("\n" );
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            chdir(args[1]);
        }
        else if (strcmp(args[0], "pwd") == 0)
        {
            char *buffer;
            size_t size = 1024;
            buffer = (char *)malloc(size);
            getcwd(buffer, size);
            printf("%s\n", buffer);
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            exit(0);
        }
        else if (strcmp(args[0], "fg") == 0)
        {
            int jobID = atoi(args[1]);
            struct Job *job = getJob(jobID);
            if (job)
            {
                printf("Bringing job %d with PID %s to foreground\n", job->pid, job->name);
                waitpid(job->pid, NULL, 0); // wait for the specific process
                removeJob(job->pid);        // Remove the job from list once it's done
            }
            else
            {
                printf("Job %d not found\n", jobID);
            }
        }
        else if (strcmp(args[0], "jobs") == 0)
        {
            for (int i = 0; i < 20; i++)
                {
                    if (jobs[i].name != NULL)
                    {
                        printf("Job %d - PID: %i, Name: %s\n", i, jobs[i].pid, jobs[i].name);
                    }
                }
            printf("\n");
        }
        else
        {
            pid_t pid = fork();

            if (pid<0)
            {
                printf("fork failed");
                return (-1);
            }

            if (pid==0)
            {
                // child process
            }
            else
            {
                if (bg)
                {
                    addjob(pid, args[0]);
                }
                else
                {
                    wait(NULL); // wait for the child process to finish
                }
            }
        }
        if (redir_flag)
        {
            if (dup2(original_stdout, STDOUT_FILENO) < 0)
            {
                perror("dup2");
                exit(EXIT_FAILURE);
            }

            close(original_stdout); // Close the stored file descriptor
            redir_flag = 0;
        }
    }
}