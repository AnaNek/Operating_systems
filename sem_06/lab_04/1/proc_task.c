#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

#define BUF_SIZE 0x100

int read_env()
{
    char buf[BUF_SIZE];
    int len, i;
    FILE *f = fopen("/proc/self/environ", "r");

    if (!f)
    {
        printf("env: fopen err\n");
        return -1;
    }

    while ((len = fread(buf, 1, BUF_SIZE, f)) > 0)
    {
        for (i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = 10;
        buf[len - 1] = 0;
        printf("%s", buf);
    }

    if (fclose(f))
    {
        printf("env: fclose err\n");
        return -1;
    }

    return 0;
}

int read_cmdline()
{
    char buf[BUF_SIZE];
    int len, i;
    FILE *f = fopen("/proc/self/cmdline", "r");

    if (!f)
    {
        printf("cmdline: fopen err\n");
        return -1;
    }

    while ((len = fread(buf, 1, BUF_SIZE, f)) > 0)
    {
        for (i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = 10;
        buf[len - 1] = 0;
        printf("%s", buf);
    }

    if (fclose(f))
    {
        printf("cmdline: fclose err\n");
        return -1;
    }

    return 0;
}

int read_stat()
{
    char *records[] = {"pid", "comm", "state", "ppid", "pgrp", "session", "tty_nr", "tpgid", "flags", "minflt",
    "cminflt", "majflt", "cmajflt", "utime", "stime", "cutime", "cstime", "priority", "nice", "num_threads",
    "itrealvalue", "starttime", "vsize", "rss", "rsslim", "startcode", "endcode", "startstack", "kstkesp",
    "kstkeip", "signal", "blocked", "sigignore", "sigcatch", "wchan", "nswap", "cnswap", "exit_signal",
    "processor", "rt_priority", "policy", "delayacct_blkio_ticks", "guest_time", "cguest_time", "start_data",
    "end_data", "start_brk", "arg_start", "arg_end", "env_start", "env_end", "exit_code"};

    int i;
    char buf[BUF_SIZE];
    FILE *f = fopen("/proc/self/stat", "r");

    if (!f)
    {
        printf("stat: fopen err\n");
        return -1;
    }

    fread(buf, 1, BUF_SIZE, f);
    char *pch = strtok(buf, " ");

    i = 0;
    while (pch != NULL)
    {
        printf("%s = %s\n", records[i], pch);
        pch = strtok(NULL, " ");
        i++;
    }

    if (fclose(f))
    {
        printf("stat: fclose err\n");
        return -1;
    }

    return 0;
}

int read_fd()
{
    struct dirent *dirp;
    DIR *dp;
    char str[BUF_SIZE];
    char path[BUF_SIZE];
    dp = opendir("/proc/self/fd");//открыть директорию

    if (!dp)
    {
        printf("fd: opendir err\n");
        return -1;
    }

    while ((dirp = readdir(dp)) != NULL)//читаем директорию
    {
        if ((strcmp(dirp->d_name, ".") != 0) &&
        (strcmp(dirp->d_name, "..") != 0))
        {
            sprintf(path, "%s%s", "/proc/self/fd/", dirp->d_name);
            readlink(path, str, BUF_SIZE);
            printf("%s -> %s\n", dirp->d_name, str);
        }
    }

    if (closedir(dp))
    {
        printf("fd: closedir err\n");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    printf("ENVIRON\n");
    if (read_env())
        return -1;

    printf("\n\nCMDLINE\n");
    if (read_cmdline())
        return -1;

    printf("\n\nSTAT\n");
    if (read_stat())
        return -1;

    printf("\n\nFD\n");
    if (read_fd())
        return -1;

    return 0;
}
