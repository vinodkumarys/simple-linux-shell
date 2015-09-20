
#define BLKSIZE 1024

/*----------------------------------------------------------------------------*\
	fd_copyfile() - low level file copy fucntion
\*----------------------------------------------------------------------------*/
int fd_copyfile(int fromfd, int tofd)
{
  char buf[BLKSIZE];
  int bytesread, byteswritten;
  int totalbytes = 0;

  for (;;)
    {
      if ((bytesread = read(fromfd, buf, BLKSIZE)) <= 0)
        break;
      if ((byteswritten = write(tofd, buf, bytesread)) == -1)
        break;
      totalbytes += byteswritten;
    }
  return totalbytes;
}

/*----------------------------------------------------------------------------*\
	copyfile() - copies from one file to another
\*----------------------------------------------------------------------------*/
int copyfile(char *file1,char *file2)
{
  int fd1,fd2;

  if((fd1 = open(file1,O_RDONLY)) == - 1)
    {
      fprintf(stderr,"Error : %s\n",strerror(errno));
      return 1;
    }
  if((fd2 = open(file2,O_WRONLY | O_CREAT | O_TRUNC )) == - 1)
    {
      fprintf(stderr,"Error : %s\n",strerror(errno));
      return 2;
    }

  fd_copyfile(fd1,fd2);

  close(fd1),close(fd2);
  return 0;
}

/*----------------------------------------------------------------------------*\
	fileexists() - checks if the given file exists or not
\*----------------------------------------------------------------------------*/
int fileexists(char *file)
{
  if(access(file,F_OK) != -1)
	return 1;
  return 0;
}

int filereadable(char *file)
{
  if(access(file,R_OK) != -1)
	return 1;
  return 0;
}

int filewritable(char *file)
{
  if(access(file,W_OK) != -1)
	return 1;
  return 0;
}

int fileexecuatble(char *file)
{
  if(access(file,X_OK) != -1)
	return 1;
  return 0;
}


/*----------------------------------------------------------------------------*\
	trim() - Strip whitespace from the start and end of the given STRING.
\*----------------------------------------------------------------------------*/
char *trim(char *string)
{
  register char *s, *t;

  for (s = string; whitespace (*s); s++)
    ;
  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

/*----------------------------------------------------------------------------*\
	get3sindex() - used to get corresponding filenames from 
	redirection command given by the user.
\*----------------------------------------------------------------------------*/
int get3sindex(int a,int b,int c,int i,int type)
{
  if(type == 1 || type == 2 || type ==4)
    return 1;

  switch(type)
    {
    case 1: return ((i == 1) ? 1 : 0);
    case 2: return ((i == 2) ? 1 : 0);
    case 4: return ((i == 3) ? 1 : 0);

    case 3:
      if(i == 1) return ((a < b) ? 1 : 2);
      if(i == 2) return ((b < a) ? 1 : 2);
      break;

    case 5:
      if(i == 1) return ((a < c) ? 1 : 2);
      if(i == 3) return ((c < a) ? 1 : 2);
      break;

    case 6:
      if(i == 2) return ((b < c) ? 1 : 2);
      if(i == 3) return ((c < b) ? 1 : 2);
      break;

    case 7:
      if(i == 1 && a < b && a < c) return 1;
      if(i == 1 && a > b && a < c) return 2;
      if(i == 1 && a < b && a > c) return 2;
      if(i == 1 && a > b && a > c) return 3;

      if(i == 2 && b < a && b < c) return 1;
      if(i == 2 && b > a && b < c) return 2;
      if(i == 2 && b < a && b > c) return 2;
      if(i == 2 && b > a && b > c) return 3;

      if(i == 3 && c < b && c < a) return 1;
      if(i == 3 && c > b && c < a) return 2;
      if(i == 3 && c < b && c > a) return 2;
      if(i == 3 && c > b && c > a) return 3;
      break;
    }
  return 0;
}

/*----------------------------------------------------------------------------*\
	strchrcount() - count no. of occurences of a character in a given string
\*----------------------------------------------------------------------------*/
int strchrcount(char *str,char ch)
{
  int count=0,i,len = strlen(str);
  for(i=0;i<len;i++)
    if(str[i] == ch)
      count++;

  return count;
}

/*----------------------------------------------------------------------------*\
	get_prompt() - returns the string used as the prompt for the user
\*----------------------------------------------------------------------------*/
const char *get_prompt()
{
    return "$ ";
}
