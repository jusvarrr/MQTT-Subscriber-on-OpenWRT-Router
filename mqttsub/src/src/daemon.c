#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "daemon.h"
#include <syslog.h>

int // returns 0 on success -1 on error
become_daemon(int flags)
{
  int maxfd, fd;
  switch(fork())                    // become background process
  {
    case -1: return -1;
    case 0: break;                  // child falls through
    default: _exit(EXIT_SUCCESS);   // parent terminates
  }

  if(setsid() == -1)                // become leader of new session
    return -1;

  switch(fork())
  {
    case -1: return -1;
    case 0: break;                  // child breaks out of case
    default: _exit(EXIT_SUCCESS);   // parent process will exit
  }

  if(!(flags & BD_NO_UMASK0))
    umask(0);                       // clear file creation mode mask

  if(!(flags & BD_NO_CHDIR))
    chdir("/");                     // change to root directory

if (!(flags & BD_NO_CLOSE_FILES)) {
        // Log the max number of open files, if available
        maxfd = sysconf(_SC_OPEN_MAX);
        if (maxfd == -1) {
            syslog(LOG_USER | LOG_ERR, "sysconf(_SC_OPEN_MAX) failed, using fallback.");
            maxfd = BD_MAX_CLOSE;  // Use fallback value
        }
        syslog(LOG_USER | LOG_INFO, "Max open files: %d", maxfd);

        // Close only stdin, stdout, stderr
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

  if(!(flags & BD_NO_REOPEN_STD_FDS))
  {
    close(STDIN_FILENO);

    fd = open("/dev/null", O_RDWR);
    if(fd != STDIN_FILENO)
      return -1;
    if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
      return -2;
    if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
      return -3;
  }

  return 0;
}