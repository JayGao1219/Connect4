#include <iostream>
#include <ctime>				// ʹ��clock������ʱ
#include "Point.h"
#include "Strategy.h"
#include "Judge.h"
#include <conio.h>				// ������Դ�ӡ
#include <atlstr.h>


#define MAX_TIME 2.9
#define MAX_M 12
#define MAX_N 12


using namespace std;

// ���Գ���
const bool DEBUG = false;

// ���̸��
const int GRID_EMPTY = 0;
const int GRID_YOU = 1;
const int GRID_ME = 2;
const int GRID_NO = 3;

// ���ڵ���
const int MAX_NODES = 2000000;

// ������ұ��
const int PLAYER_YOU = 1;
const int PLAYER_ME = 2;
const int PLAYER_ADD = 3;

// ������ֽ����жϵķ��ش���
const int CODE_TIE = 0;
const int CODE_WIN = 1;
const int CODE_NOT_END = 2;

// �����ӽڵ㹫ʽ�еĳ���, ����Խ��Խ�����ڷ���, ԽСԽ����������
const float FORMULA_C = 1;

// ����ֵ�ͷ�
const float PUNISH_QUICK_DEAD = 5;

// ���̴�С, ����Ϊȫ�ֱ���, ������ֺ������໥����
int GRID_M = 0;
int GRID_N = 0;

// �ȶ���һ����ʱ������,����ÿ���¿�һ�������˷�ʱ��
int tmp_top[MAX_N];
int** tmp_board = new int*[MAX_M];

// �����������ṹ, һ���԰ѿ����õ���treenode�ռ�ȫ��
int num_node_used = 0;
struct searchTreeNode
{
	searchTreeNode *child[MAX_M];
	searchTreeNode *father;
	int num_visit;
	int num_win;
};
searchTreeNode tree[MAX_NODES];

// ��Ա����
searchTreeNode* newNode(searchTreeNode *father);
int isTerminate(int *top, int** board, int x, int y, int current_player);
int isTerminate(int** board, int x, int y, int current_player);
void backUp(searchTreeNode *node, int gain);
int defaultPolicy(int *top, int** board, int last_x, int last_y, int current_player);
searchTreeNode* treePolicy(searchTreeNode *root, int *top, int** board, int &last_x, int &last_y, int &current_player);
int bestChild(searchTreeNode *root, int *feasible, int feasible_count);
searchTreeNode* expand(searchTreeNode *node, int &chosen_child, int *feasible, int feasible_count);
int bestSolution(searchTreeNode* root);
void print_board_top(int *top, int** board);
void print_feasible(int *feasible, int feasible_count);

/*
	���Ժ����ӿ�,�ú������Կ�ƽ̨����,ÿ�δ��뵱ǰ״̬,Ҫ�����������ӵ�,�����ӵ������һ��������Ϸ��������ӵ�,��Ȼ�Կ�ƽ̨��ֱ����Ϊ��ĳ�������
	
	input:
		Ϊ�˷�ֹ�ԶԿ�ƽ̨ά����������ɸ��ģ����д���Ĳ�����Ϊconst����
		M, N : ���̴�С M - ���� N - ���� ����0��ʼ�ƣ� ���Ͻ�Ϊ����ԭ�㣬����x��ǣ�����y���
		top : ��ǰ����ÿһ���ж���ʵ��λ��. e.g. ��i��Ϊ��,��_top[i] == M, ��i������,��_top[i] == 0
		_board : ���̵�һά�����ʾ, Ϊ�˷���ʹ�ã��ڸú����տ�ʼ���������Ѿ�����ת��Ϊ�˶�ά����board
				��ֻ��ֱ��ʹ��board���ɣ����Ͻ�Ϊ����ԭ�㣬�����[0][0]��ʼ��(����[1][1])
				board[x][y]��ʾ��x�С���y�еĵ�(��0��ʼ��)
				board[x][y] == 0/1/2 �ֱ��Ӧ(x,y)�� ������/���û�����/�г������,�������ӵ㴦��ֵҲΪ0
		lastX, lastY : �Է���һ�����ӵ�λ��, ����ܲ���Ҫ�ò�����Ҳ������Ҫ�Ĳ������ǶԷ�һ����
				����λ�ã���ʱ��������Լ��ĳ����м�¼�Է������ಽ������λ�ã�����ȫȡ�������Լ��Ĳ���
		noX, noY : �����ϵĲ������ӵ�(ע:��ʵ���������top�Ѿ����㴦���˲������ӵ㣬Ҳ����˵���ĳһ��
				������ӵ�����ǡ�ǲ������ӵ㣬��ôUI�����еĴ�����Ѿ������е�topֵ�ֽ�����һ�μ�һ������
				��������Ĵ�����Ҳ���Ը�����ʹ��noX��noY��������������ȫ��Ϊtop������ǵ�ǰÿ�еĶ�������,
				��Ȼ�������ʹ��lastX,lastY�������п��ܾ�Ҫͬʱ����noX��noY��)
		���ϲ���ʵ���ϰ����˵�ǰ״̬(M N _top _board)�Լ���ʷ��Ϣ(lastX lastY),��Ҫ���ľ�������Щ��Ϣ�¸������������ǵ����ӵ�
	output:
		������ӵ�Point
*/
extern "C" __declspec(dllexport) Point* getPoint(const int M, const int N, const int* top, const int* _board,
	const int lastX, const int lastY, const int noX, const int noY) {

	// ��¼��ʼʱ��
	double tic = (double)clock() / CLOCKS_PER_SEC;
	// ��¼�м��ʱ��
	double toc = 0;
	// ��¼ÿ��̽���Ľڵ�
	searchTreeNode *node;
	// ��¼̽���ڵ�����ӵ�λ��
	int last_x = 0;
	int last_y = 0;
	// ��¼̽���ڵ�����һ���Ķ���
	int player = 0;
	// ��¼ÿһ��̽���Ľ��
	int gain = 0;
	// ��¼���̴�С
	GRID_M = M;
	GRID_N = N;
	// ��ʼ��node����
	num_node_used = 0;

	// for debug
	if (DEBUG) AllocConsole();

	/*
		������ǰ����
		board�п��ܵ�״̬ΪEMPTY, YOU, ME, NO
	*/
	// ���ص����ӵ�
	int x = -1, y = -1;
	// �½�һ���������̱���, �½�һ����ʱ���̱���
	int** board = new int*[M];
	for (int i = 0; i < M; i++) {
		board[i] = new int[N];
		tmp_board[i] = new int[MAX_N];
		for (int j = 0; j < N; j++) {
			tmp_board[i][j] = 0;
			board[i][j] = _board[i * N + j];
			if ((i == noX) && (j == noY))
				board[i][j] = GRID_NO;
		}
	}

	/*
		ʹ�������Ͻ����㷨(UCT)
	*/
	searchTreeNode *root = newNode(NULL);
	// ������ѭ��
	int count = 0;
	while (1) {
		count++;
		// ��������
		for (int i = 0; i < GRID_N; i++) {
			tmp_top[i] = top[i];
			for (int j = 0; j < GRID_M; j++) {
				tmp_board[j][i] = board[j][i];
			}
		}
		// ���Դ�ӡ
		if ((DEBUG) && (count == 1)) print_board_top(tmp_top, tmp_board);
		// ÿ���ҳ�Ŀǰ��õĽڵ������չ, �ڵ�����ۼ��ʹ�ýϺõķ�֧��̽��������С�ķ�֧
		node = treePolicy(root, tmp_top, tmp_board, last_x, last_y, player);
		gain = defaultPolicy(tmp_top, tmp_board, last_x, last_y, player);
		backUp(node, gain);
		// ��ʱ,���ʱ��쵽�˾Ͳ�������
		toc = (double)clock() / CLOCKS_PER_SEC;
		if ((toc - tic > MAX_TIME) || (num_node_used > MAX_NODES - 10))
			break;
	}
	y = bestSolution(root);
	x = top[y] - 1;

	if (DEBUG) _cprintf("node exploited = %d  path searched = %d \n", num_node_used, count);
	
	clearArray(M, N, board);
	return new Point(x, y);
}

/*
getPoint�������ص�Pointָ�����ڱ�dllģ���������ģ�Ϊ��������Ѵ���Ӧ���ⲿ���ñ�dll�е�
�������ͷſռ䣬����Ӧ�����ⲿֱ��delete
*/
extern "C" __declspec(dllexport) void clearPoint(Point* p) {
	delete p;
	return;
}

/*
	��ӡ���,����ר��
*/
void print_board_top(int *top, int** board) {
	_cprintf("top = [");
	for (int i = 0; i < GRID_N; i++) {
		_cprintf("%d, ", top[i]);
	}
	_cprintf("] \n board = \n");
	for (int j = 0; j < GRID_M; j++) {
		_cprintf("[");
		for (int i = 0; i < GRID_N; i++) {
			_cprintf("%d, ", board[j][i]);
		}
		_cprintf("]\n");
	}
}

/*
	��ӡ���н�,����ר��
*/
void print_feasible(int *feasible, int feasible_count) {
	_cprintf("Feasible list = [");
	for (int i = 0; i < feasible_count; i++) {
		_cprintf("%d ", feasible[i]);
	}
	_cprintf("]\n");
}

/*
	���top��board����
*/
void clearArray(int M, int N, int** board) {
	for(int i = 0; i < M; i++){
		delete[] board[i];
	}
	delete[] board;
}

/*
	�ҳ��������Ӿ���
*/
int bestSolution(searchTreeNode* root) {
	float max_val = -FLT_MAX;
	float val;
	int ind = -1;
	if (DEBUG) _cprintf("find sol \n");
	for (int i = 0; i < GRID_N; i++) {
		if (root->child[i]) {
			val = (float)root->child[i]->num_win / (float)root->child[i]->num_visit;
			if (DEBUG)  _cprintf(" [%d, %d, %d, %.3f] \n", i, root->child[i]->num_visit, root->child[i]->num_win, val);
			if (val > max_val) {
				max_val = val;
				ind = i;
			}
		}
	}
	return ind;
}

/*
	��ʼ�������ڵ�
*/
searchTreeNode* newNode(searchTreeNode *father) {
	searchTreeNode *node = &tree[++num_node_used];
	node->num_visit = 0;
	node->num_win = 0;
	node->father = father;
	for (int i = 0; i < GRID_N; i++) {
		node->child[i] = NULL;
	}
	return node;
}

/* 
	IsTerminate Function
	����һ�����״̬, ��һ���������, ���ӵ�ѡ��
	�ж��������Ƿ����,���ص�ֵΪCODE_WIN, CODE_TIE, CODE_NOT_END
*/
int isTerminate(int *top, int** board, int x, int y, int current_player) {
	if ((current_player == PLAYER_ME) && (machineWin(x, y, GRID_M, GRID_N, board))) {
		return CODE_WIN;
	}
	else if ((current_player == PLAYER_YOU) && (userWin(x, y, GRID_M, GRID_N, board))) {
		return CODE_WIN;
	}
	else {
		for (int i = 0; i < GRID_N; i++) {
			if (top[i] != 0) {
				return CODE_NOT_END;
			}
		}
		return CODE_TIE;
	}
}

/*
	IsTerminate Function ����
	����һ�����״̬, ��һ���������, ���ӵ�ѡ��
	�жϸ������Ƿ��ܹ�ȡʤ,���ص�ֵΪCODE_WIN, CODE_NOT_END
	����treePolicy�е��ж�,���������治��Ҫ����ƽ�ֽ����ж�,��� ����������������ڶ����Ƿ��ʤ���ж�
*/
int isTerminate(int** board, int x, int y, int current_player) {
	if ((current_player == PLAYER_ME) && (machineWin(x, y, GRID_M, GRID_N, board))) {
		return CODE_WIN;
	}
	else if ((current_player == PLAYER_YOU) && (userWin(x, y, GRID_M, GRID_N, board))) {
		return CODE_WIN;
	}
	else {
		return CODE_NOT_END;
	}
}

/*
	BackUp Function in UCT
	����ѵõ��Ľڵ�����淴����µ��������ڵ���,���Ұ���Ӧ������״̬���λָ�����ʼ״̬
*/
void backUp(searchTreeNode *node, int gain) {
	while (node) {
		// �ı���ص�ͳ����
		node->num_visit++;
		node->num_win += gain;
		gain = (gain > 1) ? (1 - gain) : ( (gain < -1) ? (-1 - gain) : (-gain) );
		node = node->father;
	}
}

/*
	DefaultPolicy Function in UCT
	��������ȸ������ж������ҳ��ж�����Ȼ������ж�,�ﵽ���Ľڵ�֮�󷵻���Ӧ������
	���ｫ�ᴫ�����̵�״̬,��󷵻ص�ʱ������״̬�ʹ����ʱ��һ��
*/
int defaultPolicy(int *top, int** board, int last_x, int last_y, int current_player) {
	// _cprintf("enter defaultPolicy\n");
	// _cprintf("defaultPolicy:: enter with (%d, %d) with player %d \n", last_x, last_y, current_player);

	const int init_player = current_player;
	int status = 0;
	int feasible[MAX_N];
	int feasible_count;
	int ind;
	
	int count = 0;
	while (1) {
		count++;
		if (count > 150) { _cprintf("default policy inf loop\n"); }
		status = isTerminate(top, board, last_x, last_y, current_player);
		switch (status)
		{
			case CODE_WIN: {
				if (init_player == current_player) {
					// �������ڵ��������ʤ,���ཱ��
					return (count == 1) ? PUNISH_QUICK_DEAD : 1;
				}
				else {
					// �������ڵ�����������,����ͷ�
					return (count == 1) ? -PUNISH_QUICK_DEAD : -1;
				}
			}
			case CODE_TIE: {
				return 0;
			}
			case CODE_NOT_END: {
				// �������п��еĵ�
				feasible_count = 0;
				for (int i = 0; i < GRID_N; i++) {
					// �����п�λ���ܹ�����,����Ҫô��һ��Ϊ��,Ҫô��һ��Ϊ���ŵ㵫�ǽ��ŵ��Ϸ����п�λ��
					if ((top[i] > 0) && ((board[top[i] - 1][i] == GRID_EMPTY) || (top[i] > 1))) {
						feasible[feasible_count] = i;
						feasible_count++;
					}
				}

				// ����ҳ�һ�����еĽ�
				ind = feasible[rand() % feasible_count];
				// �������ӷ�
				current_player = PLAYER_ADD - current_player;
				// ����
				top[ind]--;
				// ����˵����,���������Ϸ���һ��λ��
				if (board[top[ind]][ind] == GRID_NO) {
					top[ind]--;
				}
				// �������̲���
				board[top[ind]][ind] = current_player;

				// �������ӵ�
				last_x = top[ind];
				last_y = ind;
			}
			default:
				break;
		}
	}
}

/*
	TreePolicy Function in UCT
	�ҳ�Ŀǰ̽��������ǰ��,����̽���ĸ��ڵ�ָ��,����̽������ǰ�ؽڵ�ָ��
	���ͬʱtop��boardҲ���ָ���״̬���Ӧ,Ϊ�˺����жϷ���,����������һ������������
	���ص�current_playerΪ��һ�������һ��(�γ�������������һ�������һ��)
*/
searchTreeNode* treePolicy(searchTreeNode *root, int *top, int** board, int &last_x, int &last_y, int &current_player) {
	// ÿһ��������������ؽ��̽���Ľڵ���
	int chosen_child = 0;
	current_player = PLAYER_ME;
	int feasible[MAX_N];
	int feasible_count;
	// ģ�����ӵ�x����
	int check_x;
	// �ʼ����ֿ϶���û����ֹ��
	int count = 0;
	// ��¼�Ƿ�������һ����ʤ������
	bool win_flag = false;
	while (1) {
		count++;
		if (count > 150) { _cprintf("tree policy inf loop\n"); }
		// �������п��еĵ�
		feasible_count = 0;
		for (int i = 0; i < GRID_N; i++) {
			// �����п�λ���ܹ�����,����Ҫô��һ��Ϊ��,Ҫô��һ��Ϊ���ŵ㵫�ǽ��ŵ��Ϸ����п�λ��
			if ((top[i] > 0) && ((board[top[i] - 1][i] == GRID_EMPTY) || (top[i] > 1))) {
				feasible[feasible_count] = i;
				feasible_count++;
				// ������û���ĳ��λ����������֮���ֱ�ӻ�ʤ,���е㼯��ֻ����һ����
				check_x = (board[top[i] - 1][i] == GRID_EMPTY) ? (top[i] - 1) : (top[i] - 2);
				board[check_x][i] = current_player;
				if (isTerminate(board, check_x, i, current_player) == CODE_WIN) {
					feasible[0] = i;
					feasible_count = 1;
					board[check_x][i] = GRID_EMPTY;
					win_flag = true;
					break;
				}
				board[check_x][i] = GRID_EMPTY;
			}
		}

		if (feasible_count > 0) {
			// �ڿ��е㼯���ҳ�û�б�̽������
			searchTreeNode *node = expand(root, chosen_child, feasible, feasible_count);
			// �������NULL,ͬʱchosen_childΪ-1,���ʾÿ�����нڵ㶼��̽������,��˷�������ؽ��̽���Ľڵ�
			if (!node) {
				// ֻҪ�п��н�,��ôһ���ܷ���һ���ӽڵ�
				chosen_child = bestChild(root, feasible, feasible_count);
			}
			// ��չ����ڵ�
			top[chosen_child]--;
			// ����˵����,���������Ϸ���һ��λ��
			if (board[top[chosen_child]][chosen_child] == GRID_NO) {
				top[chosen_child]--;
			}
			// ����
			last_x = top[chosen_child];
			last_y = chosen_child;
			board[top[chosen_child]][chosen_child] = current_player;

			if (node) {
				return node;
			}
			else {
				// ����������̽��
				root = root->child[chosen_child];
			}
		}
		else {
			// û�н�һ���Ŀ���չ�ڵ���,��������ڵ�
			return root;
		}

		if (win_flag) {
			return root;
		}

		// ������һ�����ӷ�
		current_player = PLAYER_ADD - current_player;
	}
}

/*
	BestChild Function in UCT
	�ҳ������ڵ�����ؽ��̽�����ӽڵ�,��Ҫע�����,���������������ʱ�������root�ڵ�ĸ����ӽڵ�϶��Ƕ���̽������
*/
int bestChild(searchTreeNode *root, int *feasible, int feasible_count) {
	float logN = (float)log(root->num_visit);
	float max_val = -FLT_MAX;
	int max_ind = -1;
	float val = 0;
	int j = 0;
	for (int i = 0; i < feasible_count; i++) {
		j = feasible[i];
		val = ((float)root->child[j]->num_win / (float)root->child[j]->num_visit) \
			+ (FORMULA_C * sqrt(2 * logN / (float)root->child[j]->num_visit));
		if (val > max_val) {
			max_val = val;
			max_ind = j;
		}
	}
	return max_ind;
}


/*
	Expand Function in UCT
	��չĳ���ڵ����滹û��ѡ������ж�, �½�����ڵ㲢�ҷ��ش��ӽڵ��ָ��
	ͬʱΪ�˷���Ҳ���ؾ���ѡ���ӽڵ�ı��
*/
searchTreeNode* expand(searchTreeNode *node, int &chosen_child, int *feasible, int feasible_count) {
	for (int i = 0; i < feasible_count; i++) {
		if (!(node->child[feasible[i]])) {
			chosen_child = feasible[i];
			node->child[chosen_child] = newNode(node);
			return node->child[chosen_child];
		}
	}
	chosen_child = -1;
	return NULL;
}
