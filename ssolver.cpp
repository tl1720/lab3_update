#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <omp.h>
#include <fstream>
#include <sstream>

using namespace std;

#define BOARD_SIZE 9
#define BOX_SIZE 3

vector<vector<int> > board;
int thread_number = 2;
bool find_flag = false;
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
	int box_row = cur_row/BOX_SIZE*BOX_SIZE;
	int box_col = cur_col/BOX_SIZE*BOX_SIZE;
	for (int i = box_row;i < box_row+BOX_SIZE;i++) {
		for (int j = box_col;j < box_col+BOX_SIZE;j++) {
			if (board[i][j] == val && (i != cur_row || j != cur_col)) {
				return false;
			}
		}
	}
	
	return true;
}


bool sudoku_solver (vector<vector<int> > &board, int empty_idx, int &node_count) {
	//node_count++;
	if (empty_idx == empty_cells.size()) {
		sol = vector<vector<int> >(board);
		find_flag = true;
		return true;
	}
	
	int flag = 0;
	int row = empty_cells[empty_idx].first;
	int col = empty_cells[empty_idx].second;
	
	for (int digit = 1;digit <= BOARD_SIZE;digit++) {
		board[row][col] = digit;
		//#pragma omp task 
		{

		if (!flag && !find_flag && is_vaild(board, row, col)) {
			if (sudoku_solver(board, empty_idx+1, node_count)) { 
				flag = 1;
			}
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
	if (argc < 2) {
		printf("error argument number\n");
		return 0;
	} 

	if (argc == 3) {
		thread_number = atoi(argv[2]);
	} else if (argc == 2) {
		thread_number = 3;
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

	
	bool res = false;
	int result = 1;
	omp_set_num_threads(thread_number);
	
	double tstart = 0.0, ttaken = 0.0;
	tstart = omp_get_wtime();

	row = empty_cells[0].first;
	col = empty_cells[0].second;

	int node_counts[9] = {0};
	int digit = 4;

	#pragma omp parallel for schedule(static)
	for (int digit = 1;digit <= BOARD_SIZE;digit++) {	
		vector<vector<int> > new_board(board);
		new_board[row][col] = digit;
		int local_count = 0;
		sudoku_solver(new_board, 1, local_count);
	}
	ttaken = omp_get_wtime() - tstart;
	printf("Time take for the main part: %f\n",ttaken);

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

	return 0;
}