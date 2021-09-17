/*
    MyShell Program
    CS421 - Operating Systems
    myshell.c
    By: Christian Magpantay
    Code/Book Reference:
    Operating Systems Concepts, Enhanced Edition by 
    A. Silberschatz, G. Gange, P. Galvin
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 50
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_HISTORY 10
static char buffer[BUFFER_SIZE];

/* to hold history of user's commands using bare minimum queue */
struct Node {
    char data[MAX_LINE];
    struct Node *next;
};
struct queue {
    struct Node *front, *newNode, *temp, *toRemove;
};
void initQueue(struct queue *q) {
    q->front = NULL;
}
void enqueue(struct queue *q, char command[]) {
    int i = 0;
    q->newNode = (struct Node *)malloc(sizeof(struct Node));
    if(q->front==NULL) {
        while(command[i]){
            q->newNode->data[i] = command[i];
        }
        q->newNode->next = NULL;
        q->front = q->newNode; 
        q->temp = q->front;
    } else {
        while(command[i]){
            q->newNode->data[i] = command[i];
        }
        q->newNode->next = NULL;
        q->temp->next = q->newNode;
        q->temp = q->newNode;
    }
}
void dequeue(struct queue *q){
    q->toRemove = q->front;
    q->front = q->front->next;
    free(q->toRemove);
}

char* display(struct queue *q){
    if(q->front == NULL){
        return "NULL\n";
    }
    else {
        return q->front->data;
    }
}

/* signal handler8 */
void handle_SIGTSTP() {
    write(STDOUT_FILENO,buffer,strlen(buffer));
}

/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 */

void setup(char inputBuffer[], char *args[],int *background, int *words)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */


    ct = 0;
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE); 

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */
    if (length < 0){
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    /* examine every character in the inputBuffer */
    for (i=0;i<length;i++) {
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
                start = -1;
                inputBuffer[i] = '\0';
            }
          }
     } 
     *words = ct;  
     args[ct] = NULL; /* just in case the input line was > 80 */
}

/**
 * runChild() runs the command by using fork(). It first checks if the 
 * inputBufer holds exit to run exit(), and yell to use yell()
 * If not, execute the command and print prompt of child pid and 
 * whether the background is true or false
 */
void runChild(char inputBuffer[], char *args[],int background, int words) {
    pid_t pid;    
    char command[] = "ps -o pid,ppid,pcpu,pmem,etime,user,command";
    int i, j;
    int status;


    if (strcmp(inputBuffer, "exit") == 0){          // exit command
        printf("PID\t PPID\t %CPUD\t %MEM\t ELASPED USER\t COMMAND\n");
        system(command);
        printf("\n");
        exit(0);            
    }
    else if( strcmp(inputBuffer, "yell" ) == 0 ) {  // yell command
        for(i = 1; i < words; i++) {
            for(j = 0; j < strlen(args[i]); j++){
                printf("%c", toupper(args[i][j]));
                if(j == strlen(args[i])-1)
                    printf(" ");
            }
        }
        printf("\n");
    }
    else {  // fork() process
        pid = fork();
        if (pid < 0) {     /* fork a child process           */
            printf("*** ERROR: forking child process failed\n");
            exit(1);
        } else if (pid == 0) {
            execvp(inputBuffer, args);
        } else {
            printf("[Child pid = %d, background = %d]\n\n", pid, background);
            if(background == 0)
                wait(NULL);
            else
                waitpid(pid, &status, 0);
                printf("Child process completed.\n");
        }
    }
}

/* to print prompt and welcome message */
void shellPrompt(int num){
    int pid = getpid();
    if(num == 1){
        printf("Welcome to cmshell. My pid is %d\n", pid);
    }
    printf("\ncmshell[%d]:", num);
}

void main(void)
{
    char inputBuffer[MAX_LINE];     /* buffer to hold the command entered */ 
    char *args[(MAX_LINE/2)+1];     /* command line (of 80) has max of 40 arguments */
    char *history[10];              /* history buffer */
    int words;                      /* used for the yell(), counts words to yell */
    int promptNum = 1;              /* init prompt number */
    int background;                 /* equals 1 if a command is followed by '&' */
    int nodeCounter;

    /* set up the history handler */
    struct queue *q;
    q = malloc(sizeof(struct queue));
    initQueue(q);

    /* set up the signal handler */
    struct sigaction handler;
    handler.sa_handler = handle_SIGTSTP;
    handler.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &handler, NULL);
    strcpy(buffer,"Caught <ctrl><z>\n");


    /**
     * init set background and words to 0
     * display prompt
     * print newline to show prompt
     * setup()
     * runChild() which runs the command with fork()
     * increment prompt number for display
     * end when exit() is called otherwise loop infinitely
     */
    while (1) {                     /* Program terminates normally inside setup */
        background = 0;             
        words = 0;
        shellPrompt(promptNum);     /*   display a prompt  */
        printf("\n");
        setup(inputBuffer,args,&background, &words);    /* get next command */
        // if(nodeCounter == 10){
        //     dequeue(q);
        //     enqueue(q,inputBuffer);
        // } else {
        //     enqueue(q,inputBuffer);
        //     nodeCounter++;
        // }
        runChild(inputBuffer, args, background, words); /* otherwise, execute the command */
        promptNum++;
        
        /* the steps are:
		(0) if built-in command, handle internally
		(1) if not, fork a child process using fork()
		(2) the child process will invoke execvp()
		(3) if background == 0, the parent will wait,
				otherwise returns to the setup() function. */
    }
}