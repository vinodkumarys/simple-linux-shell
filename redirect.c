
#define RD_STDIN 	0x1
#define RD_STDOUT 	0x2
#define RD_STDERR 	0x4

#define RD_OUT_CREATE_FLAGS (O_WRONLY | O_CREAT | O_TRUNC)
#define RD_OUT_CREATE_FLAGS_APPEND (O_WRONLY | O_CREAT | O_APPEND)
#define RD_IN_CREATE_FLAGS (O_RDONLY)

#define CREATE_MODE (S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)

typedef struct
{
  int rd_type;
  char *out_file;
  char *in_file;
  char *err_file;
} REDIRECT;

/*--------------------------------------------------------------------------*\
	redirect_execute() - executes given program in foreground or background
	with the support for redirection.
\*--------------------------------------------------------------------------*/
int redirect_execute(char *program,char **args,REDIRECT *rd)
{
  int child_stats,fd,rd_fileno;
  pid_t child_pid;

  /*
  fprintf(stdout,"------- rd_exec start\n");
  fprintf(stdout,"rd_type = %d\n",rd->rd_type);
  fprintf(stdout,"%s\n",rd->in_file);
  fprintf(stdout,"%s\n",rd->out_file);
  fprintf(stdout,"%s\n",rd->err_file);
  fprintf(stdout,"------- rd_exec end");
  return 0;
  */

  child_pid = fork();
  if (child_pid == 0)
  {
    if((rd->rd_type & RD_STDIN) == RD_STDIN)
    {
      if(fileexists(rd->in_file))
      {
	 fd = open(rd->in_file,RD_IN_CREATE_FLAGS, CREATE_MODE);

	 if (dup2(fd,STDIN_FILENO) == -1)
	 {
	   fprintf(stderr,"Error : %s\n",strerror(errno));
	   abort();
	 }
	 if (close(fd) == -1)
	 {
	   fprintf(stderr,"Error : %s\n",strerror(errno));
	   abort();
	 }
      }
      else
      {
		fprintf(stderr,"Error : file \"%s\" not found\n",rd->in_file);
		abort();
      }
    }

    if((rd->rd_type & RD_STDOUT) == RD_STDOUT)
    {
      fd = open(rd->out_file,RD_OUT_CREATE_FLAGS, CREATE_MODE);

      if (dup2(fd,STDOUT_FILENO) == -1)
      {
		fprintf(stderr,"Error : %s\n",strerror(errno));
		abort();
      }

      if (close(fd) == -1)
      {
		fprintf(stderr,"Error : %s\n",strerror(errno));
		abort();
      }
    }

    if((rd->rd_type & RD_STDERR) == RD_STDERR)
    {

      fd = open(rd->err_file,RD_OUT_CREATE_FLAGS, CREATE_MODE);
      if (dup2(fd,STDERR_FILENO) == -1)
      {
		fprintf(stderr,"Error : %s\n",strerror(errno));
		abort();
      }

      if (close(fd) == -1)
      {
		fprintf(stderr,"Error : %s\n",strerror(errno));
		abort();
      }
    }

    if( (rd->rd_type & RD_STDIN)  !=  RD_STDIN &&
	(rd->rd_type & RD_STDOUT) !=  RD_STDOUT &&
	(rd->rd_type & RD_STDERR) !=  RD_STDERR )
    {
      fprintf(stderr,"Error : Invalid redirection type.\n");
      abort();
    }

    execv(program,args);

    fprintf(stderr,"Error executing %s : %s.",program,strerror(errno));
    abort();
  }
  else
  {
    waitpid(child_pid,&child_stats,0);
    /*
    if(WIFEXITED(child_stats))
    printf("Child process exited with code: %d\n",WEXITSTATUS(child_stats));
    else
    printf("Child Process exited abnormally.\n");
    */
  }

  return 0;
}

/*---------------------------- End Of File ---------------------------------*/
