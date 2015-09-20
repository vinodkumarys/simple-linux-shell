
/*---------------------- Preprocessor declarations -------------------------*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>

#include <fcntl.h>
#include <dirent.h>

#include <unistd.h>
#include <assert.h>

#include <readline/readline.h>	/* for user input and command history */

#include <readline/history.h>

#include <sys/stat.h>	/* for file information */
#include <sys/wait.h>

#include <sys/utsname.h>	/* system information */
#include <sys/sysinfo.h>	/* system statistics information */

#define USE_EXPAND

/*--------------- Internal commands Fuction Prototypes ---------------------*/

int exit_handler(int,char **);
int md_handler(int,char **);
int cd_handler(int,char **);
int rd_handler(int,char **);
int cwd_handler(int,char **);
int env_handler(int,char **);
int date_handler(int,char **);
int about_handler(int,char **);
int help_handler(int,char **);
int dir_handler(int,char **);
int hex_handler(int,char **);
int copy_handler(int,char **);
int move_handler(int,char **);
int del_handler(int,char **);
int sysinfo_handler(int,char **);
int ren_handler(int,char **);
int fcat_handler(int,char **);
int print_handler(int,char **);

/*------------------------- Global Variables -------------------------------*/

extern char **environ;
char *buf;

typedef struct
{
  char *cmd;
  char *info;
  int (*handler)(int argc,char **argv);
}internal_command;

internal_command cmd_list[] =
{
  {"env","display environment variables",env_handler},
  {"dir","list all the files in the current directory",dir_handler},
  {"help","display this help",help_handler},
  {"about","diplay about information",about_handler},
  {"md","make directory",md_handler},
  {"cd","change directory",cd_handler},
  {"rd","remove directory",rd_handler},
  {"print","prints the give string onto standard output",print_handler},
  {"date","display system date",date_handler},
  {"copy","copy file from one location to another",copy_handler},
  {"move","move file from one location to another",move_handler},
  {"del","deletes a file",del_handler},
  {"ren","renames a file",del_handler},
  {"fcat","fcat files and displays it on standard output",fcat_handler},
  {"hex","display hex dump of the given file",hex_handler},
  {"cwd","display current working directory",cwd_handler},
  {"sysinfo","display system information",sysinfo_handler},
  {"exit","exit shell",exit_handler}
};

int cmd_count = sizeof(cmd_list)/sizeof(internal_command);

/*----------------------------- Modules ------------------------------------*/

#include "utils.c"
#include "parse.c"
#include "inbuilt.c"
#include "exec.c"
#include "redirect.c"

#ifdef USE_EXPAND
char ex_buf[5000];
#include "expand.c"
#endif

/*--------------------- Execute command Functions --------------------------*/

int execute_external_cmd(char *cmd,char **args,REDIRECT *rd)
{
  char *exe_path;

  /* if absolute path is given and if valid, execute it directly */
  if(fileexists(cmd))
    exe_path = cmd;
  else /* else search for the corresponding program */
    exe_path = look_for_executable(cmd);

  if(exe_path != NULL) /* if valid program found */
  {
      /* args[0] = exe_path_buf; */
     if( rd->rd_type == 0 )
       execute(exe_path,args);
     else
       redirect_execute(exe_path,args,rd);
  }
  else /* progam not found */
    printf("\"%s\" command not found.\n",cmd);
  return 0;
}

int execute_internal_cmd(char *cmd,int argc,char **argv,REDIRECT *rd)
{
  int i;
  for(i=0;i < cmd_count;i++)
  if(!strcmp(cmd_list[i].cmd,cmd)) /* true if cmd is an internal command */
  {
    /* check for redirection and open corresponding files*/
	
    if((rd->rd_type & RD_STDIN) ==  RD_STDIN)
    {
	  if((my_stdin = fopen(rd->in_file,"r")) == NULL)
	  {
	    fprintf(stderr,"Error : %s\n",strerror(errno));
	    return 0;
	  }
    }
    else
      my_stdin = stdin;

    if((rd->rd_type & RD_STDOUT) ==  RD_STDOUT)
    {
	  if((my_stdout = fopen(rd->out_file,"w")) == NULL)
	  {
	    fprintf(stderr,"Error : %s\n",strerror(errno));
	    return 0;
	  }
    }
    else
      my_stdout = stdout;

    if((rd->rd_type & RD_STDERR) ==  RD_STDERR)
    {
	  if((my_stderr = fopen(rd->err_file,"w")) == NULL)
	  {
	    fprintf(stderr,"Error : %s\n",strerror(errno));
	    return 0;
	  }
    }
    else
      my_stderr = stderr;

	/* execute the internal command */
    cmd_list[i].handler(argc,argv);

	/* close opened files */
    if((rd->rd_type & RD_STDIN) ==  RD_STDIN)
       fclose(my_stdin);
    if((rd->rd_type & RD_STDOUT) ==  RD_STDOUT)
       fclose(my_stdout);
    if((rd->rd_type & RD_STDERR) ==  RD_STDERR)
       fclose(my_stderr);

    return 0; /* internal command executed sucessfull */
  }
  return 1; /* not an internal command */
}

int execute_command(int argc,char **argv,REDIRECT *rd)
{ 
   /* if cmd is not an internal cmd, execute it as external cmd */
   if(execute_internal_cmd(argv[0],argc,argv,rd) != 0)
    execute_external_cmd(argv[0],argv,rd);
}

/*----------------------------- Main Fucntion ------------------------------*/

int main(int argc,char *argv[],char **envp)
{
  int cmd_count,rd_cmds_count,args_count;
  int n,i,j,k,l,z;
  int nrdl,nrdg,irdi,irdo,irde,rdgf,lrdl,lrdg1,lrdg2;
  char *tmpstr,*s;
  char **commands,**rd_commands,**args_list;
  char **tmp_list1,**tmp_list2,**tmp_list3;
  REDIRECT rd;

  for(;;)
  {
    /* read user input */
    buf = readline(get_prompt());
    if (!buf) break;

    /* if input is not blank, add it to history list */
    s = trim(buf);
    if (*s)
      add_history (s);
    else {
       free (buf);
       continue;
    }
	
	#ifdef USE_EXPAND
	expand();
	printf("%s\n",ex_buf);
	#endif
	
    /* check for multiple commands. ';' is used as the delimiter */
    cmd_count = make_tokens(buf,";",&commands);

    for(i=0;i < cmd_count;i++)
    {
       /* initially assume no redirection is used */
	   rd.rd_type = 0;
       rd.in_file = rd.out_file = rd.err_file = NULL;

       /* check if redirection is used */
       if(  strchr(commands[i],'>') != NULL ||
	    strchr(commands[i],'<') != NULL  )
       {
  /*------------------------- start redirection -------------------------*/

  /* check for no. of '<' and '>'. there can be 2 '>' but only one '<' */
  nrdl = strchrcount(commands[i],'<');
  nrdg = strchrcount(commands[i],'>');

  lrdg1 = lrdg2 = lrdl = 999;
  irdi = irdo = irde = rdgf = 0;

  if(nrdl > 1 || nrdg > 2)
  {
    printf("Error : Invalid redirection command.\n");
    goto skip_redirection;
  }

  /* if '<' is present, i/p redirection is set and '>' sets o/p redirection */
  if(nrdl == 1)	rd.rd_type |= RD_STDIN;
  if(nrdg >= 1)	rd.rd_type |= RD_STDOUT;

  if(nrdl == 1)
  {
    tmpstr = strchr(commands[i],'<');
	/* get starting location of i/p redirection charater */
    lrdl = strlen(commands[i]) - strlen(tmpstr);
  }

  if(nrdg >= 1)
  {
    tmpstr = strchr(commands[i],'>');
	/* get starting location of o/p redirection charater */
    lrdg1 = strlen(commands[i]) - strlen(tmpstr);
  }

  /* check for 2nd '>' chacracter for stderr redirection */
  if(nrdg == 2)
  {
    tmpstr++;
    tmpstr = strchr(tmpstr,'>');
	/* get starting location of stderr redirection charater */
    lrdg2 = strlen(commands[i]) - strlen(tmpstr);
  }

  /* check if stderr redirection is uses using '2>' charaters */
  if(nrdg != 0)
  {
    k = lrdg1;
    if(k > 1 && commands[i][k-1] == '2')
    {
       commands[i][k-1] = ' ';
       rd.rd_type |= RD_STDERR;
       if(nrdg == 1)
       rd.rd_type &= ~RD_STDOUT;
       rdgf = 1;
    }

    k = lrdg2;
    if(k > 1 && commands[i][k-1] == '2')
    {
      commands[i][k-1] = ' ';
      if(rdgf)
      {
         /* if both redirection are stderr redirections, its invalid */
	     printf("Error : Invalid redirection command.\n");
	     goto skip_redirection;
      }
      rd.rd_type |= RD_STDERR;
    }

    /* make lrdg1 always point to o/p redirection character location */
    if(rdgf)
    {
      z = lrdg1;
      lrdg1 = lrdg2;
      lrdg2 = z;
    }
  }
  
  /* check is '<' or '>' is the first character itself, then its invalid. */
  rdgf = make_tokens(commands[i]," \n\t\r",&rd_commands);
  if(rd_commands[0][0] == '<' || rd_commands[0][0] == '>')
  {
    printf("Error : Invalid redirection command.\n");
    free_make_tokens(rd_commands);
    goto skip_redirection;
  }
  free_make_tokens(rd_commands);

  /* get file name index in the command for respective redirections */
  irdi = get3sindex(lrdl,lrdg1,lrdg2,1,rd.rd_type);
  irdo = get3sindex(lrdl,lrdg1,lrdg2,2,rd.rd_type);
  irde = get3sindex(lrdl,lrdg1,lrdg2,3,rd.rd_type);

  /* printf("irdi = %d\nirdo = %d\nirde = %d",irdi,irdo,irde); */

  /* separate the command into exe part and redirection file names part */
  rd_cmds_count = make_tokens(commands[i],"<>",&rd_commands);

  if( rd_cmds_count > 4 ||
      irdi >= rd_cmds_count ||
      irdo >= rd_cmds_count ||
      irde >= rd_cmds_count  )
      {
	   printf("Error : Invalid redirection command.\n");
	   free_make_tokens(rd_commands);
	   goto skip_redirection;
      }

  /* extract and assign file used for coressponding redirections */
  if((rd.rd_type & RD_STDIN) == RD_STDIN)
  {
    make_tokens(rd_commands[irdi]," \n\r\t",&tmp_list1);
    rd.in_file = tmp_list1[0];
    /* printf("infile = %s\n",tmp_list1[0]); */
  }

  if((rd.rd_type & RD_STDOUT) == RD_STDOUT)
  {
    make_tokens(rd_commands[irdo]," \n\r\t",&tmp_list2);
    rd.out_file = tmp_list2[0];
    /* printf("outfile = %s\n",tmp_list2[0]); */
  }

  if((rd.rd_type & RD_STDERR) == RD_STDERR)
  {
    make_tokens(rd_commands[irde]," \n\r\t",&tmp_list3);
    rd.err_file = tmp_list3[0];
    /* printf("errfile = %s\n",tmp_list3[0]); */
  }

  args_count = make_tokens(rd_commands[0]," \n\r\t",&args_list);
  execute_command(args_count,args_list,&rd);
  free_make_tokens(args_list);

  /* free corresponding memory allocated to redirection command tokens */
  if((rd.rd_type & RD_STDIN) == RD_STDIN)
    free_make_tokens(tmp_list1);
  if((rd.rd_type & RD_STDOUT) == RD_STDOUT)
    free_make_tokens(tmp_list2);
  if((rd.rd_type & RD_STDERR) == RD_STDERR)
    free_make_tokens(tmp_list3);

  /* finally free memory allocated to redirection command tokens */
  free_make_tokens(rd_commands);

  /* jump here if invalid redirection command is issued */
  skip_redirection:
  z = 0; /* dummy code to avoid error */

  /*-------------------------- end redirection -------------------------*/
 	  }
	  else /* comes here if no redirection is used */
	  {
	    args_count = make_tokens(commands[i]," \n\r\t",&args_list);
	    if(args_list[0] != NULL)
	    execute_command(args_count,args_list,&rd);
	    free_make_tokens(args_list);
	  }
    } /* multiple commands for loop end */
    
	/* free memory allocated to multiple commands buffer*/
	free_make_tokens(commands);
	
	/* free memory allocated to user input */
    free(buf);
	
    } /* infinite for loop end */
	
} /* main end */

/*----------------------------- End Of File --------------------------------*/
