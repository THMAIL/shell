// COMP1521 18s2 mysh ... command history
// Implements an abstract data object

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

// This is defined in string.h
// BUT ONLY if you use -std=gnu99
//extern char *strdup(const char *s);

// Command History
// array of command lines
// each is associated with a sequence number

#define MAXHIST  20
#define MAXSTR   200

#define HISTFILE ".mymysh_history"

typedef struct _history_entry {
   int   seqNumber;
   char commandLine[200];
} HistoryEntry;

typedef struct _history_list {
   int nEntries;
   HistoryEntry commands[MAXHIST];
} HistoryList;

HistoryList CommandHistory;
int historyListPointer = 0;

// initCommandHistory()
// - initialise the data structure
// - read from .history if it exists

int initCommandHistory()
{
    // TODO
    //restore historical commands to array from text
    FILE *fp;
    if((fp = fopen(HISTFILE,"r")) == NULL)
    { 
        //printf("Open Falied!"); 
        return 0; 
    } 
    HistoryEntry temp;
    while(!feof(fp)) {
        fscanf(fp," %3d",&temp.seqNumber);
        fscanf(fp,"%[^\n]%*c",temp.commandLine);
        CommandHistory.commands[CommandHistory.nEntries++] = temp;
    }
    CommandHistory.nEntries--;
    fclose(fp);
    return 1;
}

// addToCommandHistory()
// - add a command line to the history list
// - overwrite oldest entry if buffer is full

void addToCommandHistory(char *cmdLine, int seqNo)
{
   // TODO
   HistoryEntry temp;
   if(CommandHistory.nEntries < 20){
       temp.seqNumber = CommandHistory.nEntries;
       strcpy(temp.commandLine, cmdLine);
       CommandHistory.commands[CommandHistory.nEntries++] = temp;
       return;
   }
   if(historyListPointer < 20){
        temp.seqNumber = historyListPointer;
        strcpy(temp.commandLine, cmdLine);
        CommandHistory.commands[historyListPointer++] = temp;
   }else{
        historyListPointer = 0;
        temp.seqNumber = historyListPointer;
        strcpy(temp.commandLine, cmdLine);
        CommandHistory.commands[historyListPointer++] = temp;
   }
   
}

// showCommandHistory()
// - display the list of 

void showCommandHistory(FILE *histFile)
{
    for(int i = 0; i < CommandHistory.nEntries; i++){
        printf(" %3d  %s\n",CommandHistory.commands[i].seqNumber,CommandHistory.commands[i].commandLine);
    }
}

// getCommandFromHistory()
// - get the command line for specified command
// - returns NULL if no command with this number

char *getCommandFromHistory(int cmdNo)
{
   // TODO
   if(0 <= cmdNo && cmdNo < CommandHistory.nEntries){
       return CommandHistory.commands[cmdNo].commandLine;
   }else{
       return("No such command");
   }
}

// saveCommandHistory()
// - write history to $HOME/.mymysh_history

void saveCommandHistory()
{
   // TODO
   FILE *fp = fopen(HISTFILE, "w+");
   for(int i = 0; i < CommandHistory.nEntries; i++){
       fprintf(fp, " %3d  %s\n", CommandHistory.commands[i].seqNumber,CommandHistory.commands[i].commandLine);
   }
   fclose(fp);
}

// cleanCommandHistory
// - release all data allocated to command history

void cleanCommandHistory()
{
   // TODO
   remove(HISTFILE);
}
