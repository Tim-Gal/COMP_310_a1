#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


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
    char *args[20];
    int bg;
    while(1) {

        for (int i = 0; i < 20; i++)
        {
            args[i] = NULL;
        }

        bg = 0;
        int cnt = getcmd("\n>> ", args, &bg);

        if (args[0] == NULL)
        {
            printf("No command entered.\n");
            continue;
        }

        if (strcmp(args[0], "echo") == 0)
        {
            for (int i = 1; args[i] != NULL; i++)
                printf("%s ", args[i]);
            printf("\n");
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
    }
}