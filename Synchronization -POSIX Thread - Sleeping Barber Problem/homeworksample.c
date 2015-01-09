/* 
 * file:        homework.c
 * description: Skeleton code for CS 5600 Homework 2
 *
 * Peter Desnoyers, Northeastern CCIS, 2011
 * $Id: homework.c 530 2012-01-31 19:55:02Z pjd $
 */

#include <stdio.h>
#include <stdlib.h>
#include "hw2.h"

/********** YOUR CODE STARTS HERE ******************/

/*
 * Here's how you can initialize global mutex and cond variables
 */
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t k = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t C = PTHREAD_COND_INITIALIZER;

int customer_count = 0;
int chair_count = 5;
int flag_sleeping=0;
int max_customer_count = 10;
int total = 10;
//int first = 0;
pthread_cond_t call_customer=PTHREAD_COND_INITIALIZER;
pthread_cond_t Barber = PTHREAD_COND_INITIALIZER;
pthread_cond_t Done = PTHREAD_COND_INITIALIZER;
pthread_cond_t Wait =  PTHREAD_COND_INITIALIZER;
pthread_cond_t start_haircut=PTHREAD_COND_INITIALIZER;
pthread_cond_t end_haircut =PTHREAD_COND_INITIALIZER;
pthread_cond_t wake_barber=PTHREAD_COND_INITIALIZER;
pthread_cond_t check_remaining = PTHREAD_COND_INITIALIZER;
pthread_cond_t call_barber = PTHREAD_COND_INITIALIZER;

/* the barber method
 */
void barber(void)
{
  pthread_mutex_lock(&m);
    
  while(1)
    {

      //    if(first==0)
      //{
      //  pthread_cond_wait(&Barber,&m);
      //  flag_sleeping = 0;
      //}
      if(customer_count == 0)
	{
	  flag_sleeping = 1;
	  printf("DEBUG: %f barber goes to sleep\n", timestamp());
	  // pthread_cond_signal(&call_barber);
	  pthread_cond_wait(&Barber,&m);
	  flag_sleeping = 0;
	}

      sleep_exp(1.2,&m);
      pthread_cond_signal(&Done);

	
    }
  pthread_mutex_unlock(&m);
}

/* the customer method
 */
void customer(int customer_num)
{
  pthread_mutex_lock(&m);
  sleep_exp(0.1,&m);
   while(1)
     {
      
      if (customer_count >= chair_count)
	{
	  printf("DEBUG: %f customer %d enters shop\n", timestamp(), customer_num);
	  printf("DEBUG: %f customer %d leaves shop\n",timestamp(),customer_num);
	}

      else
	{
	   customer_count++;
	  printf("DEBUG: %f customer %d enters shop\n", timestamp(), customer_num);
	 
	  

	  if (customer_count > 1)
	    {
	      //first=1;
	      pthread_cond_wait(&Wait, &m);
	    }
	  // if (customer_count == 0)
	  // {
	     //pthread_mutex_lock(&k);
	  //printf("Hello0000000000000000000000\n");
	  // pthread_cond_wait(&call_barber, &m);
	      //pthread_mutex_lock(&k);
	  // }
	  if (flag_sleeping == 1)
	    {
	      //first=1;
	      pthread_cond_signal(&Barber);
	      printf("DEBUG: %f barber wakes up\n", timestamp());
	    }
	  printf("DEBUG: %f customer %d starts haircut\n",timestamp(),customer_num); 
	  pthread_cond_wait(&Done, &m);
	  printf("DEBUG: %f customer %d leaves shop\n",timestamp(),customer_num);
	
	  customer_count--;
	  pthread_cond_signal(&Wait);
	}
        sleep_exp(10.0,&m);
       }
  pthread_mutex_unlock(&m);
}

/* Threads which call these methods. Note that the pthread create
 * function allows you to pass a single void* pointer value to each
 * thread you create; we actually pass an integer (the customer number)
 * as that argument instead, using a "cast" to pretend it's a pointer.
 */

/* the customer thread function - create 10 threads, each of which calls
 * this function with its customer number 0..9
 */
void *customer_thread(void *context) 
{
  int customer_num = (int)context;
  customer(customer_num);
  return 0;
}

/*  barber thread
 */
void *barber_thread(void *context)
{
  
  barber(); /* never returns */
  return 0;
}

void q2(void)
{
  /* to create a thread:
     pthread_t t; 
     pthread_create(&t, NULL, function, argument);
     note that the value of 't' won't be used in this homework
	  
  */
   
  pthread_t t_customer[10];
  pthread_t t_barber;
  int i;
  pthread_create(&t_barber,NULL,barber_thread, NULL);
 
  for(i=0;i<10;i++)
    {
      pthread_create(&t_customer[i],NULL,customer_thread,(void* )i);
    }	  
  // 
  
   
  wait_until_done();
 
}

/* For question 3 you need to measure the following statistics:
 *
 * 1. fraction of  customer visits result in turning away due to a full shop 
 *    (calculate this one yourself - count total customers, those turned away)
 * 2. average time spent in the shop (including haircut) by a customer 
 *     *** who does not find a full shop ***. (timer)
 * 3. average number of customers in the shop (counter)
 * 4. fraction of time someone is sitting in the barber's chair (counter)
 *
 * The stat_* functions (counter, timer) are described in the PDF. 
 */

void q3(void)
{
  /* your code goes here */
}
