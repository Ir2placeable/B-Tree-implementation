/*
 * File processing, 2020
 * btree.cpp
 * implementation of B-tree
 */

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <stack>
#include <iostream>
#include <vector>

using namespace std;


/**
 * BTNode represents a single node in B-tree.
 */
typedef struct BTNode {
	int numofKeys;
	vector<int> Keys;
	vector<BTNode*> Ptrs;
} BTNode;

/**
 * getBTNode retruns a new BTNode.
 * @param m: number of branch points in B-tree
 * @return a new BTNode
 */
BTNode* getBTNode(int m) {
	BTNode* node = new BTNode;
	node->numofKeys = 0;
	node->Keys.resize(m);
	node->Ptrs.resize(m + 1, nullptr);

	return node;
}

typedef BTNode* BTree;


void insertNode(BTNode** node, int newKey);

void split(BTNode* T, BTNode** leftNode, BTNode** rightNode, int* midIndex)
{
	int n = T->numofKeys;
	*midIndex = n / 2;

	int i = 0;
	for (i; i < n / 2; i++)
	{
		insertNode(leftNode, T->Keys[i]);
		(*leftNode)->Ptrs[i] = T->Ptrs[i];
	}
	(*leftNode)->Ptrs[i] = T->Ptrs[i];

	int j = 0;
	for (i = n / 2 + 1; i < n; i++)
	{
		insertNode(rightNode, T->Keys[i]);
		(*rightNode)->Ptrs[j++] = T->Ptrs[i];
	}
	(*rightNode)->Ptrs[j] = T->Ptrs[i];
}

/**
 * insertBT inserts newKey into T.
 * @param T: a B-tree
 * @param m: number of branch points in B-tree
 * @param newKey: a key to insert
 */

void insertNode(BTNode** node, int newKey)
{
	if ((*node)->numofKeys == 0)
	{
		(*node)->Keys[0] = newKey;
		(*node)->numofKeys++;
		return;
	}
	int i = 0;
	while ((*node)->Keys[i] != NULL)
		i++;
	(*node)->Keys[i] = newKey;
	(*node)->numofKeys++;
	sort((*node)->Keys.begin(), (*node)->Keys.begin() + (*node)->numofKeys);

}
void insertBT(BTree* T, int m, int newKey) {

	// 노드가 없는 경우
	if (*T == NULL)
	{
		*T = getBTNode(m);
		insertNode(T, newKey);
		return;
	}

	BTNode* x = *T;
	BTNode* y = getBTNode(m);
	stack<BTNode*> stk;
	int i = 0;

	do {
		i = 0;
		// 삽입할 위치 i를 알아냄
		while (i < x->numofKeys && newKey > x->Keys[i])
			i++;
		// 삽입할 값이 이미 있는 경우
		if (i < x->numofKeys && newKey == x->Keys[i])
			return;

		stk.push(x);
	} while ((x = x->Ptrs[i]) != NULL);

	x = NULL;
	BTNode* tempNode = getBTNode(m);
	bool finished = false;

	x = stk.top();
	stk.pop();

	do
	{
		// 노드에 공간이 남는경우
		if (x->numofKeys < m - 1)
		{
			insertNode(&x, newKey);
			finished = true;
			break;
		}

		// 노드에 공간이 없는경우 split 해야한다.
		tempNode = x;
		int midIndex = 0;
		insertNode(&tempNode, newKey);

		BTNode* leftNode = getBTNode(m);
		BTNode* rightNode = getBTNode(m);
		split(tempNode, &leftNode, &rightNode, &midIndex);
		newKey = tempNode->Keys[midIndex];

		if (!stk.empty())
		{
			x = stk.top();
			stk.pop();

			int a = 0;
			while (a < x->numofKeys && newKey > x->Keys[a])
				a++;
			x->Ptrs[a] = leftNode;
			x->Ptrs.insert(x->Ptrs.begin() + a + 1, rightNode);
		}
		else {
			*T = getBTNode(m);
			insertNode(T, newKey);
			(*T)->Ptrs[0] = leftNode;
			(*T)->Ptrs[1] = rightNode;
			finished = true;
		}
	} while (!finished);

}


bool isLeafNode(BTNode* node)
{
	for (int i = 0; i < node->numofKeys + 1; i++)
	{
		if (node->Ptrs[i] != NULL)
			return false;
	}
	return true;
}

bool underFlow(BTNode* node, int m)
{
	if (node->numofKeys < (m - 1) / 2)
		return true;
	return false;
}

// 부모노드 node를 받아와서 형제노드 returnNode를 반환하는 함수
// broNodeIndex는 부모노드에서 형제노드의 위치
BTNode* bestSibling(BTNode* node, int oldKey, int m, int* broNodeIndex, int *myIndex)
{
	BTNode* returnNode = getBTNode(m);
	int n = node->numofKeys;

	int i = 0;
	while (i < n && node->Keys[i] <= oldKey)
		i++;
	*myIndex = i;

	if (i == 0)
	{
		returnNode = node->Ptrs[1];
		*broNodeIndex = 1;
	}
	else if (i == n)
	{
		returnNode = node->Ptrs[node->numofKeys - 1];
		*broNodeIndex = node->numofKeys - 1;
	}
	else {
		if (node->Ptrs[i - 1]->numofKeys > (m - 1) / 2)
		{
			returnNode = node->Ptrs[i - 1];
			*broNodeIndex = i - 1;
		}
		else if (node->Ptrs[i + 1]->numofKeys > (m - 1) / 2)
		{
			returnNode = node->Ptrs[i + 1];
			*broNodeIndex = i + 1;
		}
		else
		{
			returnNode = node->Ptrs[i - 1];
			*broNodeIndex = i - 1;
		}
	}
	if (returnNode == NULL)
		return getBTNode(m);
	return returnNode;
}

void deleteNode(BTNode** node, int oldKey)
{
	int i = 0;
	while ((*node)->Keys[i] != oldKey)
		i++;

	(*node)->Keys.erase((*node)->Keys.begin() + i);
	(*node)->numofKeys--;
	(*node)->Keys.push_back(NULL);
}
/**
 * deleteBT deletes oldKey from T.
 * @param T: a B-tree
 * @param m: number of branch points in B-tree
 * @param oldKey: a key to delete
 */
void deleteBT(BTree* T, int m, int oldKey) {

	stack<BTNode*> stk;

	BTNode* x = *T;
	int i = 0;
	do
	{
		i = 0;
		while (i<x->numofKeys && oldKey > x->Keys[i])
			i++;
		if (i < x->numofKeys && oldKey == x->Keys[i])
			break;

		stk.push(x);
	} while ((x = x->Ptrs[i]) != NULL);

	if (x == NULL)
		return;

	BTNode* internalNode = getBTNode(m);
	int change = oldKey;

	if (!isLeafNode(x))
	{
		internalNode = x;
		stk.push(x);
		x = x->Ptrs[i + 1];
		do
		{
			stk.push(x);
		} while ((x = x->Ptrs[0]) != NULL);
	}

	if (x == NULL)
	{
		
		x = stk.top();
		stk.pop();
		int temp = internalNode->Keys[i];
		internalNode->Keys[i] = x->Keys[0];
		x->Keys[0] = temp;
		change = internalNode->Keys[i];
	}

	bool finished = false;
	deleteNode(&x, oldKey);
	BTNode* y = getBTNode(m);
	y->numofKeys++;

	if (!stk.empty())
	{
		y = stk.top();
		stk.pop();
	}

	BTNode* brotherNode = getBTNode(m);
	int broNodeIndex = 0;
	do
	{
		int a = 0;
		brotherNode = bestSibling(y, change, m, &broNodeIndex, &a);
		// 현재노드 x에 여유가 있는 경우
		if (isLeafNode(*T) || !underFlow(x, m))
		{
			finished = true;
			break;
		}
		// 재분배
		// 현재노드 x에 여유가 없지만, 형제노드에 여유가 있는 경우
		else if (brotherNode->numofKeys > (m - 1) / 2)
		{
			// 형제노드가 왼쪽에 있는경우
			if (broNodeIndex < a)
			{
				int stolenKey = brotherNode->Keys[brotherNode->numofKeys - 1];
				insertNode(&x, y->Keys[broNodeIndex]);
				y->Keys[broNodeIndex] = stolenKey;

				x->Ptrs.insert(x->Ptrs.begin(), brotherNode->Ptrs[brotherNode->numofKeys]);
				brotherNode->Ptrs.erase(brotherNode->Ptrs.begin() + brotherNode->numofKeys);
				deleteNode(&brotherNode, stolenKey);
			}
			// 형제노드가 오른쪽에 있는경우
			else {
				int stolenKey = brotherNode->Keys[0];
				insertNode(&x, y->Keys[a]);
				y->Keys[a] = stolenKey;
				deleteNode(&brotherNode, stolenKey);

				x->Ptrs.insert(x->Ptrs.begin() + x->numofKeys, brotherNode->Ptrs[0]);
				brotherNode->Ptrs.erase(brotherNode->Ptrs.begin());
			}
			finished = true;
		}
		// 병합
		// 현재노드 x에 여유가 없고, 형제노드에도 여유가 없는 경우
		else {
			// 형제노드가 오른쪽에 있는경우
			if (broNodeIndex > a)
			{
				int stolenKey = y->Keys[a];
				y->Ptrs.erase(y->Ptrs.begin() + a);
				insertNode(&brotherNode, stolenKey);
				deleteNode(&y, stolenKey);

				brotherNode->Ptrs.insert(brotherNode->Ptrs.begin(), x->Ptrs[x->numofKeys]);
			}
			//형제노드가 왼쪽에 있는경우
			else
			{
				int stolenKey = y->Keys[broNodeIndex];
				y->Ptrs.erase(y->Ptrs.begin() + a);
				insertNode(&brotherNode, stolenKey);
				deleteNode(&y, stolenKey);

				brotherNode->Ptrs.insert(brotherNode->Ptrs.begin()+brotherNode->numofKeys, x->Ptrs[0]);
			}

			x = y;
			if (!stk.empty())
			{
				y = stk.top();
				stk.pop();
			}
			else
				finished = true;
		}
	} while (!finished);
	/* write your code here */
	if (y->numofKeys == 0)
	{
		*T = brotherNode;
	}
}

/**
 * inorderBT implements inorder traversal in T.
 * @param T: a B-tree
 */
void inorderBT(BTree T) {

	if (T == NULL)
		return;
	int i;
	for (i = 0; i < (*T).numofKeys; i++)
	{
		inorderBT((*T).Ptrs[i]);
		cout << (*T).Keys[i] << " ";
	}
	inorderBT((*T).Ptrs[i]);
}

int main() {
	/* do not modify the code below */

	int insertTestCases[] = { 40, 11, 77, 33, 20, 90, 99, 70, 88, 80, 66, 10, 22, 30, 44, 55, 50, 60, 100, 28, 18, 9, 5, 17, 6, 3, 1, 4, 2, 7, 8, 73, 12, 13, 14, 16, 15, 25, 24, 28, 45, 49, 42, 43, 41, 47, 48, 46, 63, 68, 61, 62, 64, 69, 67, 65, 54, 59, 58, 51, 53, 57, 52, 56, 83, 81, 82, 84, 75, 89 };
	int deleteTestCases[] = { 66, 10, 22, 30, 44, 55, 50, 60, 100, 28, 18, 9, 5, 17, 6, 3, 1, 4, 2, 7, 8, 73, 12, 13, 14, 16, 15, 25, 24, 28, 40, 11, 77, 33, 20, 90, 99, 70, 88, 80, 45, 49, 42, 43, 41, 47, 48, 46, 63, 68, 53, 57, 52, 56, 83, 81, 82, 84, 75, 89, 61, 62, 64, 69, 67, 65, 54, 59, 58, 51 };

	BTree T = NULL;

	for (int tC : insertTestCases) { insertBT(&T, 3, tC); inorderBT(T); printf("\n"); }
	for (int tC : deleteTestCases) { deleteBT(&T, 3, tC); inorderBT(T); printf("\n"); }
	cout << endl;

	T = NULL;

	for (int tC : insertTestCases) { insertBT(&T, 4, tC); inorderBT(T); printf("\n"); }
	for (int tC : deleteTestCases) { deleteBT(&T, 4, tC); inorderBT(T); printf("\n"); }
}