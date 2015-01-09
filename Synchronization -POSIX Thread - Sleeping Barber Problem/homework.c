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

int customerCounter = 0;
int chair_count = 5;
int sleepCheckFlag=0;
float total = 0;
float return_back = 0;
int flagToCheckCustomerCounter=1;

void *counter_total=NULL;
void *counter_barber=NULL;
void *timer_total=NULL;

double val_3 = 0;
double time_2 = 0;
double val_4 = 0;

pthread_cond_t call_customer=PTHREAD_COND_INITIALIZER;
pthread_cond_t Barber = PTHREAD_COND_INITIALIZER;
pthread_cond_t haircutFinished = PTHREAD_COND_INITIALIZER;
pthread_cond_t waitingCustomer =  PTHREAD_COND_INITIALIZER;

/*
  the barber method
*/

void barber(void)
{
  pthread_mutex_lock(&m);
    
  while(1)
    {
      if(customerCounter == 0)
	{
	  if (flagToCheckCustomerCounter == 0)
	    {

	      sleepCheckFlag = 1;
	      pthread_cond_wait(&Barber,&m);
	      
	      sleepCheckFlag = 0;
	    }
	  else
	    {
	      sleepCheckFlag = 1;
	      printf("DEBUG: %f barber goes to sleep\n", timestamp());
	      pthread_cond_wait(&Barber,&m);
	      sleepCheckFlag = 0;
	    }
	}

      sleep_exp(1.2,&m);
      pthread_cond_signal(&haircutFinished);
      customerCounter--;     
	
    }
  pthread_mutex_unlock(&m);
}

/*
  the customer method
*/

void customer(int customerId)
{
 
  pthread_mutex_lock(&m);
  sleep_exp(0.1,&m);
  while(1)
    {
      total++;
      
      if (customerCounter >= chair_count)
	{
	  
	  printf("DEBUG: %f customer %d enters shop\n", timestamp(), customerId);
	  return_back++;
	  printf("DEBUG: %f customer %d leaves shop\n",timestamp(),customerId);
	}

      else
	{
	  stat_count_incr(counter_total);
	  stat_timer_start(timer_total);
	  customerCounter++;
	  
	  
	  if (flagToCheckCustomerCounter == 0 && customerCounter == 1)
	    {
	      printf("DEBUG: %f barber goes to sleep\n", timestamp());
	      printf("DEBUG: %f customer %d enters shop\n", timestamp(), customerId);
	    }
	  else
	    {
	      printf("DEBUG: %f customer %d enters shop\n", timestamp(), customerId);
	    }
	  if (customerCounter > 1)
	    {
	      pthread_cond_wait(&waitingCustomer, &m);
	       
	    }

	  if (sleepCheckFlag == 1)
	    {
		
	      pthread_cond_signal(&Barber);
	      printf("DEBUG: %f barber wakes up\n", timestamp());
		
	    }
	  printf("DEBUG: %f customer %d starts haircut\n",timestamp(),customerId);
	  
	  if (customerCounter == 1)
	    {
	      flagToCheckCustomerCounter = 0;
	    }
	  else
	    {
	      flagToCheckCustomerCounter = 1;
	    }
	  stat_count_incr(counter_barber);

	  pthread_cond_wait(&haircutFinished, &m);
	 
	  printf("DEBUG: %f customer %d leaves shop\n",timestamp(),customerId);
	  stat_count_decr(counter_barber);
	  stat_count_decr(counter_total);
	  stat_timer_stop(timer_total);
	  pthread_cond_signal(&waitingCustomer);
	 
	}
      
      sleep_exp(10.0,&m);
      
      val_3 = stat_count_mean(counter_total);
      time_2 = stat_timer_mean(timer_total);
      val_4 = stat_count_mean(counter_barber);
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
  int customerId = (int)context;
  customer(customerId);
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
  pthread_t t_customer[10];
  pthread_t t_barber;
  int i;
  pthread_create(&t_barber,NULL,barber_thread, NULL);
  for(i=0;i<10;i++)
    {
      pthread_create(&t_customer[i],NULL,customer_thread,(void* )i);
    }	     
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

  counter_total = stat_counter();
  counter_barber = stat_counter();
  timer_total = stat_timer();
  q2();

  /* your code goes here */
  float fract_back;
  fract_back = (float)(return_back/total);
  printf("\nq3.1) fraction of customer visits result in truning away due to full shop  = %f\n\n",fract_back);
  printf("q3.2) average time spent in the shop  = %f\n\n",time_2);
  printf("q3.3) average number of customers in the shop  = %f\n\n",val_3);

  printf("q3.4) fraction of time someone sitting on barber chair  = %f\n\n",val_4);



}
