#include <iostream>
#include <ctime>
#include <queue>
#include <vector>
#include <random>
#include <bits/stdc++.h>
#include <chrono>

using namespace std;

typedef unsigned int ui;

bool isSolved = false;
bool flag = false;
int Size;
char mdTable[16][16];
char movableTable[16][4];
char path[85];
ui nodeCount = 0;

struct NodeInfo {
	ui id;
	ui parent;
	char blank;
	char move;
};

struct Node {
	char board[16];
	NodeInfo nodeInfo;
	char f, g, move;

	Node(char* bd, char blk, char move, ui id, ui parent, char f = 0, char g = 0){
		//#pragma omp parallel for schedule(dynamic,1)
		for (int i= 0; i<Size*Size; ++i){
			board[i]= bd[i];
		}
		nodeInfo.blank = blk;
		nodeInfo.id = id;
		nodeInfo.parent = parent;
		nodeInfo.move = move;
		this->f = f;
		this->g = g;
	}

	bool operator >(const Node& other) const{
		return f > other.f;
	}
};

void MakeMovableTable(int size){
	int moves[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
	int board[4][4];
	//#pragma omp parallel for schedule(dynamic,1) collapse(2)
	for(int y = 0; y<size; ++y){
		for(int x = 0; x<size; ++x){
			board[y][x] = x+y*size;
		}
	}
	int dx, dy, j;
	//#pragma omp parallel for schedule(dynamic,1) collapse(3)
	for(int y = 0; y<size; ++y){
		for(int x = 0; x<size; ++x){
			for(int i = 0; i<4; ++i){
				dx = moves[i][0];
				dy = moves[i][1];
				if(x+dx<0 || y+dy<0 || x+dx>=size || y+dy>=size){
					j = -1;
				}
				else{
					j = board[y+dy][x+dx];
				} 
				movableTable[x+y*size][i] = j;
			}
		}
	}
}

void MakeMDTable(int size){
	int i=1;
	//#pragma omp parallel for schedule(dynamic,2) collapse(2)
		for(int y=0;y<size*size;y=y+1){
		for(int x = 0; x < size * size; ++x){
			if(i == 0){
				mdTable[i][x] = 0;
			}
			else{
				mdTable[i][x] = abs((y / size) - (x / size)) + abs((y % size) - (x % size));
			}
		}
		i=(i+1)%(size*size);
		}
	
}

char GetDistance(int *row, char md, int nums){
	if (nums > 1){
		if (nums == 2){
			if (row[0] > row[1])
				md += 2;
		}
		else if (nums == 3){
			if (row[0] > row[1] || row[0] > row[2])
				md += 2;
			if (row[1] > row[2])
				md += 2;
		}
		else if (nums == 4){
			if (row[0] > row[1] || row[0] > row[2] || row[0] > row[3])
				md += 2;
			if (row[1] > row[2] || row[1] > row[3])
				md += 2;
			if (row[2] > row[3])
				md += 2;
		}
	}
	return md;
}

char GetManhattan(char* puzzle){
	int i, j, x, md = 0;
	int k, n;
	int temp[4];
   //#pragma omp for schedule(dynamic,2)
	for(i = 0; i < Size*Size; ++i){
		md += mdTable[puzzle[i]][i];
	}
   
	x=1;
	//#pragma omp for
	for (i = 0; i<Size; ++i){
		k = 0;
		//#pragma omp parallel for
		for (j = 0; j<Size; ++j){
			n = puzzle[i*Size+j];
			if (n <= Size*x && n>Size*(x-1)){
				temp[k++]= n;
			}
		}
		md = GetDistance(temp, md, k);
		x+=1;
	}
	
	
	for (i = 0, x = 1; i < Size; ++i, ++x){
		k = 0;
		for (j = 0; j < Size; ++j){
			n = puzzle[j * Size + i];
			if (n == x || n == x + Size || n == x + Size * 2 || n == x + Size * 3){
				temp[k++] = n;
			}
		}
		md = GetDistance(temp, md, k);
	}
	return md;
} 

char GetBlank(char *puzzle){
	
	for(int i = 0; i < Size * Size; ++i){
		if (puzzle[i] == 0)
		{
			return i;
		}
	}
}

void PrintPath(char depth){
	//#pragma omp for schedule(dynamic,2)
	for(int i = 0; i < depth; ++i){
		printf("%2d  ",path[i]);
		if((i + 1) % 10 == 0) 
			printf("\n");
	}
	printf("\n\n");
}

void PrintPuzzle(char* puzzle){
   // #pragma omp for schedule(dynamic,2)
	for(int i = 0; i < Size; ++i){
		for(int j = 0; j < Size; ++j){
			printf("%2d ", puzzle[i * Size + j]);
		}
		printf("\n");
	}
	printf("\n");
}

char IdaStar(char depth, char maxDepth, char* puzzle, char lastMove, char blank){
	char h, f, min;
	nodeCount += 1;
	h = GetManhattan(puzzle);
	f = depth + h;
	if (f > maxDepth ){
		return f;
	}

	if (h == 0){
		isSolved = true;
		return f;
	}

	min = 127;
	
	for (int move = 0; move < 4; ++move){
		if(lastMove == -1 || (move + lastMove) % 4 != 1){
			char newBlank = movableTable[blank][move];
			if(newBlank == -1) 
				continue;
			puzzle[blank] = puzzle[newBlank];
			puzzle[newBlank] = 0; 
			f = IdaStar(depth + 1, maxDepth, puzzle, move, newBlank);
			puzzle[newBlank] = puzzle[blank];
			puzzle[blank] = 0;
			if (f < min){
				min = f;
			}
			if (isSolved){
				path[depth] = puzzle[newBlank];
				return min;
			}
		}
	}
	return min;
}

void IDAStar(char* puzzle){
	time_t start, end;
	char h, depth;
	char lastMove = -1;
	char blank = GetBlank(puzzle);
	
	isSolved = false;
	ui totalCount = 0;
	
	h = GetManhattan(puzzle);
	depth = h;
	start = clock();
	

  #pragma omp parallel for
  
    for(int i =0 ;i<30 ; i++){
      if (flag) continue;
      nodeCount = 0;
      depth = IdaStar(0, depth, puzzle, lastMove, blank);
      end = clock();
      totalCount += nodeCount;
      printf("Depth = %d, Node Count = %10u, Total Count = %10u, time = %6.2f msecs\n",h, nodeCount, totalCount, (end-start)/1000.);
      // printf("depth %d",depth);
      if (isSolved){
        printf("Solved,Total Node Count = %u, shortest path length = %d, time = %.2f msecs\n",totalCount, depth, (end-start)/1000.);
        // return;
        flag = false;
		i=30;
        
      }
      h = depth;
	}}


void ShuffleArray(char arr[], int n) { //from : https://www.geeksforgeeks.org/shuffle-an-array-using-stl-in-c/
	 // // To obtain a time-based seed 
    // unsigned seed = chrono::system_clock::now().time_since_epoch().count(); 
    // // Shuffling our array 
    // shuffle(arr, arr + n, default_random_engine(seed)); 
    arr[0] = 12;
    arr[1] = 9;
    arr[2] = 13;
    arr[3] = 7;
    arr[4] = 5;
    arr[5] = 4;
    arr[6] = 11;
    arr[7] = 10;
    arr[8] = 3;
    arr[9] = 2;
    arr[10] = 14;
    arr[11] = 1;
    arr[12] = 6;
    arr[13] = 15;
    arr[14] = 0;
    arr[15] = 8;

	////////////////////////////////////////
	// arr[0] = 1;
    // arr[1] = 5;
    // arr[2] = 2;
    // arr[3] = 4;
    // arr[4] = 3;
    // arr[5] = 0;
    // arr[6] = 6;
    // arr[7] = 7;
    // arr[8] = 8;
    // arr[9] = 9;
    // arr[10] = 10;
    // arr[11] = 11;
    // arr[12] = 12;
    // arr[13] = 13;
    // arr[14] = 14;
    // arr[15] = 15;
	////////////////////////////////////////////////////////
	// arr[0] = 1;
    // arr[1] = 2;
    // arr[2] = 0;
    // arr[3] = 4;
    // arr[4] = 13;
    // arr[5] = 5;
    // arr[6] = 0;
    // arr[7] = 6;
    // arr[8] = 12;
    // arr[9] = 11;
    // arr[10] = 9;
    // arr[11] = 7;
    // arr[12] = 10;
    // arr[13] = 8;
    // arr[14] = 14;
    // arr[15] = 15;

/////////////////////////////////////////////////////




}

int* ToIntArr(char * c , int s){
 	int *board=new int[s];
	for(int i=0;i<s;i++){
		int a=(int) c[i];
		board[i]=a;
	}
	return board;
} 

bool solvable(char * c){
	int *board=ToIntArr(c,16);
	// for(int i=0;i<16;i++)
	// 	cout<<board[i]<<" ";
	// cout<<endl;
	int count=0;
	for(int i=0;i<15;i++){ 
        for (int j=i+1;j<16;j++)
             if(board[i]>board[j] && board[i]!=0 &&board[j]!=0){ 
                 count++;
             }
    }
    return (count%2+1)%2;
}

int main(){
	int  size=4;
	char puzzle[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0};
	char p[16];
	MakeMDTable(size);
	MakeMovableTable(size);
	Size = size;
	ShuffleArray(puzzle,16);
	while(!solvable(puzzle)){
		ShuffleArray(puzzle,16);
	}
	cout << "  PUZZLE TO SOLVE  \n--------------------" << endl << endl;
	PrintPuzzle(puzzle);
	cout << "--------------------" << endl << endl;
	// cout<<solvable(puzzle)<<endl;
	printf("\n===== IDA Star Algiorithm =====\n");
	IDAStar(puzzle);

	return 0;
}
