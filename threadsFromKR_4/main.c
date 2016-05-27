#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
 
#define NAME_OFFSET 7 * sizeof(char)
 
struct data {
      int num;
      double product;
};
 
size_t dim;
int numThreads;
 
struct data *parts;
 
pthread_mutex_t fileBlock;
pthread_t *threads;
 
FILE *input;
 
void *fthread(void *arg_p)
{
      double a1, a2;
      int error;
      struct data *arg = (struct data*) arg_p;
      error = pthread_mutex_lock(&fileBlock);
      if (error)
      {
            printf("pthread_mutex_lock error: %d\n", error);
            exit(3);
      }

      error = fseek(input, NAME_OFFSET + sizeof(int) + (arg->num) * sizeof(double), SEEK_SET);
      if (error == -1)
      {
            perror("fseek");
            exit(5);
      }
      error = fread(&a1, sizeof(double), 1, input);
      if (error == -1)
      {
            perror("read");
            exit(4);
      }

      error = fseek(input, NAME_OFFSET + sizeof(int) + dim * sizeof(double) + (arg->num) * sizeof(double), SEEK_SET);
      if (error == -1)
      {
            perror("fseek");
            exit(5);
      }
      error = fread(&a2, sizeof(double), 1, input);
      if (error == -1)
      {
            perror("read");
            exit(4);
      }
      error = pthread_mutex_unlock(&fileBlock);
      if (error)
      {
            printf("pthread_mutex_unlock error: %d\n", error);
            exit(3);
      }

      arg->product = a1 * a2;
      return NULL;
}
 
int main(int argc, char **argv)
{
      int i;
      double ans;
      int left;
      int iteration;
      int error;

      if (argc < 3)
      {
         printf("bad args\n");
         exit(1);
      }

      input = fopen(argv[1], "r+");
      if (input == NULL)
      {
            perror("fopen");
            exit(5);
      }

      numThreads = atoi(argv[2]) / 2;
      if (numThreads == 0)
      {
            printf("0 threads serious?\n");
            exit(1);
      }

      error = fseek(input, NAME_OFFSET, SEEK_SET);
      if (error == -1)
      {
            perror("fseek");
            exit(5);
      }

      error = fread(&dim, sizeof(int), 1, input);
      if (error == -1)
      {
            perror("read");
            exit(4);
      }

      pthread_mutex_init(&fileBlock, NULL);
      threads = malloc(sizeof(pthread_t) * numThreads);
      parts = malloc(sizeof(struct data) * numThreads);

      iteration = 0;
      ans = 0;
      left = dim;
      while (left > 0)
      {
            for (i = 0; i < numThreads && left > 0; i++, left--)
            {
                  parts[i].num = iteration * numThreads + i;
                  parts[i].product = 0;
                  pthread_create(&threads[i], NULL, fthread, &parts[i]);
            }

            for (i = 0; i < numThreads; i++)
            {
                  pthread_join(threads[i], NULL);
                  ans += parts[i].product;
            }

            iteration++;
      }

      error = fseek(input, 0, SEEK_END);
      if (error == -1)
      {
            perror("fseek");
            exit(5);
      }
      error = fwrite(&ans, sizeof(double), 1, input);
      if (error == -1)
      {
            perror("write");
            exit(4);
      }
    return 0;
}
