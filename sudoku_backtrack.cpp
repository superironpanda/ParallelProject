#include <iostream>
#include <vector>
#include <stdio.h>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include "time.h"
using namespace std;

class Solution {
public:
    void solveSudoku(vector<vector<char> >& board) {
        solveSudoku(board, 0, 0);
    }

    bool solveSudoku(vector<vector<char> > &board, int i, int j) {
        if (i == 9) return true; // done
        if (j == 9) return solveSudoku(board, i + 1, 0); // the next row
        if (board[i][j] != '.') return solveSudoku(board, i, j + 1); // the same row, the next col
        for(char c = '1'; c <= '9'; c++) {
            if (check(board, i, j, c)) {
                board[i][j] = c;
                if (solveSudoku(board, i, j + 1)) return true; // the same row, the next col
                board[i][j] = '.';
            }
        }
        return false;
    }

    bool check(vector<vector<char> > &board, int i, int j, char val) {
        int row = i - i % 3, column = j - j % 3;
        for (int x = 0; x < 9; x++) {
            if(board[x][j] == val) return false;
        }
        for (int y = 0; y < 9; y++) {
            if(board[i][y] == val) return false;
        }
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                if(board[row + x][column + y] == val) {
                    return false;
                }
            }
        }
        return true;
    }

};

vector<vector<char> > generateMatrix(int row, int col) {
        // vector<vector<char> > matrix;
        // matrix.resize(row, vector<char>(col));
        // for (int i = 0; i < row; i++) {
        //         for (int j = 0; j < col; j++) {
        //                 matrix[i][j] = '.';
        //         }
        // }
        // vector<vector<char> > matrix({
        // {'7', '3', '5' , '6','.','4','.','.','2'},{'8','4','2','9','7','3','5','6','.'},{'9','.','1','2','8','5','3','7','.'},
        // {'2','8','.','3','4','9','1','.','7'},{'4','1','.','8','5','7','.','2','6'},{'5','7','.','1','2','6','.','3','8'},
        // {'1','5','7','4','9','2','6','8','.'},{'6','9','4','7','3','.','2','1','5'},{'3','2','8','.','6','1','.','4','9'}});
        vector<vector<char> > matrix({
        {'7', '3', '5', '.','.','4','.','.','2'},{'.','4','2','9','7','3','5','6','.'},{'9','.','1','.','8','5','3','7','.'},
        {'2','8','.','.','4','9','1','.','.'},{'4','.','.','8','5','7','.','.','6'},{'5','7','.','.','2','6','.','3','8'},
        {'1','5','.','4','9','2','6','8','.'},{'6','.','4','7','3','.','2','.','5'},{'3','2','8','.','6','1','.','4','9'}});
        return matrix;
}

void printMatrix(vector<vector<char> >&board, int row, int col) {
        for (int i = 0; i < row; i++) {
                for (int j = 0; j < col; j++) {
                        cout << board[i][j] << ", ";
                }
                cout << "\n";
        }
}

int main() {
        timespec startTime, endTime;
        clock_gettime(CLOCK_REALTIME, &startTime);

        vector<vector<char> > matrix = generateMatrix(9, 9);
        Solution solver;
        solver.solveSudoku(matrix);
        printMatrix(matrix, 9, 9);

        clock_gettime(CLOCK_REALTIME, &endTime);
        printf("time used(sec): %f\n\n", endTime.tv_sec - startTime.tv_sec + (endTime.tv_nsec - startTime.tv_nsec) / 1000000000.);
}
