/* List blocked threads in Linux
 * (or others if letter is given on commandline)
 *
 * This Works is placed under the terms of the Copyright Less License,
 * see file COPYRIGHT.CLL.  USE AT OWN RISK, ABSOLUTELY NO WARRANTY.
 */

#define	WIDTH	10

#define	_GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

char *
mkstr(char *dest, size_t max, ...)
{
  va_list	list;
  const char	*ptr;

  va_start(list, max);
  while  ((ptr = va_arg(list, const char *))!=0)
    while (*ptr)
      {
        if (!max--) return 0;
        *dest++ = *ptr++;
      }
  va_end(list);
  if (!max) return 0;
  *dest	= 0;
  return dest;
}

int
fputl(long l, FILE *fd)
{
  int	n;

  n	= 0;
  if (l<0)
    {
      fputc('-', fd);
      l	= -l;
      n	= 1;
    }
  if (l>9)
    n	+= fputl(l/10, fd);
  fputc('0' + (l%10), fd);
  return n+1;
}

void
fputcn(char c, int n, FILE *fd)
{
  while (--n>=0)
    fputc(c, fd);
}

/* see https://stackoverflow.com/a/60441542/490291 */
struct states
  {
    char	c;
    const char	*desc;
  } _states[] =
  { { 'D', "waiting in uninterruptible disk sleep" }
  , { 'I', "idle" }
  , { 'K', "[Wakekill 2.6.33 to 3.13]" }
  , { 'P', "[Parked 3.9 to 3.13]" }
  , { 'R', "running" }
  , { 'S', "sleeping in interruptible wait" }
  , { 'T', "stopped on signal [or traced before 2.6.33]" }
  , { 't', "traced" }
  , { 'W', "[Paging before 2.6.0] [Waking 2.6.33 to 3.13]" }
  , { 'X', "dead" }
  , { 'x', "[Dead 2.6.33 to 3.13]" }
  , { 'Z', "zombie state" }
  , { 0, "[unknown]" }
  };

const struct states *
getstate(unsigned char s)
{
  struct states *st;

  for (st = _states; st->c && st->c!=s; st++);
  return st;
}

void
statout(long stat[256])
{
  for (int k=0; k<256; k++)
    if (stat[k])
      {
        fputc(k, stderr);
        fputc(' ', stderr);
        fputcn(' ', WIDTH-fputl(stat[k], stderr), stderr);
        fputs(getstate(k)->desc, stderr);
        fputc('\n', stderr);
      }
}

int
main(int argc, char **argv)
{
  DIR		*d1, *d2;
  struct dirent	*e1, *e2;
  char		b1[2000], b2[10000];
  long		stat[256];
  char		want[256];

  memset(stat, 0, sizeof stat);
  memset(want, 0, sizeof want);

  /* get wanted states from cmdline	*/

  for (int a=1; a<argc; a++)
    for (const char *ptr = argv[a]; *ptr; )
      want[(unsigned char)*ptr++] |= 1;

  if (argc<2)							/* use '' to not output states	*/
    want['D'] |= 2;						/* default: 'D' (blocked)	*/
  if (want['*'])						/* * list all states	*/
    for (int s=256; --s>=0; )
      want[s] |= 8;
  else if (want['?'])						/* ? list all unknown states	*/
    for (int s=256; --s>=0; )
      {
        const struct states *st;

        st	= getstate(s);
        if (!st->c)
          want[s] |= 4;
      }

  /* find the states	*/

  d1	= opendir("/proc");
  if (!d1)
    {
      fputs("cannot read /proc: ", stderr);
      fputs(strerror(errno), stderr);
      fputs("\n", stderr);
      return 1;
    }
  while ((e1 = readdir(d1))!=0)					/* walk /proc/	*/
    {
      char c = e1->d_name[0];

      if (c<'1' || c>'9') continue;				/* only access PIDs	*/
      if (!mkstr(b1, sizeof b1, "/proc/", e1->d_name, "/task", NULL)) continue;
      d2	= opendir(b1);
      if (!d2) continue;
      while ((e2 = readdir(d2))!=0)				/* walk /proc/PID/task/	*/
        {
          int	fd, n;
          char	*x, *y;

          c	= e2->d_name[0];
          if (c<'1' || c>'9') continue;				/* only access TIDs	*/

          if (!mkstr(b1, sizeof b1, "/proc/", e1->d_name, "/task/", e2->d_name, "/stat", NULL)) continue;
          fd	= open(b1, O_RDONLY);
          if (fd<0) continue;

          n	= read(fd, b2, sizeof b2);			/* read /proc/PID/task/TID/stat	*/
          close(fd);
          if (n<0) continue;

          x	= memrchr(b2, ')', n);				/* find last closing )	*/
          if (!x) continue;
          if (*++x != ' ') continue;				/* must be followed by SPC	*/
          if (++x >= b2+n) continue;				/* state character	*/

          stat[(unsigned char)*x]++;				/* count states	*/

          if (!want[(unsigned char)*x]) continue;

          /* list wanted states	*/

          fputc(*x, stdout);					/* state	*/
          fputc(' ', stdout);
          fputs(e1->d_name, stdout);				/* PID		*/
          fputcn(' ', WIDTH-strlen(e1->d_name), stdout);
          y	= memchr(b2, '(', n);
          if (!y || strcmp(e2->d_name, e1->d_name))
            fwrite(b2, x-b2-1, 1, stdout);			/* TID (proc)	*/
          else
            fwrite(y, x-y-1, 1, stdout);			/* (proc)	*/
          fputc('\n', stdout);
        }
      closedir(d2);
    }
  closedir(d1);
  fflush(stdout);

  /* output statistics	*/
  statout(stat);
  return 0;
}

