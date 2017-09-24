#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include "command.c"

// cat log.txt | grep 18/Oct/2006 | cut -d \" -f4 | grep -v -x "-" | sort | uniq -c | sort -nrk 1 | head -10 | tr -s " "
int main(int argc, char *argv[]) {
//    if (argc < 2)
//        return -1;

    const int TOP = 10;

    char *pathToFile = "log.txt";

    const char *cat[] = {"cat", pathToFile, 0};
    const char *grep1[] = {"grep", "18/Oct/2006", 0};
    const char *cut[] = {"cut", "--delimiter=\"", "-f4", 0};
    const char *grep2[] = {"grep", "-v", "-x", "-", 0};
    const char *sort1[] = {"sort", 0};
    const char *uniq[] = {"uniq", "-c", 0};
    const char *sort2[] = {"sort", "-nrk", "1", 0};
    const char *head[] = {"head", "-10", 0};
    const char *tr[] = {"tr", "-s", "' '", 0};

    struct command cmd[] = {{cat},
                            {grep1},
                            {cut},
                            {grep2},
                            {sort1},
                            {uniq},
                            {sort2},
                            {head},
                            {tr}};

    int fd[2];
    pipe(fd);

    pid_t pid = fork();


    if (!pid) {
        int in = fork_pipes(9, cmd);

        //----reading
        char ch[1];
        int i = 0;
        char result[1024];
        while (read(in, ch, 1)) {
            result[i++] = ch[0];
        }
        result[i] = NULL;
        close(in);
        //----------

        //-----calculating percentage
        int j = 0;
        int numbers[TOP];
        char referers[TOP][1024];
        int k = 0;
        while (result[j] != NULL) {
            j++;
            i = 0;
            char num[TOP];
            while(result[j] != ' ') {

                num[i++] = result[j++];
            }
            num[i] = '\n';
            numbers[k] = (int) strtol(num, (char **)NULL, 10);
            int l=0;
            while(result[j] != '\n'){
                    referers[k][l++]=result[j++];
            }
            referers[k][l] = NULL;
            k++;
            while (result[j++] != '\n');
        }
        int sum = 0;
        for (int l = 0; l < TOP; ++l) {
                sum+=numbers[l];
        }
        for (int m = 0; m < TOP; ++m) {
            printf("%s - %d - %f\%\n", referers[m], numbers[m], (double)numbers[m]*100/sum);
        }
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(pid, NULL, 0);


    return 0;
}

int spawn_proc(int in, int out, struct command *cmd) {
    pid_t pid;

    if ((pid = fork()) == 0) {
        if (in != 0) {
            dup2(in, 0);
            close(in);
        }
        if (out != 1) {
            dup2(out, 1);
            close(out);
        }

        return execvp(cmd->argv[0], (char *const *) cmd->argv);
    }
    return pid;
}

int fork_pipes(int n, struct command *cmd) {
    int i;
    pid_t pid;
    int in, fd[2];

    in = 0;

    for (i = 0; i < n; ++i) {
        pipe(fd);

        spawn_proc(in, fd[1], cmd + i);

        close(fd[1]);

        in = fd[0];
    }

    return in;
}