#include <stdint.h>
#include <stdio.h>  /* fopen, fscanf, fclose */
#include <stdlib.h> /* exit, malloc          */
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int32_t* get_matrix(const char* filename, size_t* width) {
  int32_t value, *matrix;
  FILE* fp;

  /* Open the file in text/translated mode */
  fp = fopen(filename, "rt");
  if (!fp) {
    printf("Can't open file: %s\n", filename);
    exit(-1);
  }

  /* Read the width and allocate the matrix */
  {
    int32_t iwidth = 0;
    fscanf(fp, "%d", &iwidth);
    *width = (size_t)iwidth;
  }

  matrix = malloc(*width * *width * sizeof(int32_t));

  if (!matrix) {
    printf("Can't malloc matrix\n");
    fclose(fp);
    exit(-2);
  }

  /* Read the vaules and put in the matrix */
  while (!feof(fp)) {
    int32_t result = fscanf(fp, "%d", &value);
    if (result == -1) {
      break;
    }
    *matrix++ = value;
  }
  fclose(fp);

  /* Return the address of the matrix */
  return matrix - (*width * *width);
}

void print_matrix(const int32_t* const matrix, const size_t width) {
  const size_t size = width * width;

  for (size_t i = 0; i < size; i++) {
    printf("%8i", matrix[i]);
    if ((i + 1) % width == 0) {
      printf("\n");
    }
  }

  printf("\n");
}

int32_t main(const int32_t argc, const char* const argv[]) {
  if (argc < 3) {
    printf("Insufficient parameters supplied\n");
    return -1;
  }

  const char* const child_program = argv[2];

  /* read in matrix values from file */
  /* don't forget to free the memory */
  size_t width; /* width of the matrix   */
  int32_t* matrix = get_matrix(argv[1], &width);

  /* print the matrix */
  print_matrix(matrix, width);

  // NTQ=
  // 52

  const key_t shared_memory_id =
    shmget(53, sizeof(int32_t) * (2 * width * width + 1), IPC_CREAT | 0644);

  if (shared_memory_id == -1) {
    perror("Failed to create shared memory");
    free(matrix);
    return 1;
  }

  int32_t* const shared_buffer = shmat(shared_memory_id, NULL, 0);

  if (shared_buffer == (void*)-1) {
    perror("Failed to get shared memory");
    free(matrix);
    return 1;
  }

  shared_buffer[0] = (int32_t)width;
  // copy over the read matrix into shared memory
  memcpy(&shared_buffer[1], matrix, sizeof(int32_t) * width * width);
  // set the output matrix all to 0
  memset(&shared_buffer[1 + width * width], 0, sizeof(int32_t) * width * width);

  // dont need the original matrix
  free(matrix);

  /* Fork a child for each matrix entry       */
  /* May need an array to hold the child pids */
  pid_t* const child_pids = calloc(width * width, sizeof(pid_t));

  for (size_t row = 0; row < width; row++) {
    for (size_t column = 0; column < width; column++) {

      const pid_t process_id = fork();

      if (process_id < 0) {
        perror("Failed to fork process");

        if (shmdt(shared_buffer) == -1) {
          perror("Failed to detach shared memory");
        }
        if (shmctl(shared_memory_id, IPC_RMID, NULL) == -1) {
          perror("Failed to destory shared memory");
        }
        free(child_pids);
        return 1;
      }

      if (process_id != 0) {
        continue;
      }

      char shared_id_str[16] = {'\0'};
      sprintf(shared_id_str, "%d", shared_memory_id);

      char child_number_str[16] = {'\0'};
      sprintf(child_number_str, "%d", (uint32_t)(width * row + column));

      char row_str[16] = {'\0'};
      sprintf(row_str, "%d", (uint32_t)row);

      char column_str[16] = {'\0'};
      sprintf(column_str, "%d", (uint32_t)column);

      execl(
        child_program,
        child_program,
        shared_id_str,
        child_number_str,
        row_str,
        column_str,
        NULL
      );
    }
  }

  // wait for children
  for (size_t i = 0; i < width * width; i++) {
    int32_t status = 0;

    if (waitpid(child_pids[i], &status, 0) == -1) {
      perror("Failed to wait for process");

      if (shmdt(shared_buffer) == -1) {
        perror("Failed to detach shared memory");
      }
      if (shmctl(shared_memory_id, IPC_RMID, NULL) == -1) {
        perror("Failed to destory shared memory");
      }
      free(child_pids);
      return 1;
    }
  }

  // free child list
  free(child_pids);

  /* print matrix from shared buffer */
  print_matrix(shared_buffer + 1 + width * width, width);

  /* cleanup */
  if (shmdt(shared_buffer) == -1) {
    perror("Failed to detach shared memory");
    return 1;
  }

  if (shmctl(shared_memory_id, IPC_RMID, NULL) == -1) {
    perror("Failed to destory shared memory");
    return 1;
  }

  return 0;
}
