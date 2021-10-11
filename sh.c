//Brendan Green and Ryan Allarey

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

/*
 *Name: This function is named sh because it is the logic behine the bulk of our shell.
 *Function: sh is responsible for taking user input, cutting it into tokens and then calling the correct command.
 *Param: sh takes in an integer, an array of strings, and the environment.
 *Return: Returns an interger of 0 upon success.
 *Side affects: This function mallocs memory for multiple varibales that must be freed. Such as prompt, commandpath, args, buffer, owd, cwd, and pwd.
 */ 
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
    printf("%s [%s] > ",prompt,cwd);
    fgets(buffer, MAX, stdin);
    	/* get command line and process */
    	arg = strtok(buffer,TOK_DELIM);
    	while(arg!=NULL){
      	  args[pos] = arg;
      	  pos++;
      	  arg = strtok(NULL, TOK_DELIM);
    	}
    	args[pos++] = NULL;
  
  /* check for each built in command and implement */
    
    if(strcmp(args[0],"exit")==0){
      printf("Executing built-in [%s]\n",args[0]);
      exitShell(buffer, prompt, pathlist, pwd, owd, cwd);
    }
      
    else if(strcmp(args[0],"which")==0){
      if(args[1]!=NULL){
	printf("Executing built-in [%s]\n",args[0]);
        char *path = which(args[1],pathlist);
	if(path){
	  printf("%s\n",path);
	  free(path);
	}
	else{
	  free(path);
	}
      }

      else{
        printf("Which: Too few arguments\n");
      }
 
    }

    else if(strcmp(args[0],"where")==0){
      if(args[1]!=NULL){
	printf("Executing built-in [%s]\n",args[0]);
        where(args[1],pathlist);
      }

      else{
	printf("Where: Too few arguments\n");
    
      }
    }

    else if(strcmp(args[0],"cd")==0){
      printf("Executing built-in [%s]\n",args[0]);
      cd(args, owd, cwd);
    }

    else if(strcmp(args[0],"pwd")==0){
      printf("Executing built-in [%s]\n",args[0]);
      printf("%s\n",getcwd(NULL, PATH_MAX + 1));
    }

    else if(strcmp(args[0],"pid") == 0){
      printf("Executing built-in [%s]\n",args[0]);
      printf("%d\n",getpid());
    }

    else if(strcmp(args[0],"list")==0){
      printf("Executing built-in [%s]\n",args[0]);
      list(cwd);
    }

    else if(strcmp(args[0], "printenv")==0){
        printf("Executing built-in [%s]\n", args[0]);
	printenv(args,environ);
    }

    else if(strcmp(args[0], "setenv")==0){
	    printf("Executing built-in [%s]\n", args[0]);
	    setenviron(args,environ,pathlist);
    }

    else if(strcmp(args[0], "prompt")==0){
	    printf("Executing built-in [%s]\n", args[0]);
	    promptUser(args,prompt);
    }

    else if(strcmp(args[0], "kill")==0){
	    if (args[1] != NULL){
		printf("Executing built-in [%s]\n", args[0]);
	   	killProcess(args);
	    } else {
	        printf("Kill: Too few arguments\n");
	    }
    }
 
  /*  else  program to exec */
    else{
       /* find it */
       strcpy(commandpath,which(args[0],pathlist));
       if(strcmp(commandpath,"1") != 0){

       /* do fork(), execve() and waitpid() */

         if((pid = fork()) < 0){
           perror("fork");
	   free(commandpath);
	   exit(2);
	 }

	 else if(pid == 0){

           printf("Executing [%s]\n",args[0]);

           if(execve(commandpath, args, environ) < 0){
             perror("Execve error");
	     free(commandpath);
	     exit(2);
	   }
	 }

	 else if(waitpid(pid, &status, 0) == -1){
	     perror("Wait eorror");
	     free(commandpath);
             exit(WEXITSTATUS(status));
           }
         }	 

       else{
         fprintf(stderr, "%s: Command not found.\n", args[0]);
       }

       free(commandpath);
    }

    pos = 0;
  }
  free(cwd);
  for(int i = 0; i<sizeof args; i++){
          free(args[1]);
  }
  return 0;
} /* sh() */

//Side effect: Mallocs memory so you must free it after running the command
/*
 * Name: This function is named which. It is named this because it looks through the pathlist and finds the command asked for.
 * Function: which will loop through pathlist until finding the command it is looking for then returns the path to it.
 * Param: Which takes in a string holding the command the user wants to find, and a pathelement struct holding the pathlist to all files.
 * Return: Which returns the path to the command.
 * Side Affects: Which allocates memory so we must free it after which is run.
 */
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

/*
 * Name: This function is called where because it finds every path to where that command exisits.
 * Function: Where will loop through the pathlist and find every path to the wanted command.
 * Param: Where takes in a string holding the command the user want to find, and a pathelement struct holding the pathlist to all files.
 * Returns: Where returns nothing.
 * Side Affects: Where doesn't change anything outside of itself.
 */
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

/*
 * Name: this function is called list and is called when the user types the list command into our shell.
 * Function: list will list the files in the cwd, one per line.
 * Param: dir, the directory name
 * Side effects: doesn't change anything outside of the function
 * Returns: List returns nothing
 */

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

/*
 * Name: this is the cd function for when the user types the cd command into our shell. cd means change directory.
 * Function: this function is the logic for changing the directory that the user is in.
 * Param: args, the argument list for the cd command. owd, old working directory. cwd, current working directory.
 * owd and cwd allow us to change and keep track of where the user is to be able to change the directory as necessary
 * Side effects: changes current working directory of the user and old working directory
 * Returns: void function, no returns
 */

void cd(char **args, char *owd, char *cwd){
  /* Change directory to the path that the user specifies*/

  char *tmp;
  tmp = calloc(strlen(owd) + 1, sizeof(char));
  memcpy(tmp, owd, strlen(owd)); //copies n bytes from owd into tmp

  if(args[1] == NULL){ // no argument, goes to home directory 
	
	  chdir(getenv("HOME"));
	  getcwd(cwd, sizeof cwd); //updating cwd

  } else if(strcmp(args[1], "-") == 0){ // go to previous directory


	/* copy owd into tmp, so tmp now = old working directory
	 * copy cwd into owd, owd = cwd, wanna make sure you change owd before calling, so now we can go back if called again with - argument
	 * change directory with tmp
	 * update cwd
	 */
	
	  strcpy(tmp, owd);
	  getcwd(owd, sizeof owd);
	  chdir(tmp);
	  getcwd(cwd,sizeof cwd);

  } else if((args[1] != NULL) && (access(args[1], X_OK) == 0)) { // standard cd functionality
	
	/* owd now contains current cwd
	 * call change directory with user argument
	 * update cwd to new directory you changed to
	 */

	  getcwd(owd, sizeof owd);
	  chdir(args[1]);
	  getcwd(cwd, sizeof cwd);
  } else{
	  printf("No directory called: %s\n",args[1]);
  }
  free(tmp);
  free(cwd);
}
/*
 * Name: this function is called and prints the environment when the printenv command is typed into our shell.
 * Function: this function prints the environment or prints a specific environment variable that the user wants.
 * Param: args, the argument list for the printenv command. environ, the state of the environment.y
 * Returns: Printenv returns nothing.
 * Side Effects: Printenv doesn't change anything outside of itself.
 */
void printenv(char **args, char **environ){
    int i = 0;
    char *env;
    if(args[1] == NULL){
	while(environ[i] != NULL){
	    printf("%s\n",environ[i]);
	    i++;
	}
    }
    else if((args[1] != NULL) && (args[2] == NULL)){
        env = getenv(args[1]);
	if(env != NULL){
	    printf("%s\n",env);
	}
    }
    else{
	printf("%s: Too many arguments.\n",args[0]);
    }
}
/*
 * Name: this function is called and sets the environment when the setenv command is typed into our shell.
 * Function: this will allow the user to set the environment or change any environment variable
 * Param: args, the argument list for the setenv command. environ, the state of the environment. pathlist, used 
 * to create a new linked list when PATH is changed. this allows us to change the PATH and free the old one.
 * Return: Setenviron returns nothing.
 * Side Effects: Setenviron changes HOME and PATH to user set input or can make a new variable set to user input.
 */
void setenviron(char **args, char **environ, struct pathelement *pathlist){
    struct pathelement *newPath, *tmp;

    if(args[1] == NULL){
	printenv(args,environ);
    }
    else if(args[1] != NULL && args[2] == NULL){
	setenv(args[1],"",1);
    }
    else if(args[2] != NULL && args[3] == NULL){
	if(strcmp(args[1],"PATH") == 0){//check is user is changing PATH
            setenv(args[1], args[2], 1);//change the environment variable
            newPath = get_path();//get the new path
	    while(pathlist!=NULL){//Loop until end of list, freeing each node in the list
		tmp = pathlist;
		pathlist = pathlist->next;
		free(tmp);
	    }
	    pathlist = newPath;//set the head of newPath to pathlist
	    free(newPath);
	}
	else if(strcmp(args[1], "HOME") == 0){//check if the user us changing HOME
	    setenv(args[1],args[2],1);//change the env variable to be what the user inputted
	}
        else{
	   setenv(args[1],args[2],1);
	}	   
    }
    else{
	printf("%s: Too many arguments.\n",args[0]);
    }
}

/*
 * Name: this function is called when the kill command is typed into our shell.
 * Function: this function will kill the process that the user wants and sent a SIGTERM to it.
 * Param: args is an array that keeps track of the arguments that the user types. This will be taken is as a pid for the kill function.
 * Side Effects: doesn't change anything outside of the kill function, but kills the process.
 * Returns: none
 */

void killProcess(char **args){
	if(args[2] == NULL){ // given just a pid, sends a sigterm to it
		kill(getpid(), SIGTERM);
	} else if (args[2] != NULL){
		int signal = atoi(args[1]); // converts to integer
		signal = signal*(-1); // signal number with a - in front of it
		kill(getpid(), signal); 
	}
}

/*
 * Name: This function is called promptUser because the user can type in a prompt that will appear before the path.
 * Function: PromptUser takes in user input then puts that string in front of the path.
 * Param: Prompt takes in the array of arguments and the old prompt string.
 * Side Effects: PromptUser doesn't change anything outside of itself.
 * Returns: PromptUser returns nothing.
 */
void promptUser(char **args, char *prompt){

	if(args[1] == NULL){
		printf("input prompt prefix: ");
		if(fgets(prompt,MAX,stdin) != NULL){
			prompt[(int) (strlen(prompt))-1] = '\0';
		}
	}
	else if(args[1] != NULL && args[2] == NULL){
		strcpy(prompt,args[1]);
	}
	else{
		printf("%s: Too many arguments.\n",args[0]);
	}
}

/*
 * Name: this function is called when the command exit is typed.
 * Function: this frees memory and is responsible for exiting the shell.
 * Param: buffer, prompt, args all need to be freed at the end of our program.
 * Side Effects: doesn't change anything, but we need to free memory of everything we allocated memory for.
 * Returns: void, no returns
 */

void exitShell(char *buffer, char *prompt, struct pathelement *pathlist, char *pwd, char *owd, char *cwd){
  struct pathelement *tmp;
  free(buffer);
  free(prompt);
  free(pwd);
  free(owd);
  free(cwd);
  while(pathlist != NULL){
	  tmp = pathlist->next;
	  free(pathlist);
	  pathlist = tmp;
  }
  exit(0);
}

