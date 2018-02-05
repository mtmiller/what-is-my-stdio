/*

what-is-my-stdio - report what my stdin, stdout, and stderr are.
Copyright 2017 Mike Miller

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <errno.h>
#include <glob.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
# define PATH_MAX 8192
#endif

static const char *program_name = "what-is-my-stdio";

const char *
read_proc_link (int fd)
{
  static char lname[PATH_MAX];
  char pname[32];

  if (fd >= 0)
    {
      memset (lname, 0, PATH_MAX);
      sprintf (pname, "/proc/self/fd/%d", fd);
      if (readlink (pname, lname, PATH_MAX - 1) != -1)
        return lname;
    }

  return NULL;
}

const char *
get_command_name (pid_t pid)
{
  static const char *unknown = "unknown";
  static char command_name[PATH_MAX];
  char cmdline_fname[32];
  FILE *f;

  if (pid > 0)
    {
      sprintf (cmdline_fname, "/proc/%d/cmdline", pid);
      f = fopen (cmdline_fname, "rt");
      if (f)
        {
          if (fread (command_name, 1, PATH_MAX - 1, f) > 0)
            return command_name;
          fclose (f);
        }
    }
  return "unknown";
}

const char *
find_pipe_owner (const char *pipe_name)
{
  size_t i;
  pid_t pid;
  glob_t glob_list;
  static const char *unknown = "unknown (unknown)";
  static char found[PATH_MAX];
  static char lname[PATH_MAX];
  char *pid_str_s;
  char *pid_str_e;

  if (glob ("/proc/*/fd/*", 0, NULL, &glob_list) != 0)
    return unknown;

  memset (found, 0, 32);

  for (i = 0; i < glob_list.gl_pathc; i++)
    {
      pid = 0;
      pid_str_s = strdup (&glob_list.gl_pathv[i][6]);
      pid_str_e = strchr (pid_str_s, '/');
      if (pid_str_e)
        {
          *pid_str_e = 0;
          pid = atol (pid_str_s);
        }
      free (pid_str_s);

      if (pid == 0 || pid == getpid ())
        continue;

      memset (lname, 0, PATH_MAX);
      if (readlink (glob_list.gl_pathv[i], lname, PATH_MAX - 1) != -1)
        if (strcmp (pipe_name, lname) == 0)
          {
            sprintf (found, "%s (%d)", get_command_name (pid), pid);
            break;
          }
    }

  globfree (&glob_list);

  return (*found ? found : unknown);
}

void
what_is (FILE *stream, const char *stream_name)
{
  int fd;
  int is_pipe;
  struct stat sb;
  char pname[32];
  static char lname[PATH_MAX];
  const char *proc_name;

  fd = fileno (stream);

  if (fd == -1)
    {
      printf ("%s: %s is not associated with a valid file descriptor\n",
              program_name, stream_name);
      return;
    }

  if (fstat (fd, &sb) == -1)
    {
      switch (errno)
        {
          case EBADF:
            printf ("%s: %s is not an open file descriptor\n", program_name,
                    stream_name);
            break;
          default:
            printf ("%s: fstat on %s failed: %s\n", strerror (errno));
            break;
        }
      return;
    }

  proc_name = read_proc_link (fd);
  if (proc_name && strncmp (proc_name, "pipe:[", 6) == 0)
    is_pipe = 1;
  else
    is_pipe = 0;

  printf ("%s: %s is ", program_name, stream_name);

  if (isatty (fd))
    printf ("a terminal");
  else if (S_ISBLK (sb.st_mode))
    printf ("a block special device");
  else if (S_ISCHR (sb.st_mode))
    printf ("a character special device");
  else if (S_ISDIR (sb.st_mode))
    printf ("a directory");
  else if (S_ISFIFO (sb.st_mode) && is_pipe)
    printf ("a pipe");
  else if (S_ISFIFO (sb.st_mode))
    printf ("a named pipe");
  else if (S_ISREG (sb.st_mode))
    printf ("a regular file");
  else if (S_ISLNK (sb.st_mode))
    printf ("a symbolic link");
  else if (S_ISSOCK (sb.st_mode))
    printf ("a socket");
  else
    printf ("an unknown file type");

  if (S_ISFIFO (sb.st_mode) && is_pipe)
    printf (" with the other end owned by %s", find_pipe_owner (proc_name));
  else if (proc_name)
    printf (", open to %s", proc_name);
  else
    printf (", open to an unknown file");

  printf ("\n");
}

int
main (int argc, char *argv[])
{
  what_is (stdin, "stdin");
  what_is (stdout, "stdout");
  what_is (stderr, "stderr");

  return 0;
}
