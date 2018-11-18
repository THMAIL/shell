// mysh.c ... a small shell
// Started by John Shepherd, September 2018
// Completed by <<YOU>>, September/October 2018

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glob.h>
#include <assert.h>
#include <fcntl.h>
#include "history.h"

// This is defined in string.h
// BUT ONLY if you use -std=gnu99
//extern char *strdup(char *);

// Function forward references

void trim(char *);
int strContains(char *, char *);
char **tokenise(char *, char *);
void fileNameExpand(char **tokens, char fileName[][200]);
void freeTokens(char **);
char *findExecutable(char *, char **);
int isExecutable(char *);
void prompt(void);
int cont_str(char *s);


// Global Constants

#define MAXLINE 200
#define historyFile ".mymysh_history"

// Global Data
char lastCommand[MAXLINE];

/* none ... unless you want some */


// Main program
// Set up enviroment and then run main loop
// - read command, execute command, repeat

int main(int argc, char *argv[], char *envp[])
{
   //pid_t pid;   // pid of child process
   //int stat;    // return status of child
   char **path; // array of directory names
   int cmdNo;   // command number
   int i;       // generic index

   // set up command PATH from environment variable
   for (i = 0; envp[i] != NULL; i++) {
      if (strncmp(envp[i], "PATH=", 5) == 0) break;
   }
   if (envp[i] == NULL)
      path = tokenise("/bin:/usr/bin",":");
   else
      // &envp[i][5] skips over "PATH=" prefix
      path = tokenise(&envp[i][5],":");
#ifdef DBUG
   for (i = 0; path[i] != NULL;i++)
      printf("path[%d] = %s\n",i,path[i]);
#endif
    path = path;

   // initialise command history
   // - use content of ~/.mymysh_history file if it exists

   cmdNo = initCommandHistory();
   cmdNo++;

   // main loop: print prompt, read line, execute command

   char line[MAXLINE];
   prompt();
   while (fgets(line, MAXLINE, stdin) != NULL) {
      trim(line); // remove leading/trailing space
      if(!cont_str(line)){//Input is empty
          printf("input is empty!\n");
          prompt();
          continue;
      }
      //Handling historical input statements
      if(line[0] == '!'){
          if(line[1] != '\0'){
                  if(line[2] != '\0'){
                      int a = (line[1] - '0');
                      a = a * 10;
                      int b = line[2] - '0';
                      strcpy(line, getCommandFromHistory(a+b));
                      if(strcmp(line, "No such command")){
                          puts(line);
                      }
                  }else if(line[1] == '!'){
                      strcpy(line, lastCommand);
                      puts(line);
                  }else{
                      int a = line[1] - '0';//Converting character data into integers
                      strcpy(line, getCommandFromHistory(a));
                      if(strcmp(line, "No such command")){
                          puts(line);
                      }
                  }
          }
      }
      
      //semantic analysis
      char **splitedLine = tokenise(line, " ");
      //Processing file name wildcards
      char fileNameArray[10][200] = {0};
      //fileNameExpand(splitedLine, fileNameArray);
      //Handling built in shell
      if(!strcmp(splitedLine[0], "exit")){//exit
          saveCommandHistory();
          return(EXIT_SUCCESS);
      }
      else if(!strcmp(splitedLine[0], "h") || !strcmp(splitedLine[0], "history")){//Output history command
          FILE *histFile = fopen(historyFile, "r");
          showCommandHistory(histFile);
          strcpy(lastCommand, line);
          addToCommandHistory(line, MAXLINE);
          prompt();
          continue;
      }
      else if(!strcmp(splitedLine[0], "pwd")){//Output current working directory
          char path[MAXLINE];
          getcwd(path, MAXLINE);
          puts(path);
          strcpy(lastCommand, line);
          addToCommandHistory(line, MAXLINE);
          prompt();
          continue;
      }
      else if(!strcmp(splitedLine[0], "cd")){//Switch working path
          chdir(splitedLine[1]);
          char path[MAXLINE];
          getcwd(path, MAXLINE);
          puts(path);
          strcpy(lastCommand, line);
          addToCommandHistory(line, MAXLINE);
          prompt();
          continue;
      }
      //Check input redirection
      i = 0;
      while( (splitedLine[i] != NULL) && strcmp(splitedLine[i], "<")) 
          i++;
      if(splitedLine[i] != NULL){
          if(!strcmp(splitedLine[i], "<")){
              if(splitedLine[++i] != NULL){
                  if(splitedLine[++i] != NULL){
                       puts("Wrong Input!");
                       prompt();
                       continue;
                  }else{//Correct situation
                      // puts("Correct Input!");
                  }
              }else{
                  puts("Wrong Input!");
                  prompt();
                  continue;
              }
          }
      }else{
          //Check output redirection
          i = 0;
          while( (splitedLine[i] != NULL) && strcmp(splitedLine[i], ">")) 
              i++;
          if(splitedLine[i] != NULL){
              if(!strcmp(splitedLine[i], ">")){
                  if(splitedLine[++i] != NULL){
                      if(splitedLine[++i] != NULL){
                           puts("Wrong Input!");
                           prompt();
                           continue;
                      }else{//Correct situation
                           //puts("Correct Input!");
                      }
                  }else{
                      puts("Wrong Input!");
                      prompt();
                      continue;
                  }
              }
          }
      }
      
      //Use the first word to find executable files, 
      //and if not found, there is no such command.
      char *binPath[] = {"/bin", "/usr/bin", NULL};
      char *executable = findExecutable(splitedLine[0], binPath);
      if(executable == NULL){
         puts("No command specified!");
         prompt();
         continue;
      }
      //Add history command
      strcpy(lastCommand, line);
      addToCommandHistory(line, MAXLINE);
      //run command
      //char *argv[10];
      i = 0;
      while(splitedLine[i] != NULL){
          argv[i] = splitedLine[i];
          i++;
      }
      argv[i] = NULL;
      i--;
      //char *envp[]={0,NULL}; //An array of new environment variables passed to the execution file
      if(!strcmp(fileNameArray[0], "\0")){
          printf("Running %s ... \n--------------------\n", executable);
          printf("--------------------\nReturn %d\n", system(line));
          prompt();
          continue;
      }
      for(int j = 0;strcmp(fileNameArray[j],"\0"); j++){
          argv[i] = fileNameArray[j];
          printf("Running %s ... \n--------------------\n", executable);
          printf("--------------------\nReturn %d\n", system(line));
      }

      prompt();
   }
   saveCommandHistory();
   printf("\n");
   return(EXIT_SUCCESS);
}

//Determines whether the string is empty
int cont_str(char *s)
{
    int i = 0;
    while ( s[i++] != '\0');
    if(i <= 1){
        return 0;
    }
    else{
        return 1;
    }
}

// fileNameExpand: expand any wildcards in command-line args
// - returns a possibly larger set of tokens
void fileNameExpand(char **tokens, char fileName[][200])
{
   // TODO
   int i = 0, j = 0;
   glob_t buf;
   while(! (tokens[i++] == NULL));
   glob(tokens[--i], GLOB_NOCHECK|GLOB_TILDE, NULL, &buf);
   for(j=0; j < buf.gl_pathc; j++){
       strcpy(fileName[j], buf.gl_pathv[j]);
       printf("PATH:%s\n", buf.gl_pathv[j]);
   }
}

// findExecutable: look for executable in PATH
char *findExecutable(char *cmd, char **path)
{
      char executable[MAXLINE];
      executable[0] = '\0';
      if (cmd[0] == '/' || cmd[0] == '.') {
         strcpy(executable, cmd);
         if (!isExecutable(executable))
            executable[0] = '\0';
      }
      else {
         int i;
         for (i = 0; path[i] != NULL; i++) {
            sprintf(executable, "%s/%s", path[i], cmd);
            if (isExecutable(executable)) break;
         }
         if (path[i] == NULL) executable[0] = '\0';
      }
      if (executable[0] == '\0')
         return NULL;
      else
         return strdup(executable);
}

// isExecutable: check whether this process can execute a file
int isExecutable(char *cmd)
{
   struct stat s;
   // must be accessible
   if (stat(cmd, &s) < 0)
      return 0;
   // must be a regular file
   //if (!(s.st_mode & S_IFREG))
   if (!S_ISREG(s.st_mode))
      return 0;
   // if it's owner executable by us, ok
   if (s.st_uid == getuid() && s.st_mode & S_IXUSR)
      return 1;
   // if it's group executable by us, ok
   if (s.st_gid == getgid() && s.st_mode & S_IXGRP)
      return 1;
   // if it's other executable by us, ok
   if (s.st_mode & S_IXOTH)
      return 1;
   return 0;
}

// tokenise: split a string around a set of separators
// create an array of separate strings
// final array element contains NULL
char **tokenise(char *str, char *sep)
{
   // temp copy of string, because strtok() mangles it
   char *tmp;
   // count tokens
   tmp = strdup(str);
   int n = 0;
   strtok(tmp, sep); n++;
   while (strtok(NULL, sep) != NULL) n++;
   free(tmp);
   // allocate array for argv strings
   char **strings = malloc((n+1)*sizeof(char *));
   assert(strings != NULL);
   // now tokenise and fill array
   tmp = strdup(str);
   char *next; int i = 0;
   next = strtok(tmp, sep);
   strings[i++] = strdup(next);
   while ((next = strtok(NULL,sep)) != NULL)
      strings[i++] = strdup(next);
   strings[i] = NULL;
   free(tmp);
   return strings;
}

// freeTokens: free memory associated with array of tokens
void freeTokens(char **toks)
{
   for (int i = 0; toks[i] != NULL; i++)
      free(toks[i]);
   free(toks);
}

// trim: remove leading/trailing spaces from a string
void trim(char *str)
{
   int first, last;
   first = 0;
   while (isspace(str[first])) first++;
   last  = strlen(str)-1;
   while (isspace(str[last])) last--;
   int i, j = 0;
   for (i = first; i <= last; i++) str[j++] = str[i];
   str[j] = '\0';
}

// strContains: does the first string contain any char from 2nd string?
int strContains(char *str, char *chars)
{
   for (char *s = str; *s != '\0'; s++) {
      for (char *c = chars; *c != '\0'; c++) {
         if (*s == *c) return 1;
      }
   }
   return 0;
}

// prompt: print a shell prompt
// done as a function to allow switching to $PS1
void prompt(void)
{
   printf("mymysh$ ");
}
