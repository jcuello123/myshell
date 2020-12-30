/* Jeffrey Cuello
 * 
 * I affirm that this program is original and written by me.
 *
 * This program extends myshell.c and minishell.c in order to add pipelines
 * and I/O redirections. The program redirects according to the symbols >, <, and >>.
 * It wires up the standard input and output in order to run a command
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_ARGS 20
#define BUFSIZE 1024

int get_args(char* cmdline, char* args[]) 
{
  int i = 0;

  /* if no args */
  if((args[0] = strtok(cmdline, "\n\t ")) == NULL) 
    return 0; 

  while((args[++i] = strtok(NULL, "\n\t ")) != NULL) {
    if(i >= MAX_ARGS) {
      printf("Too many arguments!\n");
      exit(1);
    }
  }
  /* the last one is always NULL */
  return i;
}

//counts the number of pipes in the command line
int pipe_count(char* cmdline)
{
 int i = 0;
 int count = 0; 

 for (i = 0; cmdline[i] != '\0'; i++ )
  {
   if (cmdline[i] == '|')
    {
     count++;
    }
  }
 return count;  
}

//checks the command for which redirection to use
int redirection(char* command)
{
 char* output = strstr(command, ">");
 char* input = strstr(command, "<");
 char* append = strstr(command, ">>");

 if (input != NULL && append != NULL)
   return 3;

 else if (output != NULL && input != NULL)
   return 2;

 else if (append != NULL)
   return 4;

 else if (output != NULL)
   return 1;

 else if (input != NULL)
   return 0;

 else
   return -1;
  
}

//parses the commandline depending on which redirection was used
const char* getcommand(char* cmdline)
{
 int rd = redirection(cmdline);
 char str[200], both[200];
 strcpy(str, cmdline);
 char* cmd = malloc(sizeof (char) * 200);

 if (rd == 1){
   cmd = strtok(str, ">");
   int len = strlen(cmd) - 1;
   cmd[len] = '\0';
   return cmd;
 }

 else if (rd == 4){
   cmd = strtok(str, ">>");
   int len = strlen(cmd) - 1;
   cmd[len] = '\0';
   return cmd;
 }

 else if (rd == 0){
   cmd = strtok(str, "<");
   int len = strlen(cmd) - 1;
   cmd[len] = '\0';
   return cmd;
 }

 else if (rd == 3 || rd == 2){
   int i = 0, j = 0, count = 0;
   
   for (i = 0; count != 3; i++){
     if (str[i] == ' ')
       count++;

     if (count >= 3)
       break;

     if (str[i] == '>' || str[i] == '<'){
       i++;
       continue;
     }
     else if (count < 2 && str[i] == ' '){
       cmd[j] = str[i];
     }
     else if (str[i] != ' ')
       cmd[j] = str[i];

     j++;

   } 
    cmd[j] = '\0';
    return cmd;
  } 

}

//parses the file name depending on which redirection was used
const char* getfilename(char* cmdline, int rd)
{

 if (rd == 1){
   char* filename = malloc(sizeof (char) * 200);
   int i = 0, j = 0;

 while (cmdline[i] != '>'){
   i++;
 }

 i += 2;

 while(cmdline[i] != '\0'){
   filename[j] = cmdline[i];
   i++;
   j++;
  }
 filename[j - 1] = '\0';
 return filename;
 }

 else if (rd == 4){
   char* filename = malloc(sizeof (char) * 200);
   int i = 0, j = 0;

 while (cmdline[i] != '>' && cmdline[i + 1] != '>'){
   i++;
 }

 i += 4;

 while(cmdline[i] != '\0'){
   filename[j] = cmdline[i];
   i++;
   j++;
  }
 filename[j - 1] = '\0';
 return filename;
 }

 else if (rd == 0){
   char* filename = malloc(sizeof (char) * 200);
   int i = 0, j = 0;

 while (cmdline[i] != '<'){
   i++;
 }

 i += 2;

 while(cmdline[i] != '\0'){
   filename[j] = cmdline[i];
   i++;
   j++;
  }
 filename[j - 1] = '\0';
 return filename;
 }

 else if (rd == 2 || rd == 3){
   char* filename = malloc(sizeof (char) * 200);
   int i = 0, j = 0,count = 0;

   for (i = 0; cmdline[i] != '\0'; i++){
     if (cmdline[i] == ' '){
       count++;
       continue;
     }

     if (count >= 4){
       filename[j] = cmdline[i];
       j++;
     }
   }
   filename[j] = '\0';
   return filename;
 }

 else if (rd == 5){
   char* filename = malloc(sizeof (char) * 200);
   int i = 0, j = 0,count = 0;

   for (i = 0; count != 3; i++){
     if (cmdline[i] == ' '){
       count++;
       continue;
     }

     if (count >= 3)
       break;

     if (count == 2){
       filename[j] = cmdline[i];
       j++;
     }
   }
   
   return filename;
 }

}

//the < command must be first, so this function organizes the command line to have
//< as the first command
const char* organize(char* cmdline)
{
  char* orgcmdline = malloc(sizeof (char) * 200);
  char* sub1 = malloc(sizeof (char) * 200);
  char* sub2 = malloc(sizeof (char) * 200);
  char* sub3 = malloc(sizeof (char) * 200);

  int i = 0, j = 0;
  int count = 0;

  int len = strlen(cmdline) - 1;
  cmdline[len] = '\0';

  while (cmdline[i] != ' '){
    sub1[i] = cmdline[i];
    i++;
  }
  
  sub1[i] = ' ';
  sub1[i + 1] = '\0';
  i++;

  while (count != 2){
    if (cmdline[i] == ' ')
      count++;

    if (count == 2)
      break;

    sub2[j] = cmdline[i];
    i++;
    j++;
  }
  
  sub2[j] = ' ';
  sub2[j + 1] = '\0';
  i++;
  j = 0;

  while(cmdline[i] != '\0'){
    sub3[j] = cmdline[i];
    i++;
    j++;
  }

  sub3[j] = ' ';
  sub3[j + 1] = '\0';
 
  if (redirection(sub1) == -1)
    strcpy(orgcmdline, sub1);

  else if (redirection(sub2) == -1)
    strcpy(orgcmdline, sub2);
  
  else if (redirection(sub3) == -1)
    strcpy(orgcmdline, sub3);

  if (redirection(sub1) == 0)
    strcat(orgcmdline, sub1);

  else if (redirection(sub2) == 0)
    strcat(orgcmdline, sub2);
  
  else if (redirection(sub3) == 0)
    strcat(orgcmdline, sub3);

  if (redirection(sub1) == 1 || redirection(sub1) == 4){
    sub1[strlen(sub1) - 1] = '\0'; 
    strcat(orgcmdline, sub1);
  }

  else if (redirection(sub2) == 1 || redirection(sub2) == 4){
    sub2[strlen(sub2) - 1] = '\0';
    strcat(orgcmdline, sub2);
  }
  
  else if (redirection(sub3) == 1 || redirection(sub3) == 4){
    sub3[strlen(sub3) - 1] = '\0';
    strcat(orgcmdline, sub3);
  }

  return orgcmdline;
}

//if there is a pipe, split the command line into two commands
const char* split(char* cmdline, int half)
{
  char* cmd = malloc(sizeof(char) * 200);
  char str[200];
  strcpy(str, cmdline);
  cmdline[strlen(cmdline)] = '\0';

  if (half == 1){
   cmd = strtok(str, "|");
   int len = strlen(cmd);
   
    if (cmd[strlen(cmd) - 1] == ' '){
      cmd[strlen(cmd) - 1] = '\0';
    }
  }

  else if (half == 2){
    int i = 0, j = 0;
    
    while (cmdline[i] != '|')
      i++;

    i += 2;

    while (cmdline[i] != '\0'){
      cmd[j] = cmdline[i];
      i++;
      j++;
    }
    int len = strlen(cmd);
    cmd[len - 1] = '\0';
  }

  return cmd;
}

void execute(char* cmdline) 
{
  int pid, pid2, async;
  char* args[MAX_ARGS];
  int npipes = pipe_count(cmdline);
  int fdout, fdout2;
  int fd1[2], fd2[2];
  int rd = redirection(cmdline);

  char* entirecmdline = malloc(sizeof(char) * 200);
  strcpy(entirecmdline, cmdline);

  int nargs = get_args(cmdline, args);
  if(nargs <= 0) return;

  if(!strcmp(args[0], "quit") || !strcmp(args[0], "exit")) {
    exit(0);
  }

  /* check if async call */
  if(!strcmp(args[nargs-1], "&")) { async = 1; args[--nargs] = 0; }
  else async = 0;

  if (npipes <= 0){
    if (rd == 1 || rd == 4 || rd == 0 || rd == 2 || rd == 3){
      char* temp = malloc(sizeof(char) * 200);
      char* orgline = malloc(sizeof(char) * 200);


      if (rd == 2 || rd == 3){
	strcpy(orgline, organize(entirecmdline));
	strcpy(temp, getcommand(orgline));
      }

      else {
        strcpy(temp, getcommand(entirecmdline)); 
      }

      char* cmd1 = malloc(sizeof(char) * 200);
      char* cmd2 = malloc(sizeof(char) * 200);
      
      char* file = malloc(sizeof(char) * 200);
      char* file2 = malloc(sizeof(char) * 200);

      //parse the filename and open it, depending on which redirection
      if (rd == 1){
        strcpy(file, getfilename(entirecmdline, 1)); 

        if ((fdout = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

      else if (rd == 4){
        strcpy(file, getfilename(entirecmdline, 4));

        if ((fdout = open(file, O_APPEND | O_CREAT | O_WRONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

      else if (rd == 0){
        strcpy(file, getfilename(entirecmdline, 0));
	
        if ((fdout = open(file, O_RDONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

      else if (rd == 2 || rd == 3){
        strcpy(file, getfilename(orgline, 5));
	strcpy(file2, getfilename(orgline, 2));
        
        if ((fdout = open(file, O_RDONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }

	if ((fdout2 = open(file2, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

      //parse the arguments if the command line has any. ex: sort file
      //will split into "sort" and "file"
      int i = 0;
      int j = 0;
      bool seccmd = false;

      while(temp[i] != '\0'){
	cmd1[i] = temp[i];
	i++;
	if (temp[i] == ' ')
	  break;
      }

      cmd1[i] = '\0';

      if (temp[i] == ' '){
        seccmd = true;
	i++;

        while(temp[i] != '\0'){
	  cmd2[j] = temp[i];
          i++;
	  j++;
        }
        cmd2[j] = '\0';
      }

      int size;

      if (seccmd){
        size = 3;
      }

      else{
        size = 2;
      }

      char *cmdtoexec[size];
      
      //set up the cmd to exec depending on if there are arguments or not
      if (seccmd){
	cmdtoexec[0] = cmd1;
	cmdtoexec[1] = cmd2;
	cmdtoexec[2] = NULL;
      }

      else {
	cmdtoexec[0] = cmd1;
	cmdtoexec[1] = NULL;
      }
      
      //read or write a file depending on which redirection was used
      if (rd == 1 || rd == 4){
        dup2(fdout, 1);
        close(fdout);
      }

      else if (rd == 0){
        dup2(fdout, 0);
        close(fdout);
      }

      else if (rd == 2 || rd == 3){
        dup2(fdout, 0);
        close(fdout);

        dup2(fdout2, 1);
        close(fdout2);
      }
      
      //execute the command
      execvp(cmdtoexec[0], cmdtoexec);

      perror(cmdtoexec[0]);
      exit(1);
  }

  //if no redirections. ex: ls
  else if (rd == -1){
    pid = fork();
    if(pid == 0) { // child process 
    execvp(args[0], args);
    // return only when exec fails 
    perror("exec failed");
    exit(-1);
  } else if(pid > 0) { // parent process 
    if(!async) waitpid(pid, NULL, 0);
    else printf("this is an async call\n");
  } else { // error occurred 
    perror("fork failed");
    exit(1);
  }

}
 
}
  //if the command line contains a pipe
  else if(npipes == 1){
    char* cmd1 = malloc(sizeof(char) * 200);
    char* cmd2 = malloc(sizeof(char) * 200);
    char* cmd3 = malloc(sizeof(char) * 200);
    char* cmd4 = malloc(sizeof(char) * 200);

    char* file1 = malloc(sizeof(char) * 200);
    char* file2 = malloc(sizeof(char) * 200);
    char* file3 = malloc(sizeof(char) * 200);
    char* file4 = malloc(sizeof(char) * 200);

    //split the commandline into 2 different commands
    char* orig1 = malloc(sizeof(char) * 200);
    char* orig2 = malloc(sizeof(char) * 200);
    strcpy(orig1, split(entirecmdline, 1));
    strcpy(orig2, split(entirecmdline, 2));


    char* temp = malloc(sizeof(char) * 200);
    char* orgline = malloc(sizeof(char) * 200);
    strcpy(temp, split(entirecmdline, 1));

    //parse the command depending on which redirection was used
    if (redirection(temp) == 0 || redirection(temp) == 1 || redirection(temp) == 4){
      strcpy(temp, getcommand(temp));
    }

    else if (redirection(temp) == 2 || redirection(temp) == 3){
      strcpy(orgline, organize(temp));
      strcpy(temp, getcommand(orgline));
    }

    char* temp2 = malloc(sizeof(char) * 200);
    strcpy(temp2, split(entirecmdline, 2));

    if (redirection(temp2) == 0 || redirection(temp2) == 1 || redirection(temp2) == 4){
      strcpy(temp2, getcommand(temp2));
    }

    else if (redirection(temp2) == 2 || redirection(temp2) == 3){
      strcpy(temp2, organize(temp2));
      strcpy(temp2, getcommand(temp2));
    }
    
    if (redirection(orig1) == 1){
        strcpy(file1, getfilename(orig1, 1)); 

        if ((fdout = open(file1, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

     else if (redirection(orig1) == 4){
       strcpy(file1, getfilename(orig1, 4));

        if ((fdout = open(file1, O_APPEND | O_CREAT | O_WRONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

     else if (redirection(orig1) == 0){
       strcpy(file1, getfilename(orig1, 0));
	
        if ((fdout = open(file1, O_RDONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }


     else if (redirection(orig1) == 2 || redirection(orig1) == 3){
       strcpy(orig1, organize(orig1));
       strcpy(file1, getfilename(orig1, 5));
       strcpy(file2, getfilename(orig1, 2));
        
        if ((fdout = open(file1, O_RDONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }

	if ((fdout2 = open(file2, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

        //parse the second half of the pipe
        
     if (redirection(orig2) == 1){
        strcpy(file3, getfilename(orig2, 1)); 

        if ((fdout = open(file3, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

      else if (redirection(orig2) == 4){
        strcpy(file3, getfilename(orig2, 4));

        if ((fdout = open(file3, O_APPEND | O_CREAT | O_WRONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

      else if (redirection(orig2) == 0){
        strcpy(file3, getfilename(orig2, 0));
	
        if ((fdout = open(file2, O_RDONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }

      else if (redirection(orig2) == 2 || redirection(orig2) == 3){
	strcpy(orig2, organize(orig2));
        strcpy(file3, getfilename(orig2, 5));
	strcpy(file4, getfilename(orig2, 2));
        
        if ((fdout = open(file3, O_RDONLY , 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }

	if ((fdout2 = open(file4, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0){
          perror("Error with opening file");
	  exit(1);
        }
      }
      
      //parse the arguments of the command line, if any
      int i = 0;
      int j = 0;
      bool seccmd = false;

      while(temp[i] != '\0'){
	cmd1[i] = temp[i];
	i++;
	if (temp[i] == ' ')
	  break;
      }

      cmd1[i] = '\0';

      if (temp[i] == ' '){
        seccmd = true;
	i++;

        while(temp[i] != '\0'){
	  cmd2[j] = temp[i];
          i++;
	  j++;
        }
        cmd2[j] = '\0';
      }

      int size;

      if (seccmd){
        size = 3;
      }

      else{
        size = 2;
      }
    
      char* cmdtoexec[size];
      
      //set up the cmd to exec depending if there are any arguments in the command line
      if (seccmd){
	cmdtoexec[0] = cmd1;
	cmdtoexec[1] = cmd2;
	cmdtoexec[2] = NULL;
      }

      else {
	cmdtoexec[0] = cmd1;
	cmdtoexec[1] = NULL;
      }

      //parse the arguments of the second half of the pipe
      
      i = 0;
      j = 0;
      seccmd = false;

      while(temp2[i] != '\0'){
	cmd3[i] = temp2[i];
	i++;
	if (temp2[i] == ' ')
	  break;
      }

      cmd3[strlen(cmd3)] = '\0';

      if (temp2[i] == ' '){
        seccmd = true;
	i++;

        while(temp2[i] != '\0'){
	  cmd4[j] = temp2[i];
          i++;
	  j++;
        }
        cmd4[j] = '\0';
      }

      if (seccmd){
        size = 3;
      }

      else{
        size = 2;
      }

      char* cmdtoexec2[size];
      
      //set up the cmd to exec depending if there are any arguments
      if (seccmd){
	cmdtoexec2[0] = cmd3;
	cmdtoexec2[1] = cmd4;
	cmdtoexec2[2] = NULL;
      }

      else {
	cmdtoexec2[0] = cmd3;
	cmdtoexec2[1] = NULL;
      }

      //pipe set up
      if (pipe(fd1) == -1){
		perror("Pipe Failed");
	}

	pid = fork();
	
	if (pid < 0){
		perror("Fork one failed\n");
	}

	else if (pid == 0){
		close(fd1[0]);  //close reading end
		dup2(fd1[1], 1);
		close(fd1[1]); // close writing end
		execvp(cmdtoexec[0], cmdtoexec); // execute first command
	}

	pid2 = fork();

	if (pid2 < 0){
		perror("Fork two failed\n");
	}

	else if (pid2 == 0){
		close(fd1[1]);  // close writing end
		dup2(fd1[0], 0);
		close(fd1[0]);  //close reading end
		execvp(cmdtoexec2[0], cmdtoexec2);  //execute second command
	}

	if (pid > 0 && pid2 > 0){ //if > 0 then it is parent process
          wait(NULL);  //waits for both children to finish
 	  exit(1);           
	}

 }
}

int main (int argc, char* argv [])
{
  char cmdline[BUFSIZE];
  
  for(;;) {
    printf("COP4338$ ");
    if(fgets(cmdline, BUFSIZE, stdin) == NULL) {
      perror("fgets failed");
      exit(1);
    }
    execute(cmdline) ;
  }
  return 0;
}
