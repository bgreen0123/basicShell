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

extern char *environ;

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd, *cwd;
  char *buffer = calloc(MAX, sizeof(char));
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, pid, status, argsct, go = 1, pos = 0;
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
  owd = getcwd(NULL, PATH_MAX + 1);
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
        char *path = which(args[1],pathlist);
	if(path){
	  printf("%s\n",path);
	  free(path);
	}
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

      if(args[1]!=NULL){
	//NEED TO FIX
	if(strcmp(args[1],"-") == 0){
          chdir(owd);
	  owd = getcwd(NULL, PATH_MAX + 1);
        }

	else if(access(args[1],X_OK)==0){
          owd = getcwd(NULL, PATH_MAX + 1);
	  chdir(args[1]);
	}
      }

      else if(args[1] == NULL){
        owd = getcwd(NULL, PATH_MAX + 1);
	chdir(homedir);
      }

      else{
        printf("You do not have authorization");
      }
    }

    else if(strcmp(args[0],"pwd")==0){
      printf("%s\n",getcwd(NULL, PATH_MAX + 1));
    }

    else if(strcmp(args[0],"pid") == 0){
      printf("%d\n",getpid());
    }
 
  /*  else  program to exec */
    else{
       /* find it */

       if(which(args[0], pathlist) != NULL){

       /* do fork(), execve() and waitpid() */

         if((pid = fork()) < 0){
           perror("fork");
	   exit(2);
	 }

	 else if(pid == 0){

           printf("Executing built-in [%s]\n",args[0]);

           if(execve(args[0],args, NULL) < 0){
             perror("Execve error");
	     exit(2);
	   }
	 }

	 else if(waitpid(pid, &status, 0) != 0){
	     perror("Wait eorror");
             exit(WEXITSTATUS(status));
           }
         }	 

       else{
         fprintf(stderr, "%s: Command not found.\n", args[0]);
       }
    }

    pos = 0;
  }
  return 0;
} /* sh() */

//Side effect: Mallocs memory so you must free it after running the command
char *which(char *command, struct pathelement *pathlist )
{
  /* loop through pathlist until finding command and return it.  Return
  NULL when not found. */
  
  char * tmp = malloc(strlen(pathlist->element)+1+strlen(command)+1);
  while (pathlist) {         // WHICH
    sprintf(tmp, "%s/%s", pathlist->element, command);
    if (access(tmp, X_OK) == 0) {
      return tmp; 
    }
    pathlist = pathlist->next;
  }
  return NULL;

} /* which() */

void *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */

  char *tmp = malloc(strlen(pathlist->element)+1+strlen(command)+1);
  while(pathlist){
    sprintf(tmp, "%s/%s", pathlist->element,command);
      if(access(tmp, F_OK) == 0){
                printf("%s\n",tmp);
	    }
	    pathlist = pathlist->next;
	}
  free(tmp);
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

