#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <string>
#include <unordered_set>
#include <ctime>
#include <algorithm>
#include <mpi.h>
#include <stdio.h>

using namespace std;

const int N = 9;
int INIT_POP_SIZE = 2000;
double SELECTION_RATE = 0.15;
double MUTATION_RATE = 0.95;

double fitness(const vector<vector<int> >&);
bool mutate(vector<vector<int> >&, double, vector<vector<int> >&);
vector<vector<int> > crossover(vector<vector<int> >&, vector<vector<int> >&);
int nextRowUnused(vector<vector<int> >&, int);
int nextColUnused(vector<vector<int> >&, int);
int nextBlockUnused(vector<vector<int> > &, int, int);
void swap(vector<vector<int> > &, int, int, int, int);
vector<vector<int> > randomFill(vector<vector<int> >&);
void printSudoku(vector<vector<int> > &);

int main(int argc, char **argv) {
  srand(time(NULL));
  int processors_size, processor_id;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &processors_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &processor_id);

  string string_board = argv[argc - 4];
  INIT_POP_SIZE = atoi(argv[argc - 3]);
  SELECTION_RATE = atof(argv[argc - 2]);
  MUTATION_RATE = atof(argv[argc - 1]);
  string_board.erase(remove(string_board.begin(), string_board.end(), ' '), string_board.end());
  vector<vector<int> > sudoku;
  int index = 0;
  int empty_spots = 0;
  for (int i = 0; i < N; i++) {
      vector<int> new_row;
      for (int j = 0; j < N; j++) {
        if (string_board[index] == '0') { empty_spots++; }
        new_row.push_back(string_board[index++] - '0');
      }
      sudoku.push_back(new_row);
  }
  int small_processor_population_size = INIT_POP_SIZE / (processors_size - 1);
  int small_top_population_size = small_processor_population_size * SELECTION_RATE;
  int large_top_population_size = small_top_population_size * (processors_size - 1);

  vector<vector<int> > spots;
  for (int i = 0; i < N; i++) {
    vector<int> row;
    for (int j = 0; j < N; j++) {
      if (sudoku[i][j] == 0) {
        row.push_back(j);
      }
    }
    spots.push_back(row);
  }

  vector<vector<vector<int> > > population;
  if (processor_id != 0) {
    // Generate (total / processors_size) populations for each worker processor
    for (int i = 0; i < small_processor_population_size; i++) {
      population.push_back(randomFill(sudoku));
    }
  } else {
    cout << "==> Processors Size: " << processors_size << endl;
    cout << "==> Init Board: " << string_board << " | Empty Spots: " << empty_spots << endl;
    cout << "==> Population size: " << INIT_POP_SIZE << " | Selection Rate: " << SELECTION_RATE << " | Mutation Rate: " << MUTATION_RATE << endl;
  }

  int generation = 1;
  timespec startTime, endTime;
  clock_gettime(CLOCK_REALTIME, &startTime);
  int flag = 0;
  double prev_best_fitness = 0.0;
  int same_iteration = 0;
  while (true) {
    // Sort (total / processors_size) population in each processor
    int top_population[small_top_population_size][N][N];
    if (processor_id != 0) {
      sort(population.begin(), population.end(), [](const vector<vector<int> > &cand1, const vector<vector<int> > &cand2) {
        return fitness(cand1) > fitness(cand2);
      });
      for(int i = 0; i < small_top_population_size; i++) {
        for (int row = 0; row < N; row++) {
          for (int col = 0; col < N; col++) {
            top_population[i][row][col] = population[i][row][col];
          }
        }
      }
      // Send the best (total * 0.15 / processors_size) population from each processor to master
      MPI_Send(top_population, small_top_population_size * N * N, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    if (processor_id == 0) {
      for (int i = 1; i < processors_size; i++) {
        // Receive local population from each worker processor
        MPI_Recv(top_population, small_top_population_size * N * N, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (int j = 0; j < small_top_population_size; j++) {
          vector<vector<int> > new_matrix;
          for (int row = 0; row < N; row++) {
            vector<int> new_row;
            for (int col = 0; col < N; col++) {
              new_row.push_back(top_population[j][row][col]);
            }
            new_matrix.push_back(new_row);
          }
          population.push_back(new_matrix);
        }
      }
      sort(population.begin(), population.end(), [](const vector<vector<int> > &cand1, const vector<vector<int> > &cand2) {
        return fitness(cand1) > fitness(cand2);
      });
      for (int i = 0; i < 5; i++) {
        printf("%f, ", fitness(population[i]));
      }
      printf("\n");
      double cur_fitness = fitness(population[0]);
      if (cur_fitness == prev_best_fitness) {
          same_iteration++;
          cout << "Same: " << same_iteration << endl;
      } else {
          prev_best_fitness = cur_fitness;
          same_iteration = 0;
      }
      if (same_iteration == 5) {
          flag = 1;
          for (int i = 1; i < processors_size; i++) {
          MPI_Send(&flag, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
          }
          break;
      }
      if (fitness(population[0]) == 1.0) {
        printf("Sudoku solved at generation %d: \n", generation);
        printSudoku(population[0]);
        flag = 1;
        for (int i = 1; i < processors_size; i++) {
          MPI_Send(&flag, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }
        break;
      }
      for (int i = 1; i < processors_size; i++) {
        MPI_Send(&flag, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
      }
      for (int i = 0; i < small_top_population_size; i++) {
        for (int row = 0; row < N; row++) {
          for (int col = 0; col < N; col++) {
            top_population[i][row][col] = population[i][row][col];
          }
        }
      }
      for (int i = 1; i < processors_size; i++) {
        MPI_Send(top_population, small_top_population_size * N * N, MPI_INT, i, 3, MPI_COMM_WORLD);
      }
    }
    if (processor_id != 0) {
      MPI_Recv(&flag, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      if (flag == 1) break;
      // Recieve the best populations from the master node
      MPI_Recv(top_population, small_top_population_size * N * N, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for (int i = 0; i < small_top_population_size; i++) {
        for (int row = 0; row < N; row++) {
          for (int col = 0; col < N; col++) {
            population[i][row][col] = top_population[i][row][col];
          }
        }
      }
      for (int i = small_top_population_size; i < small_processor_population_size; i++) {
        int i1 = rand() % small_top_population_size, i2 = rand() % small_top_population_size;
        population[i] = crossover(population[i1], population[i2]);
      }
      for (int i = 0; i < small_processor_population_size; i++) {
        mutate(population[i], MUTATION_RATE, spots);
      }
    }
    generation++;
  }
  if (processor_id == 0) {
    clock_gettime(CLOCK_REALTIME, &endTime);
    printf("time used(sec): %f\n\n", endTime.tv_sec - startTime.tv_sec + (endTime.tv_nsec - startTime.tv_nsec) / 1000000000.);
  }
  MPI_Finalize();
}


double fitness(const vector<vector<int> > &sudoku) {
  vector<int> row_count(N, 0);
  vector<int> col_count(N, 0);
  vector<int> block_count(N, 0);
  double row_sum = 0, col_sum = 0, block_sum = 0;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      row_count[sudoku[i][j] - 1]++;
    }
    unordered_set<int> row_count_set(row_count.begin(), row_count.end());
    row_sum += (1.0 / row_count_set.size()) / N;
    vector<int> tmp(N, 0);
    row_count = tmp;
  }

  for (int j = 0; j < N; j++) {
    for (int i = 0; i < N; i++) {
      row_count[sudoku[i][j] - 1]++;
    }
    unordered_set<int> col_count_set(col_count.begin(), col_count.end());
    col_sum += (1.0 / col_count_set.size()) / N;
    vector<int> tmp(N, 0);
    col_count = tmp;
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      for (int k = 0; k < 3; k++) {
        for (int l = 0; l < 3; l++) {
          block_count[sudoku[i * 3 + k][j * 3 + l] - 1]++;
        }
      }
      unordered_set<int> block_count_set(block_count.begin(), block_count.end());
      block_sum += (1.0 / block_count_set.size()) / N;
      vector<int> tmp(N, 0);
      block_count = tmp;
    }
  }

  return (int)row_sum == 1 && (int)col_sum == 1 && (int)block_sum == 1 ? 1.0 : col_sum * block_sum;
}

bool mutate(vector<vector<int> > &sudoku, double mutation_rate, vector<vector<int> > &spots) {
  double random = rand() / (double)RAND_MAX;
  if (random < mutation_rate) {
    for (int i = 0; i < 5000; i++) {
      int row = rand() % N;
      if (spots[row].size() <= 1) {
          return false;
      }
      int col1 = rand() % spots[row].size();
      int col2 = rand() % spots[row].size();
      while (col1 == col2) {
        col1 = rand() % spots[row].size();
        col2 = rand() % spots[row].size();
      }
      swap(sudoku, row, col1, row, col2);
      if ((nextColUnused(sudoku, col1) != -1 && nextColUnused(sudoku, col2) != -1) || (nextBlockUnused(sudoku, row / 3 * 3, col1 / 3 * 3) != -1 && nextBlockUnused(sudoku, row / 3 * 3, col2 / 3 * 3) != -1)) {
        return true;
      } else {
        swap(sudoku, row, col1, row, col2);
      }
    }
  }
  return false;
}
                                        

vector<vector<int> > crossover(vector<vector<int> > &p1, vector<vector<int> > &p2) {
  vector<vector<int> > child;
  int row1 = rand() % N, row2 = rand() % N;
  while (abs(row2 - row1) == 0 || abs(row2 - row1) == N - 1) {
    row1 = rand() % N;
    row2 = rand() % N;
  }
  int r1 = min(row1, row2), r2 = max(row1, row2);
  for (int i = 0; i < N; i++) {
    if (i >= r1 && i <= r2) {
      vector<int> row(p2[i].begin(), p2[i].end());
      child.push_back(row);
    } else {
      vector<int> row(p1[i].begin(), p1[i].end());
      child.push_back(row);
    }
  }
  return child;
}

int nextRowUnused(vector<vector<int> > &sudoku, int row) {
  bool repeat[N] = {0};
  for (int i = 0; i < N; i++) {
    if (repeat[sudoku[row][i]]) {
      return -1;
    }
    repeat[sudoku[row][i] - 1] = true;
  }
  for (int i = 0; i < N; i++) {
    if (!repeat[i]) {
      return i + 1;
    }
  }
  return 0;
}

int nextColUnused(vector<vector<int> > &sudoku, int col) {
  bool repeat[N] = {0};
  for (int i = 0; i < N; i++) {
    if (repeat[sudoku[i][col]]) {
      return -1;
    }
    repeat[sudoku[i][col] - 1] = true;
  }
  for (int i = 0; i < N; i++) {
    if (!repeat[i]) {
      return i + 1;
    }
  }
  return 0;
}

int nextBlockUnused(vector<vector<int> > &sudoku, int row, int col) {
  bool repeat[N] = {0};
  for (int i = row; i < row + 3; i++) {
    for (int j = col; j < col + 3; j++) {
      if (repeat[sudoku[i][j] - 1]) {
        return -1;
      }
      repeat[sudoku[i][j] - 1] = true;
    }
  }

  for (int i = 0; i < N; i++) {
    if (!repeat[i]) {
      return i + 1;
    }
  }
  return 0;
}


void swap(vector<vector<int> > &sudoku, int r1, int c1, int r2, int c2) {
  int tmp = sudoku[r1][c1];
  sudoku[r1][c1] = sudoku[r2][c2];
  sudoku[r2][c2] = tmp;
}

vector<vector<int> > randomFill(vector<vector<int> > &sudoku) {
  vector<vector<int> > count;
  for (int i = 0; i < N; i++) {
    count.push_back(vector<int>({1, 1, 1, 1, 1, 1, 1, 1, 1}));
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (sudoku[i][j] > 0) {
        count[i][sudoku[i][j] - 1]--;
      }
    }
  }

  vector<vector<int> > values;
  for (int i = 0; i < N; i++) {
    vector<int> tmp;
    for (int j = 0; j < N; j++) {
      if (count[i][j]) {
          tmp.push_back(j + 1);
      }
    }
    values.push_back(tmp);
  }

  vector<vector<int> > filled(sudoku.begin(), sudoku.end());
  for (int i = 0; i < N; i++) {
    int index = 0;
    for (int j = 0; j < N; j++) {
      if (filled[i][j] == 0) {
        int ran = (rand() % (values[i].size() - index)) + index;
        int tmp = values[i][index];
        values[i][index] = values[i][ran];
        values[i][ran] = tmp;
        filled[i][j] = values[i][index];
        index++;
      }
    }
  }
  return filled;
}

void printSudoku(vector<vector<int> > &sudoku) {
  printf("[\n");
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if (sudoku[i][j] == 0) {
        printf("_, ");
      } else {
        printf("%d, ", sudoku[i][j]);
      }
    }
    printf("\n");
  }
  printf("]\n");
}
