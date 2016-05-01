#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "mpi.h"
#include "omp.h"
#include <fstream>
#include <sstream>

using namespace std;

#define BOARD_SIZE 9
#define PARTITION_SIZE 3

vector<vector<int> > board;
int find_flag = 0;
double tstart = 0.0;
double ttaken = 0.0;
vector<pair<int, int> > empty_cells;
vector<vector<int> > sol;

bool is_vaild (vector<vector<int> > &board, int cur_row, int cur_col) {
	int val = board[cur_row][cur_col];
	if (val == 0) {
		return true;
	}

	// check row and column
	for (int i = 0;i < BOARD_SIZE;i++) {
		if (board[cur_row][i] == val && i != cur_col) {
			return false;
		}
		if (board[i][cur_col] == val && i != cur_row) {
			return false;
		}
	}

	// check box
	int box_row = cur_row/PARTITION_SIZE*PARTITION_SIZE;
	int box_col = cur_col/PARTITION_SIZE*PARTITION_SIZE;
	for (int i = box_row;i < box_row+PARTITION_SIZE;i++) {
		for (int j = box_col;j < box_col+PARTITION_SIZE;j++) {
			if (board[i][j] == val && (i != cur_row || j != cur_col)) {
				return false;
			}
		}
	}
	return true;
}


bool sudoku_solver (vector<vector<int> > &board, int empty_idx) {
	if (empty_idx == empty_cells.size()) {
		sol = vector<vector<int> >(board);
		find_flag = 1;
		return true;
	}
	
	int flag = 0;
	int row = empty_cells[empty_idx].first;
	int col = empty_cells[empty_idx].second;
	
	for (int digit = 1;digit <= BOARD_SIZE;digit++) {
		board[row][col] = digit;
		if (!flag && !find_flag && is_vaild(board, row, col)) {
			if (sudoku_solver(board, empty_idx+1)) { 
				flag = 1;
			}
		}
	}

	if (!flag) {
		board[row][col] = 0;
		return false;
	}
	return true;
}


int main (int argc, char *argv[]) {
	int proc_size;
	int proc_rank;
	MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

	if (argc < 2 || argc > 2) {
		printf("error argument number\n");
		return 0;
	}

	ifstream matrix_file;
	matrix_file.open(argv[1]);
	if (!matrix_file.is_open()) {
		cout << "no such file" << endl;
		return 0;
	}

	char inputfilename[255] = "";
	memcpy(inputfilename, argv[1], sizeof(argv[1])-2);

	string token;
	int row = 0;
	int col = 0;
	while (getline(matrix_file, token)) {
		istringstream line(token);
		vector<int> row_vec;
		line >> token;
		for (int j = 0;j < token.size();j++) {
			row_vec.push_back(token[j]-'0');
			if (token[j] == '0') {
				empty_cells.push_back(make_pair(row, j));
			} 
		}
		board.push_back(row_vec);
		row++;
	}
	matrix_file.close();

	tstart = omp_get_wtime();
    
    int from = proc_rank*PARTITION_SIZE+1;
    int to = (proc_rank+1)*PARTITION_SIZE;
    int init_row = empty_cells[0].first;
    int init_col = empty_cells[0].second;
	for (int digit = from;digit <= to;digit++) {	
		vector<vector<int> > new_board(board);
		new_board[init_row][init_col] = digit;
		sudoku_solver(new_board, 1);	
	}
	

    ttaken = omp_get_wtime() - tstart;
	printf("Time take for the main part: %f %d %d\n",ttaken,proc_rank,node_count);		
    
    if (find_flag) {
    	char file_name[255] = {0};
    	sprintf(file_name, "%s.sol", inputfilename);
    	FILE *fp = fopen(file_name, "w");
    	if (!fp) {
        	printf("can't open a file\n");
        	return 0;
    	}

    	for (int i = 0;i < BOARD_SIZE;i++) {
			for (int j = 0;j < BOARD_SIZE;j++) {
				fprintf(fp, "%d",sol[i][j]);
			}
			fprintf(fp,"\n");
		}
		fclose(fp);	
	}

	MPI_Finalize();

	return 0;
}