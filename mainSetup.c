#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
 
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
 
//		PATH NAME STRUCTS
// 
// Node struct
struct pathnameList{
	char pathname[128];
	struct pathnameList *next;
};
// Create node
struct pathnameList* createPathname(char *pathname)
{
	struct pathnameList *newNode = (struct pathnameList*)malloc(sizeof(struct pathnameList));
	strcpy(newNode->pathname,pathname);
	newNode->next = NULL;
	return newNode;
}
// Insert node
struct pathnameList* insertPathname(struct pathnameList *header,char *pathname)
{
	struct pathnameList *temp;
	if(header == NULL)	// If the list is empty
	{
		header = createPathname(pathname);
		return header;
	}
	else			// If the list is not empty
	{
		temp = header;
		while(temp != NULL)
		{
			if(temp->next == NULL)		// If end of the list
			{
				temp->next = createPathname(pathname);
				return header;
			}			
			temp = temp->next;
		}
	}
	return header;	
}
// Initialize function fills the path list with directories inside the PATH enviroment
struct pathnameList* initializePathnames(struct pathnameList *header)
{
	char path[512];
	char *path_temp = path;
	char *temp;
	strcpy(path,getenv("PATH"));
	while( (temp = strsep(&path_temp,":")) != NULL )	// Directories are separated by ':' in PATH
        	header = insertPathname(header, temp);
	return header;
}
// Delete function goes through the entire list and deletes ALL nodes that match the given pathname
struct pathnameList* deletePathname(struct pathnameList *header,char *pathname)
{
	struct pathnameList *curr;
	struct pathnameList *prev;
	
	if(header == NULL)
	{
		printf("Path list is empty.\n");
		return header;
	}
	else
	{
		curr = header;
		// Since temporary pointer 'prev' isn't defined yet, if the list has only one node, the node is deleted and header is nullified
		if(curr->next == NULL)
		{
			if (!strcmp(pathname, curr->pathname))
			{
				header = NULL;
				free(curr);
				curr = NULL;
				printf("Pathname Removed\n");
			}
			else
			{
				printf("No match found.\n");
			}
			return header;
		}
		// List with multiple nodes case
		while(curr != NULL)
		{
			// Last node case
			if(curr->next == NULL)
			{
				if (!strcmp(pathname, curr->pathname))
				{
					prev->next = NULL;
					free(curr);
					curr = NULL;
					//printf("Pathname(s) Removed\n");
				}
				return header;
			}
			// Search for given pathname inside the list and delete if found
			else
			{
				if (!strcmp(pathname, curr->pathname))
				{
					prev->next = curr->next;
					free(curr);
					curr = prev->next;
				}
				else
				{
					prev = curr;			
					curr = curr->next;
				}
			}
		}
	}
	return header;
}
// Pathnames stored in LL are concatenated with ':' in between them to resemble the PATH environment variable
void listPathnames(struct pathnameList *header)
{
	struct pathnameList *temp;
	char *pathText = malloc(128);		// Memory allocation for concatenation
	temp = header;
	if(temp == NULL)
	{
		printf("No pathnames found.\n");
		free(pathText);
		return;
	}
	else
	{
		while(temp != NULL)
		{
			strcat(pathText, temp->pathname);
			if(temp->next != NULL)
				strcat(pathText, ":");
			temp = temp->next;
		}
		printf("%s\n",pathText);
	}
	free(pathText);				// Freeing previously allocated memory
}

//		HISTORY STRUCTS
//
// Node struct
struct historyList{
	char historyInput[128];
	int index;
	struct historyList *next;
};
// Create node
// historyInput parameter is the arguments parameter. Each argument is concatenated with ' ' in between to reassemble the input entered into myshell
struct historyList* createInput(char *historyInput[], int index)
{
	char *args_temp;
	struct historyList *newNode = (struct historyList*)malloc(sizeof(struct historyList));
	for(int i=0; historyInput[i]!=NULL; i++)
	{
		args_temp = strdup(historyInput[i]);		// Copy each argument into a temp char*
		strcat(newNode->historyInput, args_temp);
		if(historyInput[i+1]!=NULL)		
			strcat(newNode->historyInput, " ");
	}
	newNode->index = index;
	newNode->next = NULL;
	return newNode;
}
// Insert node
// Each node is inserted with the index 0 at the beginning of the history list.
// Node indexes then are shifted by one and node with the index 10 is deleted after every insertion (history limit given in the project example)
struct historyList* insertHistoryInput(struct historyList *header,char *historyInput[])
{
	struct historyList *temp;
	int index=0;
	if(header == NULL)	// If the list is empty
	{
		header = createInput(historyInput, 0);
		return header;
	}
	else			// If the list is not empty
	{
		temp = createInput(historyInput, 0);
		temp->next = header;
		header = temp;
		while(temp != NULL)
		{
			index++;
			if(index == 10 && temp->next != NULL)
			{
				temp = temp->next;
				free(temp);
				return header;
			}
			if(temp->next == NULL)
			{
				return header;
			}
			temp = temp->next;
			temp->index = index;
		}
	}
	return header;
}
// List method
void listHistory(struct historyList *header)
{
	struct historyList *temp;
	temp = header;
	if(temp == NULL)
	{
		printf("History is empty!\n");
		return;
	}
	else
	{
		while(temp != NULL)
		{
			printf("\t%d\t%s\n",temp->index,temp->historyInput);
			temp = temp->next;
		}
	}
}
// Search method
// Find index in the history list and execute stored command with system() call
void searchHistory(struct historyList *header, int index)
{
	struct historyList *temp;
	temp = header;
	if(temp == NULL)
	{
		printf("History is empty!\n");
		return;
	}
	else
	{
		while(temp != NULL)
		{
			if(temp->index == index)
				system(temp->historyInput);	// History input has the command and all arguments combined in one string
			temp = temp->next;
		}
	}
	printf("Index value is not in history\n");
	return;
}

//		BACKGROUND PROCESS STRUCTS
//
// Node struct
struct backgroundProcess{
	char command[32];
	pid_t pid;
	struct backgroundProcess *next;
};
// Create node
// Node's pid is also passed into the list
struct backgroundProcess* createBackgroundProcess(char *command, pid_t pid)
{
	struct backgroundProcess *newNode = (struct backgroundProcess*)malloc(sizeof(struct backgroundProcess));
	strcpy(newNode->command,command);
	newNode->pid = pid;
	newNode->next = NULL;
	return newNode;
}
// Insert node
struct backgroundProcess* insertBackgroundProcess(struct backgroundProcess *header,char *command, pid_t pid)
{
	struct backgroundProcess *temp;
	if(header == NULL)	// If the list is empty
	{
		header = createBackgroundProcess(command, pid);
		return header;
	}
	else			// If the list is not empty
	{
		temp = header;
		while(temp != NULL)
		{
			if(temp->next == NULL)
			{
				temp->next = createBackgroundProcess(command, pid);
				return header;
			}			
			temp = temp->next;
		}
	}
	return header;	
}
// Delete background process with given pid if search is successful
struct backgroundProcess* deleteBackgroundProcess(struct backgroundProcess *header,pid_t pid)
{
	struct backgroundProcess *curr;
	struct backgroundProcess *prev;
	
	if(header == NULL)
	{
		printf("Background list is empty.\n");
		return header;
	}
	else
	{
		curr = header;
		if(curr->next == NULL)
		{
			if (pid = curr->pid)
			{
				header = NULL;
				free(curr);
				curr = NULL;
				printf("Background Process with pid = %d is moved to foreground.\n",pid);
			}
			else
			{
				printf("No match found.\n");
			}
			return header;		// Return after checking the single node in the list
		}
		while(curr != NULL)
		{
			if(curr->next == NULL)
			{
				if (pid = curr->pid)
				{
					prev->next = NULL;
					free(curr);
					curr = NULL;
					printf("Background Process with pid = %d is moved to foreground.\n",pid);
				}
				else
				{
					printf("No match found.\n");
				}
				return header;		// Return after search is complete
			}
			else
			{
				if (pid = curr->pid)
				{
					prev->next = curr->next;
					free(curr);
					curr = prev->next;
					printf("Background Process with pid = %d is moved to foreground.\n",pid);
					return header;	// Return after deleting the process with given pid
				}
				else
				{
					prev = curr;			
					curr = curr->next;
				}
			}
		}
	}
	return header;
}
// List method
void listBackgroundProcesses(struct backgroundProcess *header)
{
	struct backgroundProcess *temp;
	temp = header;
	if(temp == NULL)
	{
		printf("No background processes found.\n");
		return;
	}
	else
	{
		while(temp != NULL)
		{			
			printf("%s %d\n",temp->command,temp->pid);
			temp = temp->next;
		}
	}
}

//		SEARCH METHOD FOR VALIDATING COMMANDS
//
// Executable for the given command name (args[0]) is searched inside directories from the path list
// This is a boolean method that overwrites commandPath string if the command executable was found in one of the directories
int searchCommands(struct pathnameList *header,char *command,char *commandPath)
{
	FILE *file;
	struct pathnameList *temp;
	if(header == NULL)
	{
		printf("Empty path list!");
		return 0;
	}
	else
	{
		temp = header;
		// Check every directory in path list
		while(temp != NULL)
		{
			// commandPath = "path/command"
			strcpy(commandPath, temp->pathname);
			strcat(commandPath, "/");
			strcat(commandPath, command);
			file = fopen(commandPath, "r");
			// If the file exists, the command is valid
			if (file)
			{
				fclose(file);
				return 1;
			}
			temp = temp->next;
		}
		printf("Invalid command or executable isn't inside the path list directories.");
		return 0;
	}
}

/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

	printf(">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		    ct++;
		}
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1){
                    args[ct] = &inputBuffer[start];     
		    ct++;
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		break;

	    default :             /* some other character */
		if (start == -1)
		    start = i;
                if (inputBuffer[i] == '&'){
		    *background  = 1;
                    inputBuffer[i-1] = '\0';
		}
	} /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++)
		printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

int main(void)
{
            char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
            int background; /* equals 1 if a command is followed by '&' */
            char *args[MAX_LINE/2 + 1]; /*command line arguments */

		// Initializations and memory allocation for command path
		struct pathnameList *pathnames = NULL;
		pathnames = initializePathnames(pathnames);
		char *commandPath = malloc(128);
		struct backgroundProcess *backgroundHeader = NULL;
		struct historyList *historyHeader = NULL;
		pid_t pid;
		
            while (1){
                        background = 0;
                        printf("myshell: ");
                        /*setup() calls exit() when Control-D is entered */
                        setup(inputBuffer, args, &background);
		// Remove '&'
		int index = 0;
		while(args[index]!=NULL)	// Iterate until the arguments are empty
			index++;
        	if(index!=0)
		{
			// Setup function has already defined if the process is foreground / background depending on '&' argument
			// Remove '&' from the arguments
			if(!strcmp(args[index-1],"&"))
			{
				args[index-1]=NULL;
			}	
		}
		//		INTERNAL COMMANDS
		//
		//
		if(!args[0])
		{
			// Empty input, return to myshell
			continue;
		}
		// HISTORY
		else if(!strcmp("history", args[0]))
		{
			// "history"
			if(!args[1])
			{
				listHistory(historyHeader);
			}
			// "history -i"
			else if(!strcmp("-i", args[1]))
			{
				// "history -i num"
				if(args[2]!=NULL)
				{
					int index = atoi(args[2]);		// atoi converts char* to int
					searchHistory(historyHeader,index);
				}
				else
				{
					printf("Invalid 'history' command. Enter 'history' or 'history -i num'\n");
					continue;
				}
			}
			else
			{
				printf("Invalid 'history' command. Enter 'history' or 'history -i num'\n");
				continue;
			}
		}
		// PATH LIST
		else if(!strcmp("path", args[0]))
		{
			// "path"
			if(!args[1])
			{
				// Display pathnames	(Ex: "/bin:/sbin")
				listPathnames(pathnames);
			}
			// "path +"
			else if(!strcmp("+", args[1]))
			{
				// "path + /bin"
				if(args[2] != NULL && args[3] == NULL)
				{
					pathnames = insertPathname(pathnames, args[2]);
				}
				else
				{
					printf("You must enter a single valid pathname.\n");
					continue;
				}
			}
			// "path -"
			else if(!strcmp("-", args[1]))
			{
				// "path - /bin"
				if(args[2] != NULL && args[3] == NULL)
				{
					pathnames = deletePathname(pathnames, args[2]);
				}
				else
				{
					printf("You must enter a single valid pathname.\n");
					continue;
				}
			}
			else
			{
				printf("Invalid input. Example syntax: 'path + /bin'\n");
				continue;
			}
		}
		// FG
		else if(!strcmp("fg", args[0]))
		{
			// Move background process to foreground   (args[1] = "%num")
			if(backgroundHeader != NULL)
			{
				backgroundHeader = deleteBackgroundProcess(backgroundHeader,atoi(args[1]));
			}
			else
			{
				printf("There are no background processes still running.\n");
				continue;
			}
		}
		// EXIT
		else if(!strcmp("exit", args[0]))
		{
			// "exit"
			if(!args[1])
			{
				if(backgroundHeader == NULL)
					exit(0);
				else
				{
					printf("There are background processes still running. Terminate the program with CTRL+Z if you wish to exit now.\n");
					continue;
				}
			}
			else
			{
				// Invalid command case
				printf("Invalid input. Use 'exit' with no arguments!\n");
				continue;
			}
		}
		else
		{
			//		Fork / Redirect Sections
			// 1 if successful
			int searchResult = searchCommands(pathnames,args[0],commandPath);
			// Invalid command case
			if(searchResult==0)
				continue;
			// Add command into history
			historyHeader = insertHistoryInput(historyHeader,args);
			// Fork
			pid = fork();
			// Error case
			if (pid == -1)
			{
				perror("Can not fork, error occured.");
				return 1;
			}
			// Child pid
			else if(pid == 0)
			{
				int flag = 0;
				char *arg_list_myprog[] = {};
				// I/O Redirection
				for(int r=0; args[r]!=NULL; r++)	// Iterate for each argument
				{
					// TRUNCATE
					if(!strcmp(args[r],">"))
					{
						flag = 1;
						int fd = open(args[r+1], O_CREAT|O_WRONLY|O_TRUNC, 0644);
						dup2(fd, STDOUT_FILENO);
						close(fd);
						break;
					}
					// APPEND
					else if(!strcmp(args[r],">>"))
					{
						flag = 1;
						int fd = open(args[r+1], O_CREAT|O_WRONLY|O_APPEND, 0644);
						dup2(fd, STDOUT_FILENO);
						close(fd);
						break;
					}
					// INPUT
					else if(!strcmp(args[r],"<"))
					{
						// INPUT AND OUTPUT (myprog[args] < in_file > out_file)
						if(!strcmp(args[r+2],">"))
						{
							flag = 1;
							int fd_in = open(args[r+1], O_RDONLY, 0644);
							dup2(fd_in, STDIN_FILENO);
							close(fd_in);
							int fd_out = open(args[r+3], O_CREAT|O_WRONLY|O_TRUNC, 0644);
							dup2(fd_out, STDOUT_FILENO);
							close(fd_out);
							break;
						}
						// INPUT (myprog[args] < in_file)
						else
						{
							flag = 1;
							int fd = open(args[r+1], O_RDONLY, 0644);
							dup2(fd, STDIN_FILENO);
							close(fd);
							break;
						}
					}
					// STANDARD ERROR
					else if(!strcmp(args[r],"2>"))
					{
						flag = 1;
						int fd = open(args[r+1], O_CREAT|O_WRONLY|O_APPEND, 0644);
						dup2(fd, STDERR_FILENO);
						close(fd);
						break;
					}
					// arg_list_myprog keeps arguments up until I/O redirection symbols
					arg_list_myprog[r] = args[r];
				}
				if(flag)
				{
					execv(commandPath,arg_list_myprog);		// Call execv with command path and arguments without I/O redirection
					perror("Child could not execute command.");
				}
				else
				{
					// Make a list containing slots for all possible arguments	(32 is the assumed limit for the project)
					char *arg_list[] = {args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], 
					args[10], args[11], args[12], args[13], args[14], args[15], args[16], args[17], args[18],
					args[19], args[20], args[21], args[22], args[23], args[24], args[25], args[26], args[27], 
					args[28], args[29], args[30], args[31], NULL};
					execv(commandPath,arg_list);
					perror("Child could not execute command.");
				}
				free(commandPath);	// Free the memory allocated for commandPath
				return 1;
			}
			// Parent pid
			else
			{
				// Add background process to the list
				if(background == 1)
					backgroundHeader = insertBackgroundProcess(backgroundHeader,args[0],pid);
				else
				{
					// If foreground, wait for child to finish process
					int status;
					if (waitpid(pid, &status, 0) > 0)
					{
						if (WEXITSTATUS(status) == 127) {
							// execv failed 
							printf("execv failed\n");
						}
					}
					else 
					{
						printf("waitpid() failed\n"); 
					}
				}
			}
			free(commandPath);	// Free the memory allocated for commandPath
		}
                        /** the steps are:
                        (1) fork a child process using fork()
                        (2) the child process will invoke execv()
						(3) if background == 0, the parent will wait,
                        otherwise it will invoke the setup() function again. */
            }
}
