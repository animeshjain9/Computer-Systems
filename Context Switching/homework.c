/*

 * file:        homework.c
 * description: Skeleton for homework 1
 *
 * CS 5600, Computer Systems, Northeastern CCIS
 * Peter Desnoyers, Jan. 2012
 * $Id: homework.c 500 2012-01-15 16:15:23Z pjd $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uprog.h"

/***********************************/
/* Declarations for code in misc.c */
/***********************************/

typedef int *stack_ptr_t;
extern void init_memory(void);
extern void do_switch(stack_ptr_t *location_for_old_sp, stack_ptr_t new_value);
extern stack_ptr_t setup_stack(int *stack, void *func);
extern int get_term_input(char *buf, size_t len);
extern void init_terms(void);

extern void  *proc1;
extern void  *proc1_stack;
extern void  *proc2;
extern void  *proc2_stack;
extern void **vector;


/***********************************************/
/********* Your code starts here ***************/
/***********************************************/

/*
 * Question 1.
 *
 * The micro-program q1prog.c has already been written, and uses the
 * 'print' micro-system-call (index 0 in the vector table) to print
 * out "Hello world".
 *
 * You'll need to write the (very simple) print() function below, and
 * then put a pointer to it in vector[0].
 *
 * Then you read the micro-program 'q1prog' into memory starting at
 * address 'proc1', and execute it, printing "Hello world".
 *
 */
void print(char *line)
{

  printf("%s", line);
}

void q1(void)
{
   
  FILE *fp= NULL; //initializing file pointer
  void (*functionPointer)() = NULL; //initialized function pointer
  long numbytes = 0; // to store number of bytes in the file
  
  fp = fopen("q1prog", "r"); // opening q1prog in read mode
  fseek(fp, 0L, SEEK_END); // seek the file till end
  numbytes = ftell(fp); // total number of bytes in the file q1prog
  rewind(fp); // bring back the function pointer to beginning
  
  vector[0]= &print; // set address of print function to vector[0] as specified in comments above
  
  fread(proc1, numbytes , 1, fp); //reading q1prog through proc1 pointer
  functionPointer=proc1; // setting function pointer to proc1
  functionPointer(); // calling the function

}


/*
 * Question 2.
 *
 * Add two more functions to the vector table:
 *   void readline(char *buf, int len) - read a line of input into 'buf'
 *   char *getarg(int i) - gets the i'th argument (see below)

 * Write a simple command line which prints a prompt and reads command
 * lines of the form 'cmd arg1 arg2 ...'. For each command line:
 *   - save arg1, arg2, ... in a location where they can be retrieved
 *     by 'getarg'
 *   - load and run the micro-program named by 'cmd'
 *   - if the command is "quit", then exit rather than running anything
 *
 * Note that this should be a general command line, allowing you to
 * execute arbitrary commands that you may not have written yet. You
 * are provided with a command that will work with this - 'q2prog',
 * which is a simple version of the 'grep' command.
 *
 * NOTE - your vector assignments have to mirror the ones in vector.s:
 *   0 = print
 *   1 = readline
 *   2 = getarg
 */
void readline(char *buf, int len) /* vector index = 1 */
{

  FILE *fp = stdin; // taking standard input from command line
  fgets(buf, len-1 ,fp); // reading input into buffer using file pointer
  int Count; // Counter for length of string
  Count = strlen(buf); // length of buf
  buf[Count-1] = '\0'; // appending null character at end of string
  proc2 = buf; // assigning reference of buf to proc2
}

char *globalArray[1024];

char *getarg(int i)		/* vector index = 2 */
{
  return globalArray[i+1]; // getarg returns the 'i'th argument
}


void q2(void)
{	
  vector[0] = &print;
  vector[1] = &readline;
  vector[2] = &getarg;
  
  while (1) {
    char *buf = NULL;
    char buffer[1024]= "";
    char *token=NULL;
    int flag=0;

    buf  =  (char *)malloc(1024 *sizeof(char)); // allocating memory for buffer - 1024
    proc2 = buffer;
    readline(buf, 1024);
	
    int i;
    char *command = NULL;
    int length = strlen(proc2);
	

    if(length == 0) continue; // check if any command is entered, if not then continue to next line
    
    for( i = 0; i<strlen(proc2)-1; i++)
      {
	
	if(buffer[i]!= ' ' && i == length) // only one word command is entered (eg. quit, q1prog)
	  {
	    command = proc2;// treat whole string as command
	    break;
	  }
	else
	  {
	    flag = 1; // command and arguments are entered ( q2prog test ) 
	  }
      }
    
    if(flag == 1)
      {
      
	int j =-1;
	command = strtok(proc2," ");
	token = command;
       
	while(token != NULL  ) // separate words on command line based on spaces
	  {
	    j = j+1;
	    globalArray[j] = token;
	    token = strtok(NULL," ");
	  }
	
	globalArray[j+1] = NULL; // terminate every execution with a NULL at end of line in global array
      }

    
    FILE *fp= NULL; //initializing file pointer
    void (*functionPointer)() = NULL; //initialized function pointer
    long numbytes = 0; // to store number of bytes in the file

    if(strcmp("quit",command)==0)
      {
	break; // break if command is quit
      }
    else
      {
       
	fp = fopen(command, "r"); // opening q1prog in read mode
       
	if (fp == NULL) // invalid command is entered
	  {	
	    printf("can't open file\n");
	    continue;
	  }
	
	fseek(fp, 0L, SEEK_END); // seek the file till end
	numbytes = ftell(fp); // total number of bytes in the file q1prog
	rewind(fp); // bring back the function pointer to beginning
	fread(proc1, numbytes , 1, fp); //reading q1prog through proc1 pointer
	functionPointer=proc1; // setting function pointer to proc1
	functionPointer(); // calling the function
	fclose(fp);
      }
    
  }
  
}

/*
 * Question 3.
 *
 * Create two processes which switch back and forth.
 *
 * You will need to add another 3 functions to the table:
 *   void yield12(void) - save process 1, switch to process 2
 *   void yield21(void) - save process 2, switch to process 1
 *   void uexit(void) - return to original homework.c stack
 *
 * The code for this question will load 2 micro-programs, q3prog1 and
 * q3prog2, which are provided and merely consists of interleaved
 * calls to yield12() or yield21() and print(), finishing with uexit().
 *
 * Hints:
 * - Use setup_stack() to set up the stack for each process. It returns
 *   a stack pointer value which you can switch to.
 * - you need a global variable for each process to store its context
 *   (i.e. stack pointer)
 * - To start you use do_switch() to switch to the stack pointer for 
 *   process 1
 */

stack_ptr_t first_pointer = NULL; // global stack pointer for process 1
stack_ptr_t second_pointer = NULL; // global stack pointer for process 2
stack_ptr_t homework_pointer = NULL; // stack pointer for returning into parent stack

void yield12(void)		/* vector index = 3 */
{

  do_switch(&first_pointer, second_pointer); //switching to the stack pointer of process 2 and save the stack pointer for process1
}

void yield21(void)		/* vector index = 4 */
{
  do_switch(&second_pointer, first_pointer);// switiching to the stack pointer of process1 and save the stack pointer for process2
}

void uexit(void)		/* vector index = 5 */
{
  do_switch(NULL,homework_pointer); // switch to parent stack
}

void q3(void)
{
  vector[0] = &print;
  vector[1] = &readline;
  vector[2] = &getarg;
  vector[3] = &yield12;
  vector[4] = &yield21;
  vector[5] = &uexit;
 				 /* load q3prog1 into process 1 and q3prog2 into process 2 */
  FILE *fp1 = NULL;
  void (*functionPointer1)() = NULL;
  long numbytes1 = 0;
  fp1 = fopen("q3prog1", "rb");
  fseek(fp1, 0L, SEEK_END);
  numbytes1 = ftell(fp1);
  rewind(fp1);
  fread(proc1, numbytes1, 1, fp1);
  functionPointer1 = proc1;
  first_pointer = setup_stack(proc1_stack, functionPointer1);// setup the proc1_stack for process 1

  FILE *fp2 = NULL;
  void (*functionPointer2)() = NULL;
  long numbytes2 = 0;
  fp2 = fopen("q3prog2", "rb");
  fseek(fp2, 0L, SEEK_END);
  numbytes2 = ftell(fp2);
  rewind(fp2);
  fread(proc2, numbytes2, 1, fp2);
  functionPointer2 = proc2;
  second_pointer = setup_stack(proc2_stack, functionPointer2); // setup the proc2_stack for process 2 
	
  do_switch(&homework_pointer, first_pointer);// switching to the stack pointer of process 1 and save the homework (main program) stack pointer



}


/***********************************************/
/*********** Your code ends here ***************/
/***********************************************/
