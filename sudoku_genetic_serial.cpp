//
//  main.cpp
//  SudokuSolver
//
//  Created by Junda Lou on 5/3/20.
//  Copyright © 2020 Junda Lou. All rights reserved.
//

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <string>
#include <unordered_set>
#include <ctime>
#include <algorithm>

using namespace std;

const int N = 9;
const int INIT_POP_SIZE = 1000;
const double SELECTION_RATE = 0.15;
const double MUTATION_RATE = 0.8;

double fitness(vector<vector<int> >&);
bool mutate(vector<vector<int> >&, double, vector<vector<int> >&);
vector<vector<int> > crossover(vector<vector<int> >&, vector<vector<int> >&);
int nextRowUnused(vector<vector<int> >&, int);
int nextColUnused(vector<vector<int> >&, int);
int nextBlockUnused(vector<vector<int> > &, int, int);
void swap(vector<vector<int> > &, int, int, int, int);
vector<vector<int> > randomFill(vector<vector<int> >&);
void printSudoku(vector<vector<int> > &);

int main(int argc, char **agrv) {
  srand(time(NULL));

  // a valid sudoku
  // vector<vector<int> > sudoku({{7,3,5,6,1,4,8,9,2},{8,4,2,9,7,3,5,6,1},{9,6,1,2,8,5,3,7,4},{2,8,6,3,4,9,1,5,7},{4,1,3,8,5,7,9,2,6},{5,7,9,1,2,6,4,3,8},{1,5,7,4,9,2,6,8,3},{6,9,4,7,3,8,2,1,5},{3,2,8,5,6,1,7,4,9}});
  
  // test case
  // vector<vector<int> > sudoku({{7,3,5,6,0,4,0,0,2},{8,4,2,9,7,3,5,6,0},{9,0,1,2,8,5,3,7,0},{2,8,0,3,4,9,1,0,7},{4,1,0,8,5,7,0,2,6},{5,7,0,1,2,6,0,3,8},{1,5,7,4,9,2,6,8,0},{6,9,4,7,3,0,2,1,5},{3,2,8,0,6,1,0,4,9}});
  vector<vector<int> > sudoku({{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}});
  printSudoku(sudoku);

  // record empty positions to be filled, it will help mutate() since we cannot swap already-filled positions
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

  // randomly fill the population
  vector<vector<vector<int> > > population;
  for (int i = 0; i < INIT_POP_SIZE; i++) {
    population.push_back(randomFill(sudoku));
  }

  int generation = 1;
  while (!population.empty()) {
    // sort the population based on fitness
    sort(population.begin(), population.end(), [](vector<vector<int> > &cand1, vector<vector<int> > &cand2) {
      return fitness(cand1) > fitness(cand2);
    });

    // print the progress
    printf("Generation: %d\nFitness top 5: ", generation);
    for (int i = 0; i < min((int)population.size(), 5); i++) {
      printf("%f, ", fitness(population[i]));
    }
    printf("\n");
    printSudoku(population[0]);

    // check for answer
    if (fitness(population[0]) == 1.0) {
      printf("Sudoku solved at generation %d: \n", generation);
      printSudoku(population[0]);
      break;
    }
    vector<vector<vector<int> > > next_population;

    // Selection
    int selection = (int)(population.size() * SELECTION_RATE);
    for (int i = 0; i <= selection; i++) {
      next_population.push_back(population[i]);
    }
    
    // Crossover
    int bound = next_population.size();
    for (int i = next_population.size(); i < INIT_POP_SIZE; i++) {
      int i1 = rand() % bound, i2 = rand() % bound;
      next_population.push_back(crossover(next_population[i1], next_population[i2]));
    }

    // Mutation
    for (int i = 0; i < next_population.size(); i++) {
      mutate(next_population[i], MUTATION_RATE, spots);
    }

    population = next_population;
    generation++;
  }
}

double fitness(vector<vector<int> > &sudoku) {
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

  // cout << "row sum: " << (int)row_sum << ", col sum: " << (int)col_sum << ", block sum: " << (int)block_sum << endl;
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
      if (nextColUnused(sudoku, col1) != -1 && nextColUnused(sudoku, col2) != -1 || nextBlockUnused(sudoku, row / 3 * 3, col1 / 3 * 3) != -1 && nextBlockUnused(sudoku, row / 3 * 3, col2 / 3 * 3) != -1) {
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

// return -1 if this row is not valid (have duplicate)
// return 0 if all 9 numbers are used
// return the next unused number
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

// (row, col) = top left cell of the block
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
