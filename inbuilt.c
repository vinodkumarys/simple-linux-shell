
FILE *my_stdout,*my_stdin,*my_stderr;

/*--------------------------------------------------------------------------*\
	exit_handler() - exits shell
\*--------------------------------------------------------------------------*/
int exit_handler(int argc,char **argv)
{
  exit(0);
}

/*--------------------------------------------------------------------------*\
	md_handler() - make directory
\*--------------------------------------------------------------------------*/
int md_handler(int argc,char **argv)
{
  /* set dir access permission to 755. only the user has write acess */
  mode_t default_mode = S_IRWXU|~S_IWGRP|~S_IWOTH;
  
  if(argc != 2) /* if no directry name is specified */
    return 1;

  if(mkdir(argv[1],default_mode) != 0) /* if failed */
    fprintf(my_stderr,"Error : %s\n",strerror(errno));
  
  return 0;
}

/*--------------------------------------------------------------------------*\
	cd_handler() - change directory
\*--------------------------------------------------------------------------*/
int cd_handler(int argc,char **argv)
{
  if(argc != 2) /* if no directry name is specified */
    return 1;

  if(chdir(argv[1]) == -1)
    {
      fprintf(my_stderr,"Error : %s\n",strerror(errno));
      return 1;
    }
  return  0;
}

/*--------------------------------------------------------------------------*\
	rd_handler() - remove directory
\*--------------------------------------------------------------------------*/
int rd_handler(int argc,char **argv)
{
  if(argc != 2) /* if no directry name is specified */
    return 1;
  
  if(rmdir(argv[1]) != 0) /* if failed */
    fprintf(my_stderr,"Error : %s\n",strerror(errno));
  
  return 0;
}

/*--------------------------------------------------------------------------*\
	cwd_handler() - display current working directory
\*--------------------------------------------------------------------------*/
int cwd_handler(int argc,char **argv)
{
  char my_cwd[PATH_MAX];
  
  if(getcwd(my_cwd,PATH_MAX) == NULL) /* if failed */
    {
      fprintf(my_stderr,"Error : %s\n",strerror(errno));
      return -1;
    }
  else
    fprintf(my_stdout,"%s\n",my_cwd);
  
  return 0;
}

/*--------------------------------------------------------------------------*\
	hex_handler() - display hex dump of the given file
\*--------------------------------------------------------------------------*/
int hex_handler(int argc,char **argv)
{
  unsigned char buffer[16];
  size_t offset = 0;
  size_t bytes_read;
  int i,fd;

  /* if no arguments are passed, display usage information */
  if(argc != 2)
    {
      fprintf(my_stderr,"Usage : %s <file-name>\n",argv[0]);
      return -1;
    }

  /* open file in read only mode */
  if((fd = open( argv[1], O_RDONLY)) == -1)
    {
      fprintf(my_stderr,"Error : %s\n",strerror(errno));
      return -1;
    }
  
  /* read from the file, one chunk (16 bytes) at a time. continue until
     read comes up short, that is, reads less than we asked for. this
     indicates that we’ve hit the end of the file. */
  do
    {
      /* read the next line’s worth of bytes. */
      bytes_read = read (fd, buffer, sizeof (buffer));

      /* print the offset in the file, followed by the bytes themselves. */
      fprintf(my_stdout,"0x%06x : ", offset);
      for (i = 0; i < bytes_read; ++i)
        fprintf(my_stdout,"%02x ", buffer[i]);

      fprintf(my_stdout,": ");

      /* print corresponding ascii values */
      for (i = 0; i < bytes_read; ++i)
        fprintf(my_stdout,"%c",(isalnum(buffer[i])) ? buffer[i] : '.');
      fprintf(my_stdout,"\n");

      offset += bytes_read;
    }
  while (bytes_read == sizeof (buffer));

  close (fd);
  return 0;
}

/*--------------------------------------------------------------------------*\
	copy_handler() - copy file
\*--------------------------------------------------------------------------*/
int copy_handler(int argc,char **argv)
{
  if(argc != 3) /* if invalid no. of arguments display usage info */
    {
      fprintf(my_stderr,"Usage : %s <src-file> <dest-file>\n",argv[0]);
      return 1;
    }

  return copyfile(argv[1],argv[2]);
}

/*--------------------------------------------------------------------------*\
	move_handler() - move file
\*--------------------------------------------------------------------------*/
int move_handler(int argc,char **argv)
{
  if(argc != 3) /* if invalid no. of arguments display usage info */
    {
      fprintf(my_stderr,"Usage : %s <src-file> <dest-file>\n",argv[0]);
      return 1;
    }

  /* first copy the file, and then if sucessfull, try to
     delete the original file. else display error */
  if(copyfile(argv[1],argv[2]) == 0)
    {
      if(unlink(argv[1]) == -1)
        {
          fprintf(my_stderr,"Error : %s\n",strerror(errno));
          return 2;
        }
    }

  return 0; /* return successfull */
}

/*--------------------------------------------------------------------------*\
	del_handler() - delete file
\*--------------------------------------------------------------------------*/
int del_handler(int argc,char **argv)
{
  if(argc != 2) /* if invalid no. of arguments display usage info */
    {
      fprintf(my_stderr,"Usage : %s <file-name>\n",argv[0]);
      return 1;
    }

  /* try to delete the file, if unsucessfull, display error */
  if(unlink(argv[1]) == -1)
    {
      fprintf(my_stderr,"Error : %s\n",strerror(errno));
      return 2;
    }

  return 0; /* return successfull */
}

/*--------------------------------------------------------------------------*\
	rem_handler() - delete file
\*--------------------------------------------------------------------------*/
int rem_handler(int argc,char **argv)
{
  if(argc != 3) /* if invalid no. of arguments display usage info */
    {
      fprintf(my_stderr,"Usage : %s <old-filename> <new-filename>\n",argv[0]);
      return 1;
    }

/* try to rename the file, if unsucessfull, display error */
  if(rename(argv[1],argv[2]) == -1)
    {
      fprintf(my_stderr,"Error : %s\n",strerror(errno));
      return 2;
    }

  return 0; /* return successfull */
}


/*--------------------------------------------------------------------------*\
	fcat_handler() - file concatination and display
\*--------------------------------------------------------------------------*/
int fcat_handler(int argc,char **argv)
{
  FILE *fptr;
  int i;
  char ch;

  /* if no arguments is passed, read from stdin and write to stdout */
  if(argc < 2)
    {
      while((ch = fgetc(my_stdin)) != EOF)
        fputc(ch,my_stdout);
    }
  else
    {
	  /* display contents of each file passed as aruments */
      for(i = 1;i < argc;i++)
        {
        if((fptr = fopen(argv[i],"r")) == NULL)
          {
            fprintf(my_stderr,"Error : %s\n",strerror(errno));
            return 1;
          }
        while((ch = fgetc(fptr)) != EOF)
          fputc(ch,my_stdout);
        }
    }
  return 0;
}

/*--------------------------------------------------------------------------*\
	about_handler() - display about information
\*--------------------------------------------------------------------------*/
int about_handler(int argc,char **argv)
{
  fprintf( my_stdout,
		   "Simple Shell(ssh) : a simple linux shell implementation\n");
  fprintf( my_stdout,"Created by : Vinod Kumar Y.S\n");
}

/*--------------------------------------------------------------------------*\
	env_handler() - display environment variables
\*--------------------------------------------------------------------------*/
int env_handler(int argc,char **argv)
{
  int i;
  for(i=0; environ[i]!=NULL; i++)
    fprintf(my_stdout,"%s\n",environ[i]);
  return 0;
}

/*--------------------------------------------------------------------------*\
	dir_handler() - display list of files in a directory
\*--------------------------------------------------------------------------*/
const char* get_file_type (const char* path)
{
  struct stat st;
  lstat (path, &st);
  if (S_ISLNK (st.st_mode))
    return "link";
  else if (S_ISDIR (st.st_mode))
    return "dtry";
  else if (S_ISCHR (st.st_mode))
    return "chrd";
  else if (S_ISBLK (st.st_mode))
    return "blkd";
  else if (S_ISFIFO (st.st_mode))
    return "fifo";
  else if (S_ISSOCK (st.st_mode))
    return "sock";
  else if (S_ISREG (st.st_mode))
    return "regf";
  else
    /* Unexpected.  Each entry should be one of the types above.  */
    return "ukwn";
}

int dir_handler(int argc, char* argv[])
{
  char *dir_path,*strtime;
  DIR *dir;
  struct dirent *entry;
  char entry_path[PATH_MAX + 1];
  size_t path_len;
  struct stat file_stat;

  if (argc >= 2)
    /* If a directory was specified on the command line, use it. */
    dir_path = argv[1];
  else
    /* Otherwise, use the current directory. */
    dir_path = ".";
  
  /* Copy the directory path into entry_path. */
  strncpy (entry_path, dir_path, sizeof (entry_path));
  path_len = strlen (dir_path);
  
  /* If the directory path doesn’t end with a slash, append a slash.  */
  if (entry_path[path_len - 1] != '/')
    {
      entry_path[path_len] = '/';
      entry_path[path_len + 1] = '\0';
      ++path_len;
    }
  
  /* Start the listing operation of the directory specified on the command line.  */
  dir = opendir (dir_path);
  
  /* Loop over all directory entries. */
  while ((entry = readdir (dir)) != NULL)
    {
      if(!strcmp(entry->d_name,"."))
        continue;
      if(!strcmp(entry->d_name,".."))
        continue;
      const char* type;
      /* Build the path to the directory entry by appending the entry name to the path name. */
      strncpy (entry_path + path_len, entry->d_name,sizeof (entry_path) - path_len);
      /* Determine the type of the entry. */
      type = get_file_type (entry_path);

      bzero(&file_stat,sizeof(file_stat));
      stat(entry_path,&file_stat);
	  
	  strtime = ctime(&file_stat.st_atime);
	  strtime += 4;	/* skip day */
	  if(*(strtime + 4) == ' ')  *(strtime + 4) = '0'; 
	  *(strtime + 12) = '\0'; /* skip seconds and year */
	  
      /* Print the type and path of the entry. */
      fprintf (my_stdout,"<%s> %9u %s %s\n",type,file_stat.st_size,strtime,entry->d_name);
    }
  
  /* all done */
  closedir (dir);
  return 0;
}

/*--------------------------------------------------------------------------*\
	help_handler() - help
\*--------------------------------------------------------------------------*/
int help_handler(int argc,char **argv)
{
  int i;
  fprintf(my_stdout,"The following %d commands are supported :\n",cmd_count);
  for(i=0; i < cmd_count; i++)
    fprintf(my_stdout,"%7s : %s\n",cmd_list[i].cmd,cmd_list[i].info);
  return 0;
}

/*--------------------------------------------------------------------------*\
	date_handler() - display current date and time
\*--------------------------------------------------------------------------*/
int date_handler(int argc,char **argv)
{
  char *strdatetime;
  time_t timeptr = time(0);
  strdatetime = (char *)ctime(&timeptr);
  fprintf(my_stdout,"%s",strdatetime);
}

/*--------------------------------------------------------------------------*\
	date_handler() - display current date and time
\*--------------------------------------------------------------------------*/
int print_handler(int argc,char **argv)
{
  int i;
  char *tmpstr;
  for(i = 1;i < argc;i++)
    if(argv[i][0] != '$')
      fprintf(my_stdout,"%s ",argv[i]);
    else
      {
        tmpstr = argv[1];
        tmpstr++;
        fprintf(my_stdout,getenv(tmpstr));
      }
  fprintf(my_stdout,"\n");
}

int sysinfo_handler(int argc,char **argv)
{
  /* Conversion constants.  */
  const long minute = 60;
  const long hour = minute * 60;
  const long day = hour * 24;
  const double megabyte = 1024 * 1024;
  struct utsname u;
  struct sysinfo si;

  uname (&u);
  fprintf (my_stdout,"%s release %s on %s\n", u.sysname,u.release,u.machine);

  /* Obtain system statistics.  */
  sysinfo (&si);
  /* Summarize interesting values.  */
  fprintf (my_stdout,"system uptime : %ld days, %ld:%02ld:%02ld\n",
           si.uptime / day, (si.uptime % day) / hour,
           (si.uptime % hour) / minute, si.uptime % minute);
  fprintf (my_stdout,"total RAM     : %5.1f MB\n", si.totalram / megabyte);
  fprintf (my_stdout,"free RAM      : %5.1f MB\n", si.freeram / megabyte);
  fprintf (my_stdout,"process count : %d\n", si.procs);
  return 0;
}

/*----------------------------- End Of File --------------------------------*/
