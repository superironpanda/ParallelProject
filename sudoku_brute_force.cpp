#include <iostream>
#include <vector>
#include <stdio.h>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include "time.h"
using namespace std;

void dfs(vector<vector<int> >&, int, int);
bool valid(vector<vector<int> >&);
void printSudoku(vector<vector<int> > &);

int N = 9;
bool found = false;

int main() {
  vector<vector<int> > sudoku({{7,3,0,6,1,4,8,9,2},{8,4,2,9,7,3,5,0,1},{9,6,1,2,8,5,3,7,0},{2,8,6,3,4,9,1,5,7},{4,1,0,8,5,7,9,2,6},{5,7,9,1,0,6,0,3,8},{1,5,7,4,9,2,6,8,3},{6,9,4,7,3,8,2,1,5},{3,2,8,5,6,1,7,4,9}});
  // vector<vector<int> > sudoku({{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}});

  timespec startTime, endTime;
  clock_gettime(CLOCK_REALTIME, &startTime);

  dfs(sudoku, 0, 0);

  clock_gettime(CLOCK_REALTIME, &endTime);
  printf("time used(sec): %f\n\n", endTime.tv_sec - startTime.tv_sec + (endTime.tv_nsec - startTime.tv_nsec) / 1000000000.);
}

void dfs(vector<vector<int> > &sudoku, int row, int col) {
  if (found) {
    return;
  }

  if (row == N) {
    if (valid(sudoku)) {
      found = true;
      printSudoku(sudoku);
    }
    return;
  }

  if (col == N) {
    dfs(sudoku, row + 1, 0);
  } else if (sudoku[row][col] > 0) {
    dfs(sudoku, row, col + 1);
  } else {
    for (int i = 1; i <= N; i++) {
      sudoku[row][col] = i;
      dfs(sudoku, row, col + 1);
      sudoku[row][col] = 0;
    }
  }
}

bool valid(vector<vector<int> > &sudoku) {
  for (int i = 0; i < 9; i++) {
      bool rep[9] = {0};
      for (int j = 0; j < 9; j++) {
          int digit = sudoku[i][j] - 1;
          if (rep[digit])
              return false;
          rep[digit] = true;
      }
  }
  
  for (int j = 0; j < 9; j++) {
      bool rep[9] = {0};
      for (int i = 0; i < 9; i++) {
          int digit = sudoku[i][j] - 1;
          if (rep[digit])
              return false;
          rep[digit] = true;
      }
  }
  
  for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
          bool rep[9] = {0};
          for (int m = 0; m < 3; m++) {
              for (int n = 0; n < 3; n++) {
                  int digit = sudoku[m + 3 * i][n + 3 * j] - 1;
                  if (rep[digit])
                      return false;
                  rep[digit] = true;
              }
          }
      }
  }
  return true;
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