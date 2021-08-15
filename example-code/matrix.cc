
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

const int K = 1000;
const int M = 1000000;

extern "C" {
  int ROI_START(int argv) { return 0; }
  int ROI_END(int argv) { return 0; }
}


int main (int argc, char **argv) {
  if (argc != 3) {
    printf("usage : ./matrix <size of matrix in MB> <tile size of matrix in MB>\n");
    printf("this will generate a square matrix with the denoted size\n");
    printf("then it will tranverse a matrix tile of the deonted size\n");
    exit(0);
  }

  int mat_sz = atoi(argv[1]) * M;
  int mat_elements = mat_sz / sizeof(int);
  int mat_w = (int)sqrt(mat_elements);

  int tile_sz = atoi(argv[2]) *  M;
  int tile_elements = tile_sz / sizeof(int);
  int tile_w = (int)sqrt(tile_elements);

  /* matrix_c<int> *mat = new matrix_c<int>(mat_w, mat_w); */

  int *mat = (int *)calloc(mat_w * mat_w, sizeof(int));

  int x_start = 0;
  int y_start = 0;
  int x_end = tile_w;
  int y_end = tile_w;

  printf("starting matrix traversal\n");
  ROI_START(1);
  for (int y = y_start; y < y_end; y++) {
    for (int x = x_start; x < x_end; x++) {
      mat[mat_w * x + y]++;
    }
  }
  ROI_END(1);
  printf("finished matrix traversal\n");

  return 0;
}
