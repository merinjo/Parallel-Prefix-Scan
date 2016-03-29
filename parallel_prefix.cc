#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "carbon_user.h"

void* threadMain(void* arg);
int input_size = 1<<21;
int *input;
int *results;
int num_threads = 64;
pthread_barrier_t barrier; 

int main(int argc, char* argv[]) {
   printf("Beginning prefix scan for data size: %d\n", input_size);

   input = (int*) malloc(input_size * sizeof(int));
   for(int i = 0; i < input_size; i = i + 4){
      input[i] = 1;
      input[i+1] = 1;
      input[i+2] = 1;
      input[i+3] = 1;
   }
   results = (int*) malloc(num_threads * sizeof(int));

   int thread_args[num_threads];
   for(int i = 0; i < num_threads; i++)
   {
      thread_args[i] = i;
   }

   pthread_barrier_init(&barrier, NULL, num_threads);

   CarbonEnableModels();

   pthread_t thread_handles[num_threads];
   for (int i = 1; i < num_threads; i++)
   {
       int ret = pthread_create(&thread_handles[i], NULL, threadMain, (void*) &thread_args[i]);
       if (ret != 0)
       {
           fprintf(stderr, "ERROR spawning thread %i\n", i);
           exit(EXIT_FAILURE);
       }
                              }
    threadMain((void*) &thread_args[0]);

    #ifdef DEBUG
      fprintf(stderr, "Created Threads.\n");
    #endif

    for (int i = 1; i < num_threads; i++)
    {
       pthread_join(thread_handles[i], NULL);
    }

  
   CarbonDisableModels();

   printf("Final sum is input[%d]=%d\n", (input_size-1), input[input_size - 1]);
   return 0;
}

void* threadMain(void* arg)
{
   int* args = (int*) arg;
   int tid = *args;
   int blocks = input_size/num_threads;
   int stride = 1;
   
   for(int i = (tid*blocks)+1; i < (tid*blocks)+blocks; i++)
   {
      input[i] = input[i] + input[i-1];
   }

   results[tid] = input[(tid*blocks)+ blocks - 1];

   pthread_barrier_wait(&barrier);

   while(stride <= num_threads/2)
   {
      int index = (tid+1)*stride*2 - 1;
      if(index < num_threads)
         results[index] += results[index-stride];
      stride = stride * 2;

      pthread_barrier_wait(&barrier);
   }

   //stride = num_threads/2;
    while(stride > 0)
   {
      int index = (tid+1)*stride*2 - 1 ;
      if( index+stride < num_threads)
         results[index+stride] += results[index];
      stride = stride/2;

      pthread_barrier_wait(&barrier);
  }

    if(tid != 0)
    {
       for(int i = tid*blocks; i < (tid*blocks)+blocks; i++ )
          input[i] = input[i] + results[tid -1];
    }

 
}











