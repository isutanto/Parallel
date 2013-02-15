/*
RankSort v1.0: This code uses the rank-sort algorithm to sort an array of
integers.

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
#include <sys/time.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
  int i, j, cnt, val, size = 0;
  int *a, *b;
  struct timeval start, end;

  /*Initializing MPI*/
  int comm_sz;   // Number of process
  int my_rank;   // Process rank

  MPI_Init(NULL,NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


  if (my_rank == 0)
    printf("RankSort MPI OuterLoop\n");

  /* check command line */
  if (argc != 2) 
  {
    if(my_rank == 0)
      fprintf(stderr, "usage: %s number_of_elements\n", argv[0]); 
    MPI_Finalize();
    exit(-1);
  }
  size = atoi(argv[1]);
  if (size < 1) 
  {
    if(my_rank == 0)
      fprintf(stderr, "number of elements must be at least 1\n"); 
    MPI_Finalize();
    exit(-1);
  }

  /* allocate arrays */
  a = (int *)malloc(size * sizeof(int));
  b = (int *)malloc(size * sizeof(int));
  if ((a == NULL) || (b == NULL)) 
  {
    if(my_rank == 0)
      fprintf(stderr, "could not allocate arrays\n"); 
    MPI_Finalize();
    exit(-1);
  }
  
  
  /* generate input */
  for (i = 0; i < size; i++) a[i] = i;
  if(my_rank == 0)
    printf("Message from process %d sorting %d values\n", my_rank, size);
  else
    printf("Process %d not doing anything\n",my_rank);


  /* Barrier */
  MPI_Barrier(MPI_COMM_WORLD);

  /* start time */
  gettimeofday(&start, NULL);

  /* sort the values */
  for (i = 0; i < size; i++) {
    cnt = 0;
    val = a[i];
    for (j = 0; j < size; j++) {
      if (val > a[j]) cnt++;
    }
    b[cnt] = val;
  }

  /* end time */
  gettimeofday(&end, NULL);
  if(my_rank == 0)
    printf("runtime: %.4lf s\n", end.tv_sec + end.tv_usec / 1000000.0 - start.tv_sec - start.tv_usec / 1000000.0);

  /* verify result */
  i = 1;
  if(my_rank == 0)
  {
    while ((i < size) && (b[i - 1] < b[i])) i++;
    if (i < size) printf("NOT sorted\n\n"); else printf("sorted\n\n");
  }

  /*MPI_Finalize*/
  MPI_Finalize();

  free(a);
  free(b);
  return 0;
}

