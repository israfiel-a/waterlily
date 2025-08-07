#include <Waterlily.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

bool waterlily_files_execute(char *const *args)
{
    int pid = fork();
    if (pid < 0)
    {
        waterlily_engine_log(ERROR, "Failed to fork process, code %d.", pid);
        return false;
    }

    if (pid == 0)
    {
        if (args[0] == nullptr)
            return false;
        (void)execve(args[0], args, nullptr);
        waterlily_engine_log(
            ERROR, "Failed to replace current process, code %d.", errno);
        return false;
    }

    int status;
    int waitStatus = waitpid(pid, &status, 0);
    if (waitStatus < 0)
    {
        waterlily_engine_log(ERROR, "Child process was killed via signal.");
        return false;
    }

    if (status == 1)
    {
        waterlily_engine_log(ERROR, "Child process exited abnormally.");
        return false;
    }

    return true;
}

