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

extern char **environ;

//Side effect of making a buffer, must free it.
int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandpath = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *p, *pwd, *owd, *cwd;
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
      printf("Execting built-in [%s]\n",args[0]);
      exitShell(buffer, prompt, args);
    }
      
    else if(strcmp(args[0],"which")==0){
      if(args[1]!=NULL){
	printf("Execting built-in [%s]\n",args[0]);
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
	printf("Execting built-in [%s]\n",args[0]);
        where(args[1],pathlist);
      }

      else{
	printf("Where: Too few arguments\n");
    
      }
    }

    else if(strcmp(args[0],"cd")==0){
      printf("Execting built-in [%s]\n",args[0]);
      cd(args, owd, cwd);
    }

    else if(strcmp(args[0],"pwd")==0){
      printf("Execting built-in [%s]\n",args[0]);
      printf("%s\n",getcwd(NULL, PATH_MAX + 1));
    }

    else if(strcmp(args[0],"pid") == 0){
      printf("Execting built-in [%s]\n",args[0]);
      printf("%d\n",getpid());
    }

    else if(strcmp(args[0],"list")==0){
      printf("Executing built-in [%s]\n",args[0]);
      list(cwd);
    } 


    /*
     * last remaining commands to support
     */

    else if(strcmp(args[0], "printenv")==0){
	    printf("Executing built-in [%s]\n", args[0]);
    }

    else if(strcmp(args[0], "setenv")==0){
	    printf("Executing built-in [%s]\n", args[0]);
    }

    else if(strcmp(args[0], "prompt")==0){
	    printf("Executing built-in [%s]\n", args[0]);
    }

    else if(strcmp(args[0], "kill")==0){
	    printf("Executing built-in [%s]\n", args[0]);
    }
 
  /*  else  program to exec */
    else{
       /* find it */
       strcpy(commandpath,which(args[0],pathlist));
       if(strcmp(commandpath,"1") != 0){

       /* do fork(), execve() and waitpid() */

         if((pid = fork()) < 0){
           perror("fork");
	   exit(2);
	 }

	 else if(pid == 0){

           printf("Executing [%s]\n",args[0]);

           if(execve(commandpath, args, environ) < 0){
             perror("Execve error");
	     exit(2);
	   }
	 }

	 else if(waitpid(pid, &status, 0) == -1){
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
  char *exit = "1";
  char * tmp = malloc(strlen(pathlist->element)+1+strlen(command)+1);
  while (pathlist) {         // WHICH
    sprintf(tmp, "%s/%s", pathlist->element, command);
    if (access(tmp, X_OK) == 0) {
      return tmp; 
    }
    pathlist = pathlist->next;
  }
  free(tmp);
  return exit;

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

void list ( char *dir ){

  DIR *files;
  struct dirent *entry;

  if((files = opendir(dir)) == NULL){
    perror("list Error");
  }
  else{
    while((entry = readdir(files)) != NULL){
      printf("%s\n",entry->d_name);
    }
    closedir(files);
  }
} /* list() */

void cd(char **args, char *owd, char *cwd){
  /* Change directory to the path that the user specifies*/

  char *tmp;
  tmp = calloc(strlen(owd) + 1, sizeof(char));
  memcpy(tmp, owd, strlen(owd)); //copies n bytes from owd into tmp

  if(args[1] == NULL){ // no argument, goes to home directory 
	
	  printf("testing \n");

	  printf("Home: %s\n", getenv("HOME"));
	  chdir(getenv("HOME"));
	  strcpy(cwd, getcwd(NULL, 0)); //updating cwd

  } else if(strcmp(args[1], "-") == 0){ // go to previous directory


	/* copy owd into tmp, so tmp now = old working directory
	 * copy cwd into owd, owd = cwd, wanna make sure you change owd before calling, so now we can go back if called again with - argument
	 * change directory with tmp
	 * update cwd
	 */
	
	  strcpy(tmp, owd);
	  strcpy(owd, getcwd(NULL, 0));
	  chdir(tmp);
	  strcpy(cwd, getcwd(NULL, 0));

  } else { // standard cd functionality
	
	/* owd now contains current cwd
	 * call change directory with user argument
	 * update cwd to new directory you changed to
	 */

	  strcpy(owd, getcwd(NULL, 0));
	  chdir(args[1]);
	  strcpy(cwd, getcwd(NULL, 0));
  }

}

void exitShell(char *buffer, char *prompt, char **args){
  free(buffer);
  free(prompt);
  free(args);
  exit(0);
}

