/*--------------------------------------------------------------------------*\
	execute() - executes given program in foreground or background.
\*--------------------------------------------------------------------------*/
int execute(char *program,char **args)
{
  int child_status;
  pid_t child_pid;

  child_pid = fork();
  if(child_pid == 0)
  {
    execve(program,args,environ);

    fprintf(stderr,"Error executing %s : %s.",program,strerror(errno));
    abort();
  }
  else
  {
    waitpid(child_pid,&child_status,0);
    /*
    if(WIFEXITED(child_status))
      printf("Child process exited with code : %d\n",WEXITSTATUS(child_status));
    else
      printf("Child Process exited abnormally.\n");
    */
  }
  return 0;
}

/*----------------------------- End Of File --------------------------------*/
