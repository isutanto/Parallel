/*
TSP v1.0: This code uses the Monte Carlo method to determine progressively
better solutions for the travelling salesman problem.

Copyright (c) 2013, Texas State University-San Marcos. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted for academic, research, experimental, or personal use provided
that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   * Neither the name of Texas State University-San Marcos nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

For all other uses, please contact the Office for Commercialization and Industry
Relations at Texas State University-San Marcos <http://www.txstate.edu/ocir/>.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Author: Martin Burtscher <burtscher@txstate.edu>
*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>
#include <mpi.h>

#define MAXCITIES 1296

static int read_input(char *filename, float *posx, float *posy)
{
  int cnt;
  int i1, cities;
  float i2, i3;
  FILE *f;

  /* open input text file */
  f = fopen(filename, "r+t");
  if (f == NULL) {fprintf(stderr, "could not open file %s\n", filename); exit(-1);}

  /* read the number of cities from first line */
  cities = -1;
  fscanf(f, "%d\n", &cities);
  if ((cities < 1) || (cities >= MAXCITIES)) {fprintf(stderr, "cities out of range\n"); exit(-1);}

  /* read in the cities' coordinates */
  cnt = 0;
  while (fscanf(f, "%d %f %f\n", &i1, &i2, &i3)) {
    posx[cnt] = i2;
    posy[cnt] = i3;
    cnt++;
    if (cnt > cities) {fprintf(stderr, "input too long\n"); exit(-1);}
    if (cnt != i1) {fprintf(stderr, "input line mismatch\n"); exit(-1);}
  }
  if (cnt != cities) {fprintf(stderr, "wrong number of cities read\n"); exit(-1);}

  /* return the number of cities */
  fclose(f);
  return cities;
}

int main(int argc, char *argv[])
{
  int i, j, cities, iter, samples, from, to, len, length;
  float dx, dy;
  float posx[MAXCITIES], posy[MAXCITIES];
  unsigned short tmp;
  unsigned short tour[MAXCITIES + 1];
  struct timeval start, end;
  
  float local_time, total_time;

  total_time = 0;

  /*Initializing MPI*/
  int comm_sz;   // Number of process
  int my_rank;   // Process rank

  MPI_Init(NULL,NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0)
    printf("TSP w/ MPI v1.0\n");

  /* check command line */
  if (argc != 3) 
    {
      if (my_rank == 0)
	fprintf(stderr, "usage: %s input_file_name number_of_samples\n", argv[0]); 
      MPI_Finalize();
      exit(-1);
    }
  
  if(my_rank ==0)
    {					       
      cities = read_input(argv[1], posx, posy);
      samples = atoi(argv[2]);

      MPI_Bcast(posx, MAXCITIES ,MPI_FLOAT ,0,MPI_COMM_WORLD);
      MPI_Bcast(posy, MAXCITIES,MPI_FLOAT,0,MPI_COMM_WORLD);
      MPI_Bcast(&cities,1,MPI_INT,0, MPI_COMM_WORLD);
    }

  if (samples < 1) 
    {
      if(my_rank ==0)
	fprintf(stderr, "number of samples must be at least 1\n"); 
      MPI_Finalize();
      exit(-1);
    }
  
  if(my_rank == 0)
    printf("%d cities and %d samples (%s)\n", cities, samples, argv[1]);

  /* initialize */
  tour[cities] = 0;
  length = INT_MAX;
  int final_length = 0;

  MPI_Barrier(MPI_COMM_WORLD);

  /* start time */
  gettimeofday(&start, NULL);

  /* iterate number of sample times with cyclic distribution */
  for (iter = my_rank+1; iter <= samples; iter+=comm_sz)//1; iter <= samples; iter++)
 {
    /* generate a random tour */
    srand(iter);
    for (i = 1; i < cities; i++) tour[i] = i;
    for (i = 1; i < cities; i++) {
      j = rand() % (cities - 1) + 1;
      tmp = tour[i];
      tour[i] = tour[j];
      tour[j] = tmp;
    }

    /* compute tour length */
    len = 0;
    from = 0;
    for (i = 1; i <= cities; i++) {
      to = tour[i];
      dx = posx[to] - posx[from];
      dy = posy[to] - posy[from];
      len += (int)(sqrtf(dx * dx + dy * dy) + 0.5f);
      from = to;
    }

    /* check if new shortest tour */
    if (length > len) {
      length = len;
      //if (my_rank == 0)
	//printf("iteration %d: %d\n", iter, len);
    }
  }

  /* global minimum for length */
  MPI_Reduce(&length, &final_length,1,MPI_INT,MPI_MIN,0,MPI_COMM_WORLD);

  /* end time */
  gettimeofday(&end, NULL);
  
  local_time = end.tv_sec + end.tv_usec / 1000000 - start.tv_sec - start.tv_usec / 1000000;

  MPI_Allreduce(&local_time,&total_time,1,MPI_FLOAT,MPI_SUM, MPI_COMM_WORLD);
  //printf("runtime: %.4lf s\n",end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0);






  /* output result */
  if(my_rank == 0)
    {
      printf("runtime: %.4lf s\n",total_time);
      printf("length of shortest found tour: %d\n\n",final_length);
    }

  MPI_Finalize();
  return 0;
}

