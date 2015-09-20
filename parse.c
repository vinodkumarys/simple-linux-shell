
/*--------------------------------------------------------------------------*\
   make_tokens() - given a string, this fucntion creates a list of tokens
   seperated by the specified	delimiters.
\*--------------------------------------------------------------------------*/
int make_tokens(const char *s, const char *delimiters, char ***argvp)
{
  int error, i, numtokens;
  char *t;
  const char *snew;

  if ((s == NULL) || (delimiters == NULL) || (argvp == NULL))
    {
      errno = EINVAL;
      return -1;
    }

  *argvp = NULL;
  snew = s + strspn(s, delimiters);	/* snew is real start of string */
  if ((t = malloc(strlen(snew) + 1)) == NULL)
    return -1;
  strcpy(t, snew);
  numtokens = 0;
  if (strtok(t, delimiters) != NULL)	/* count the number of tokens in s */
    for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++)
      ;

  /* create argument array for ptrs to the tokens */
  if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL)
    {
      error = errno;
      free(t);
      errno = error;
      return -1;
    }

  /* insert pointers to tokens into the argument array */
  if (numtokens == 0)
    free(t);
  else
    {
      strcpy(t, snew);
      **argvp = strtok(t, delimiters);
      for (i = 1; i < numtokens; i++)
	*((*argvp) + i) = strtok(NULL, delimiters);
    }
  /* put in final NULL pointer */
  *((*argvp) + numtokens) = NULL;

  return numtokens;
}

/*--------------------------------------------------------------------------*\
   free _make_tokens() - deallocates memory allocated by make_tokens().
\*--------------------------------------------------------------------------*/
void free_make_tokens(char **args)
{
  if (args == NULL)
    return;
  if (*args != NULL)
    free(*args);
  free(args);
}

/*--------------------------------------------------------------------------*\
   look_for_executable() - searches for an execuatble file in all the
   specified paths in "PATH" environment variable.
\*--------------------------------------------------------------------------*/
char *look_for_executable(const char *cmd)
{
  char *path_env,**path_entries;
  static char *path_entry_buf;
  int count,i;
  struct stat stat_buf;

  if((path_env = getenv("PATH")) == NULL)
    return NULL;

  count = make_tokens(path_env,":",&path_entries);

  for(i=0;i < count;i++)
    {
      if(path_entry_buf != NULL)
	    free(path_entry_buf);

      path_entry_buf = (char *)malloc(strlen(path_entries[i])+strlen(cmd)+2);
      strcpy(path_entry_buf,path_entries[i]);
      strcat(path_entry_buf,"/");
      strcat(path_entry_buf,cmd);

      bzero(&stat_buf,sizeof(stat_buf));
      stat(path_entry_buf,&stat_buf);
      if(S_ISREG(stat_buf.st_mode))
      {
	return path_entry_buf;
      }
    }

  free_make_tokens(path_entries);
  return NULL;
}
