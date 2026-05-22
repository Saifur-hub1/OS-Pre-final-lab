#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

pid_t fg_pid = -1;   // foreground process id

/* ---------------- SIGINT handler ---------------- */
void sigint_handler(int sig) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGINT);  // forward Ctrl-C to child only
    }
}

/* ---------------- parse into argv ---------------- */
int parse(char *line, char **argv) {
    int argc = 0;
    char *tok = strtok(line, " \t\n");

    while (tok && argc < MAX_ARGS - 1) {
        argv[argc++] = tok;
        tok = strtok(NULL, " \t\n");
    }

    argv[argc] = NULL;
    return argc;
}

/* ---------------- split by symbol ---------------- */
int find_symbol(char **argv, const char *sym) {
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], sym) == 0)
            return i;
    }
    return -1;
}

/* ---------------- execute simple command ---------------- */
void exec_simple(char **argv, int background) {
    pid_t pid = fork();

    if (pid == 0) {
        signal(SIGINT, SIG_DFL); // child gets Ctrl-C normally
        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    }
    else if (pid > 0) {
        if (!background) {
            fg_pid = pid;
            waitpid(pid, NULL, 0);
            fg_pid = -1;
        }
    }
    else {
        perror("fork");
    }
}

/* ---------------- pipe execution ---------------- */
void exec_pipe(char **left, char **right, int background) {
    int fd[2];
    pipe(fd);

    pid_t p1 = fork();

    if (p1 == 0) {
        signal(SIGINT, SIG_DFL);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(left[0], left);
        perror("execvp left");
        exit(1);
    }

    pid_t p2 = fork();

    if (p2 == 0) {
        signal(SIGINT, SIG_DFL);
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        execvp(right[0], right);
        perror("execvp right");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);

    if (!background) {
        fg_pid = p1;
        waitpid(p1, NULL, 0);
        waitpid(p2, NULL, 0);
        fg_pid = -1;
    }
}

/* ---------------- redirection ---------------- */
void handle_redirection(char **argv, int background) {
    int idx = find_symbol(argv, ">");

    if (idx == -1) {
        exec_simple(argv, background);
        return;
    }

    char *outfile = argv[idx + 1];
    argv[idx] = NULL;

    pid_t pid = fork();

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);

        int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open");
            exit(1);
        }

        dup2(fd, STDOUT_FILENO);
        close(fd);

        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    }
    else if (pid > 0) {
        if (!background) {
            fg_pid = pid;
            waitpid(pid, NULL, 0);
            fg_pid = -1;
        }
    }
}

/* ---------------- main shell ---------------- */
int main() {
    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    signal(SIGINT, sigint_handler);

    while (1) {
        printf("mysh > ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        int argc = parse(line, argv);
        if (argc == 0) continue;

        if (strcmp(argv[0], "exit") == 0)
            break;

        /* -------- background check -------- */
        int background = 0;
        if (strcmp(argv[argc - 1], "&") == 0) {
            background = 1;
            argv[argc - 1] = NULL;
        }

        /* -------- built-in cd -------- */
        if (strcmp(argv[0], "cd") == 0) {
            if (argv[1] == NULL)
                chdir(getenv("HOME"));
            else
                chdir(argv[1]);
            continue;
        }

        /* -------- pipe handling -------- */
        int p = find_symbol(argv, "|");
        if (p != -1) {
            argv[p] = NULL;
            exec_pipe(argv, &argv[p + 1], background);
            continue;
        }

        /* -------- redirection -------- */
        handle_redirection(argv, background);
    }

    return 0;
}
