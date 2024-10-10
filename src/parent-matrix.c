#include <stdio.h> /* fopen, fscanf, fclose */
#include <stdlib.h> /* exit, malloc          */
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int *get_matrix(const char *filename, int *width) {
  int value, *matrix;
  FILE *fp;

  /* Open the file in text/translated mode */
  fp = fopen(filename, "rt");
  if (!fp) {
    printf("Can't open file: %s\n", filename);
    exit(-1);
  }

  /* Read the width and allocate the matrix */
  fscanf(fp, "%d", width);
  matrix = malloc(*width * *width * sizeof(int));
  if (!matrix) {
    printf("Can't malloc matrix\n");
    fclose(fp);
    exit(-2);
  }

  /* Read the vaules and put in the matrix */
  while (!feof(fp)) {
    int result = fscanf(fp, "%d", &value);
    if (result == -1) break;
    *matrix++ = value;
  }
  fclose(fp);

  /* Return the address of the matrix */
  return matrix - (*width * *width);
}

void print_matrix(int *matrix, int width) {
  int i, size = width * width;
  for (i = 0; i < size; i++) {
    printf("%8i", matrix[i]);
    if ((i + 1) % width == 0) printf("\n");
  }
  printf("\n");
}

int main(int argc, const char **argv) {
  int width; /* width of the matrix   */
  int *matrix; /* the matrix read in    */

  if (argc < 3) {
    printf("Insufficient parameters supplied\n");
    return -1;
  }

  /* read in matrix values from file */
  /* don't forget to free the memory */
  matrix = get_matrix(argv[1], &width);

  /* print the matrix */
  print_matrix(matrix, width);

  const char *program = argv[2];

  int id = shmget(123, sizeof(int) * (width * width + 1), SHM_W);

  /* Fork a child for each matrix entry       */
  /* May need an array to hold the child pids */
  for (int r = 0; r < width; r++) {
    for (int c = 0; c < width; c++) {

      execl(program, "", "", NULL);

      /* Do forking and stuff */
    }
  }

  /* wait for children */


  /* print matrix from shared buffer */
  print_matrix(buffer + 1 + width * width, width);

  /* cleanup */
  free(matrix);

  return 0;
}
