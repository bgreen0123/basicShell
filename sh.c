#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd, *cwd;
  char *buffer = calloc(MAX, sizeof(char));
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1, pos = 0;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/
     
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();


  while ( go )
  {
    cwd = getcwd(NULL, PATH_MAX+1);

    /* print your prompt */
    printf("%s%s > ",prompt,cwd);
    fgets(buffer, MAX, stdin);
    
    /* get command line and process */
    arg = strtok(buffer,TOK_DELIM);
    while(arg!=NULL){
      args[pos] = arg;
      pos++;

      arg = strtok(NULL, TOK_DELIM);
    }
  
  /* check for each built in command and implement */
    if(strcmp(args[0],"exit")==0){
      exitShell();
    }
      
    else if(strcmp(args[0],"which")==0){
      if(args[1]!=NULL){
        which(args[1],pathlist);
      }

      else{
        printf("Which: Too few arguments\n");
      }
 
    }

    else if(strcmp(args[0],"where")==0){
      if(args[1]!=NULL){
        where(args[1],pathlist);
      }

      else{
	printf("Where: Too few arguments\n");
    
      }
    
    }

    else if(strcmp(args[0],"cd")==0){
      owd = cwd;
      if(args[1]!=NULL){
	if(strcmp(args[1],"-")==0){
          cd(owd);
        }

        cd(args[1]);
      }

      else{
	 cd(pwd);
      }
    }
    pos = 0; 
  /*  else  program to exec */
    {
       /* find it */
       /* do fork(), execve() and waitpid() */

      /* else */
        /* fprintf(stderr, "%s: Command not found.\n", args[0]); */
    }
  }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
  /* loop through pathlist until finding command and return it.  Return
  NULL when not found. */
 
  while (pathlist) {         // WHICH
    sprintf(command, "%s", pathlist->element);
    if (access(command, X_OK) == 0) {
      printf("[%s]\n", command);
      break;
    }

    pathlist = pathlist->next;
  }
  return NULL;

} /* which() */

char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */

  while(pathlist){
    sprintf(command, "%s", pathlist->element);
      if(access(command, F_OK)==0){
              printf("[%s]\n", command);
                break;
	    }
	    pathlist = pathlist->next;
	}
	return NULL;
} /* where() */

void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */

void cd(char *path){
  /* Change directory to the path that the user specifies*/
  if(chdir(path)!=0){
    printf("No such file or directory\n");
  

  }
}

void exitShell(){
	exit(0);
}

