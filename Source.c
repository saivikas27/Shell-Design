// C Program to design a shell in Linux 
#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h>
#include<errno.h>
#include<setjmp.h> 
#include<readline/readline.h> 
#include<readline/history.h> 


#define MAX 100
 
// Clearing the shell using escape sequences 
#define clear() printf("\033[H\033[J") 

// Greeting shell during startup 
void welcome() 
{    
        //clear the screen
	clear();
        //To print the Welcome Screen 
	printf("\nThis is the Programmed Shell"); 
        //To get the Username and print it
	char* username = getenv("USER"); 
	printf("\nThe User name is: %s", username); 
	printf("\n"); 
        //Stops the execution for 1 second.
	sleep(1); 
	clear(); 
} 

//Handle exit signals from child processes
void sig_handler(int signal) {
  int status;
  int result = wait(&status);
}


//Function to read input from the shell
int inputline(char* str) 
{ 
	char* temp; 
        //We have used readline
	temp = readline("\n--> "); 
	if (strlen(temp) != 0) { 
		add_history(temp); 
		strcpy(str,temp); 
		return 0; 
	} else { 
		return 1; 
	} 
} 

// Function to print Current Directory before every command. 
void printDirectory() 
{ 
	char cwd[1024]; 
        //Getting the username and current working directory.
        char* username = getenv("USER");
	getcwd(cwd, sizeof(cwd)); 
        //printing them
	printf("\n %s@Directory: %s",username,cwd); 
}
// Function where the piped system commands is executed 
void PipedComExec(char** firstCom, char** secondCom) 
{        
     int fds[2];
     pipe(fds);
     int i;

         //forking to execute mutiple times
         pid_t pid=fork();

         if(pid==-1){
           printf("error\n");
           }
         else if(pid==0){
           //child process
           close(fds[1]);
           dup2(fds[0],0);
           //execution of the second command after pipe operator
           execvp(secondCom[0],secondCom);
           }
         else{
           //parent process
           close(fds[0]);
           dup2(fds[1],1); 
           //execution of the first command before pipe operator
           execvp(firstCom[0],firstCom);
  }
}
// function for parsing command words 
void parsing(char* str, char** tokens) 
{ 
	int i; 
        //for the total length we parse using space 
	for (i = 0; i < MAX; i++) { 
		tokens[i] = strsep(&str," "); 
         //if the end occurs then this will come out of the loop
		if (tokens[i] == NULL) 
			break; 
		if (strlen(tokens[i]) == 0) 
			i--; 
                  
          }
 
} 



// Function to execute builtin commands 
int writtenCommands(char** parsed) 
{ 
	int N= 2,i,k= 0; 
	char* L[N]; 
	char* user; 
        //The commands are stored in the list L
	L[0] = "exit"; 
	L[1] = "cd"; 
        //The parsed words are compared with these written commands
	for (i = 0; i < N; i++) { 
		if (strcmp(parsed[0], L[i]) == 0) { 
			k = i + 1; 
			break; 
		} 
	} 
        
	switch (k) { 
	case 1: 
                printf("\nYou are going to exit the shell");
		printf("\nGoodbye\n"); 
       //Exit is executed
		exit(0); 
	case 2: 
        // change of directory is executed
		chdir(parsed[1]); 
		return 1; 
	default: 
                
		break; 
	} 

	return 0; 
} 
/*
 * Check for pipe operator
 */
int piped(char **args) {
  int i;

  for(i = 1; args[i] != NULL; i++) ;

  if(args[i-1][0] =='|') {
   //Pass this to be the Executable function
   PipedComExec(args,args);
    return 1;
  } else {
    return 0;
  }
  
  return 0;
}


 // Check for ampersand as the last argument

int ampersand(char **args) {
  int i;

  for(i = 1; args[i] != NULL; i++) ;
  
  if(args[i-1][0] =='&') {
/*Deleting that command but returning int indicating ampersand's presence*/
    args[i-1] = NULL;
    return 1;
  } else {
    return 0;
  }
  
  return 0;
}
 
//Commands Execution
int Exe(char **tokens, int amper,int i, char *i_name,int o, char *o_name) {
  
  int result;
  pid_t child_pid;
  int status;

  // Fork the child process
  child_pid = fork();

  // Check for errors in fork()
  switch(child_pid) {
  case EAGAIN:
    //Error due to I/O
    perror("Error EAGAIN: ");
    return 0;
  case ENOMEM:
    //Not able to allocate memory error
    perror("Error ENOMEM: ");
    return 1;
  }

  if(child_pid == 0) {
     
    // Set up redirection in the child process
    if(i)
     //if there is an input symbol
      freopen(i_name, "r", stdin);

    if(o)
      //if there is an output symbol
      freopen(o_name, "w+", stdout);

    // Execute the command
    result = execvp(tokens[0],tokens);

    exit(-1);
  }

  // Wait for the child process to complete, 
  if(amper) {
    result = waitpid(child_pid, &status, 0);
  }
}



 // Check for input redirection
 
int inputSymbolCheck(char **tokens, char **i_name) {
  int i;
  int j;

  for(i = 0; tokens[i] != NULL; i++) {

    // Checking for the < symbol
    if(tokens[i][0] == '<') {
     

      // Reading the command if its not null
      if(tokens[i+1] != NULL) {
	*i_name = tokens[i+1];
      } else {
	return -1;
      }

      // Adjusting the other commands
      for(j = i; tokens[j-1] != NULL; j++) {
	tokens[j] = tokens[j+2];
      }

      return 1;
    }
  }

  return 0;
}



 //Check for output redirection

int outputSymbolCheck(char **tokens, char **o_name) {
  int i;
  int j;

  for(i = 0; tokens[i] != NULL; i++) {

    // Look for the > symbol
    if(tokens[i][0] == '>') {
     
      // Reading the command if its not null
      if(tokens[i+1] != NULL) {
	*o_name = tokens[i+1];
      } else {
	return -1;
      }

      // Adjusting the other commands
      for(j = i; tokens[j-1] != NULL; j++) {
	tokens[j] = tokens[j+2];
      }

      return 1;
    }
  }

  return 0;
}
 //The main function
int main() 
{ 
  char inputComs[MAX]; 
  char *tokens[MAX];  
  int i,o;
  char *o_name;
  char *i_name;
  int a;
        //The Welcome Page of the shell gets printed
	welcome(); 
        //Infinite loop till there exists a exit command or break
	while (1) { 
                // Printing the directory name
		printDirectory(); 
                //Check if there is inout or not
                //if not persent then iterate through loop once more 
		if (inputline(inputComs)) 
	            continue; 
                //parsing those input commands
		parsing(inputComs,tokens);
                //if there is pipe operator then it is executed
                //The continue loops to taking new command
                if(piped(tokens))
                    continue;
                //checking for ampersand 
                a=(ampersand(tokens) == 0); 
                //Executing the commands if they are defined
                writtenCommands(tokens);
               
    // Look for < symbol
    i= inputSymbolCheck(tokens, &i_name);

    switch(i) {
    case -1:
            //There is an error
            printf("Syntax error!\n");
            continue;
            break;
    case 0: 
            //< symbol not present
            break;
    case 1:
      // < present so outputing the required file or command name
            printf("Redirecting input from: %s\n", i_name);
            break;
    }

    // Check for redirected output
    o= outputSymbolCheck(tokens, &o_name);

    switch(o) {
    case -1:
           //There is an error
           printf("Syntax error!\n");
           continue;
           break;
    case 0:
           //< symbol not present
           break;
    case 1:
      // < present so outputing the required file or command name
           printf("Redirecting output to: %s\n", o_name);
           break;
    }
      
       // Execute the commands
       Exe(tokens,a,i, i_name,o,o_name);
           
	} 
	return 0; 
}
