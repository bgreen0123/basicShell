
#include "get_path.h"

int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
void *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void cd(char **args, char *owd, char *cwd);
void exitShell(char *buffer, char *prompt, char **args);
void printenv(char **args, char **environ);
void setenviron(char **args, char **envrion, struct pathelement *pathlist);

#define PROMPTMAX 32
#define MAXARGS 10
#define MAX 258
#define TOK_DELIM " \n"
