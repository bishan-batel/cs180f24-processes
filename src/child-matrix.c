#include <stdint.h>
#include <stdio.h>  /* fopen, fscanf, fclose */
#include <stdlib.h> /* exit, malloc          */
#include <string.h>
#include <sys/shm.h>

int32_t matrix_get(
  const int32_t* values,
  const size_t width,
  const size_t column,
  const size_t row
) {
  return values[row * width + column];
}

void matrix_set(
  int32_t* values,
  const size_t width,
  const size_t column,
  const size_t row,
  const int32_t v
) {
  values[row * width + column] = v;
}

int32_t child_matrix(const size_t argc, const char* argv[]) {
  if (argc < 5) {
    perror("Incorrect argument count");
    return 1; // failure
  }

  int32_t shared_mem_id = 0;
  uint32_t child_index = 0;
  uint32_t row = 0, column = 0;

  int32_t err = 0;
  err += sscanf(argv[1], "%d", &shared_mem_id);
  err += sscanf(argv[2], "%u", &child_index);
  err += sscanf(argv[3], "%u", &row);
  err += sscanf(argv[4], "%u", &column);

  if (err < 4) {
    perror("Failed to parse arugments");
    return 1;
  }

  int32_t* const shared_memory = shmat(shared_mem_id, NULL, 0);

  if (shared_memory == NULL) {
    perror("Failed to get shared memory");
    return 1;
  }

  const size_t width = (size_t)shared_memory[0];

  const int32_t* const input_matrix = &shared_memory[1];

  int32_t* const output_matrix = &shared_memory[1 + width * width];

  int32_t sum = 0;

  for (size_t i = 0; i < width; i++) {
    sum += matrix_get(input_matrix, width, column, i) *
           matrix_get(input_matrix, width, i, row);
  }

  matrix_set(output_matrix, width, column, row, sum);

  return 0;
}

int main(int argc, const char* argv[]) {
  return child_matrix(argc, argv);
}
