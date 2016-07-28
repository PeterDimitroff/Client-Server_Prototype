#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MYPORT 4343
#define BACKLOG 10
#define NODECOUNT 30
#define ENDWORD "endword"


typedef struct dat_t
{
	int val;
	struct dat_t *next;
}dat;

typedef struct node_t
{
  int val;
  struct node_t *left;
  struct node_t *right;
  int height;	//ignored for binary tree
}node;

node *rootAVL = NULL;
node *rootBin = NULL;
dat *list = NULL;

//Menu & misc.:
int menu(int choice, int clientSock);
int getChoice();
int sendOptions(int clientSock);
dat *newList(dat *pHead);
node *newRandBinTree(node *pRootBin);
node *newAscendingBinTree(node *pRootBin);
node *newDescendingBinTree(node *pRootBin);
node *newRandAVLTree(node *pRootAVL);
node *newAscendingAVLTree(node *pRootAVL);
node *newDescendingAVLTree(node *pRootAVL);
void memError();
int searchTree(node *pRoot,int clientSock);
char *findTreeStr(node *pRoot, int clientSock);
int searchList(dat *pHead,int clientSock);
char *findListStr(dat *pHead, int clientSock);
int getSearchedValue(int clientSock);
//List:
dat *getHead();
int destroyList(dat* listHead);
dat *generateList();
int sendList(dat *listHead, int clientSock);
char *getList(dat *listHead);
dat *sortList(dat *listHead);
dat *findPrevMin(dat *list);
dat *findNode(int value, dat *pList);
//Binary Tree:
node *getRootBin();
int addToBinTree(int value, node **leaf);
node *genRandBinTree();
node *genAscendingBinTree();
node *genDescendingBinTree();
//AVL Tree
node *getRootAVL();
int max(int a, int b);
int height(node *N);
node* newNode(int key);
node *rightRotate(node *y);
node *leftRotate(node *x);
int getBalance(node *N);
node *genRandAVLTree();
node *genAscendingAVLTree();
node *genDescendingAVLTree();
//Both Trees
node *findLeaf(int value, node *leaf);
char *getTreeStr(node *root);
int sendTree(node *leaf, int clientSock);
char *printNode(node *leaf);
int destroyTree(node *leaf);
//Network
int getListenSock();
int serve(int clientSock);
int getRequest(int clientSock);
int sendText(int clientSock, char *buf);
int sendAll(int clientSock, char *buf);
int sendError(int clientSock);
int sendSuccess(int clientSock);


/************************************** AVL: **********************************/

node *getRootAVL()
{
	return rootAVL;
}

int max(int a, int b)
{
    return (a > b)? a : b;
}

int height(node *N)
{
    if (N == NULL)
        return 0;
    return N->height;
}

node* newNode(int key)
{
    node *pNode = (node*) malloc( sizeof(node) );
    if(pNode == NULL)
    	return NULL;
    pNode->val = key;
    pNode->left = NULL;
    pNode->right = NULL;
    pNode->height = 1;  // new node is initially added at leaf
    return pNode;
}

node *rightRotate(node *y)
{
	if(y == NULL)
		return NULL;
	
    node *x = y->left;
    node *T2 = x->right;
 
    // Perform rotation
    x->right = y;
    y->left = T2;
 
    // Update heights
    y->height = max(height(y->left), height(y->right))+1;
    x->height = max(height(x->left), height(x->right))+1;
 
    // Return new root
    return x;
}

node *leftRotate(node *x)
{
	if(x == NULL)
		return NULL;
	
    node *y = x->right;
    node *T2 = y->left;
 
    // Perform rotation
    y->left = x;
    x->right = T2;
 
    //  Update heights
    x->height = max(height(x->left), height(x->right))+1;
    y->height = max(height(y->left), height(y->right))+1;
 
    // Return new root
    return y;
}

int getBalance(node *N)
{
    if (N == NULL)
        return 0;
    return height(N->left) - height(N->right);
}

node* addToAVLTree(node *pNode, int key)
{
    /* 1.  Perform the normal BST rotation */
    if (pNode == NULL)
    {
		pNode = newNode(key);
        return pNode;
    }
 	
 	while(pNode->val == key)
 	{
 		free(pNode);
 		pNode == NULL;
 		pNode = newNode(rand() % 999);
 		if(pNode == NULL)
 		{
        	destroyTree(rootAVL);
        	return NULL;
        }
 	}
    if (key < pNode->val)
    {
        pNode->left  = addToAVLTree(pNode->left, key);
        if(pNode->left == NULL)
        {
        	destroyTree(rootAVL);
        	return NULL;
        }
	}
	else
	{
		pNode->right = addToAVLTree(pNode->right, key);
		if(pNode->right == NULL)
		{
        	destroyTree(rootAVL);
        	return NULL;
        }
	}
 
    /* 2. Update height of this ancestor node */
    pNode->height = max(height(pNode->left), height(pNode->right)) + 1;
 
    /* 3. Get the balance factor of this ancestor node to check whether
       this node became unbalanced */
    int balance = getBalance(pNode);
 
    // If this node becomes unbalanced, then there are 4 cases
 
    // Left Left Case
    if (balance > 1 && key < pNode->left->val)
        return rightRotate(pNode);
 
    // Right Right Case
    if (balance < -1 && key > pNode->right->val)
        return leftRotate(pNode);
 
    // Left Right Case
    if (balance > 1 && key > pNode->left->val)
    {
        pNode->left =  leftRotate(pNode->left);
        return rightRotate(pNode);
    }
 
    // Right Left Case
    if (balance < -1 && key < pNode->right->val)
    {
        pNode->right = rightRotate(pNode->right);
        return leftRotate(pNode);
    }
 
    /* return the (unchanged) node pointer */
    return pNode;
}

node *genRandAVLTree()
{
	node *pRootAVL = getRootAVL();
	int i = 0;
	int n;
	
	for(i = 0; i < NODECOUNT; i++)
	{
		n = rand() % 999;
		pRootAVL = addToAVLTree(pRootAVL, n);
	}
	
	return pRootAVL;
}

node *genAscendingAVLTree()
{
	node *pRootAVL = getRootAVL();
	int i = 0;
	for(i = 0; i < NODECOUNT; i++)
	{
		if( (pRootAVL = addToAVLTree(pRootAVL, i * 5)) == NULL)
		{
			if(destroyTree(pRootAVL) != 0)
				memError();
			return NULL;
		}
	}
	return pRootAVL;
}

node *genDescendingAVLTree()
{
	node *pRootAVL = getRootAVL();
	int i = 0;
	for(i = NODECOUNT; i > 0; i--)
	{
		if( (pRootAVL = addToAVLTree(pRootAVL, i * 5)) == NULL)
		{
			if(destroyTree(pRootAVL) != 0)
				memError();
			return NULL;
		}
	}
	return pRootAVL;
}


/********************************************* MENU: *****************************************/

int menu(int choice, int clientSock)
{
	dat *pHead = getHead();
	node *pRootBin = getRootBin();
	node *pRootAVL = getRootAVL();
	
	switch(choice)
	{
		//list
		case 1:
			list = newList(pHead);
			if( getHead() == NULL )
				return sendError(clientSock);
			else
				return sendSuccess(clientSock); 
			break;
		case 2:
			return sendList(pHead, clientSock);
			break;
		case 3:
			if(pHead == NULL)
				return sendText(clientSock, "There is no list.\n");
				
			if((list = sortList(pHead)) == NULL)
				return sendError(clientSock);
			else
				return sendSuccess(clientSock);
			break;
		//binary tree
		case 4:
			rootBin = newRandBinTree(pRootBin);
			if (getRootBin() == NULL)
				return sendError(clientSock);
			else
				return sendSuccess(clientSock);
			break;
		case 5:
			rootBin = newAscendingBinTree(pRootBin);
			if (getRootBin() == NULL)
				return sendError(clientSock);
			else
				return sendSuccess(clientSock);
			break;
		case 6:
			rootBin = newDescendingBinTree(pRootBin);
			if (getRootBin() == NULL)
				return sendError(clientSock);
			else
				return sendSuccess(clientSock);
			break;
		case 7:
			return sendTree(pRootBin, clientSock);
			break;
		//AVL tree
		case 8:
			rootAVL = newRandAVLTree(pRootAVL);
			if (getRootAVL() == NULL)
				return sendError(clientSock);
			else
				return sendSuccess(clientSock);
			break;
		case 9:
			rootAVL = newAscendingAVLTree(pRootAVL);
			if (getRootAVL() == NULL)
				return sendError(clientSock);
			else
				return sendSuccess(clientSock);
			break;
		case 10:
			rootAVL = newDescendingAVLTree(pRootAVL);
			if (getRootAVL() == NULL)
				return sendError(clientSock);
			else
				return sendSuccess(clientSock);
			break;
		case 11:
			return sendTree(pRootAVL, clientSock);
			break;
		//Search:
		case 12:
			return searchTree(pRootBin, clientSock);
			break;/////////////////////////////////////////////////
		case 13:
			return searchTree(pRootAVL, clientSock);
			break;
		case 14:///////////////////////////////////////////////////////////////
			return searchList(pHead, clientSock);
			break;
		//exit
		case 15:
			if(destroyList(pHead) != 0 || destroyTree(pRootBin) != 0 || destroyTree(pRootAVL) != 0)
				memError();
			sendText(clientSock, ENDWORD);
			close(clientSock);
			return 0;
		default:
			if(destroyList(pHead) != 0 || destroyTree(pRootBin) != 0 || destroyTree(pRootAVL) != 0)
				memError();
			sendText(clientSock, ENDWORD);
			close(clientSock);
			return 0;
	}
}

int sendOptions(int clientSock)
{
	char options[] = "\n*****\nOptions:\n1. Generate a list with 30 nodes.\n2. Print the list.\n3. Sort the list in ascending order.\n***\n4. Generate random binary tree.\n5. Generate ascending binary tree.\n6. Generate descending binary tree.\n7. Print the current binary tree.\n***\n8. Generate random AVL tree.\n9. Generate ascending AVL tree.\n10. Generate descending AVL tree.\n11. Print the current AVL tree.\n***\n12. Search the binary tree.\n13. Search the AVL tree.\n14. Search the list.\n***\n15. Exit.\n";
	
	return sendText(clientSock, options);
}

int searchTree(node *pRoot, int clientSock)
{
	char *found = NULL;
	int res = 0;
	
	if((found = findTreeStr(pRoot, clientSock)) == NULL)
		return sendError(clientSock);
	else
	{
		res = sendText(clientSock, found);
		free(found);
		found = NULL;
		return res;
	}
}

char *findTreeStr(node *pRoot, int clientSock)
{
	int sVal = 0;
	char *found = NULL;
	char *line = NULL;
	node *result = NULL;
	
	sVal = getSearchedValue(clientSock);
	if(pRoot == NULL)
	{
		if(( found = (char*) malloc(20) ) == NULL)
			return NULL;
		strcpy(found, "The tree is empty.\n\0");
		return found;
	}
	result = findLeaf(sVal, pRoot);
	if(result == NULL)
	{
		if(( found = (char*) malloc(26) ) == NULL)
			return NULL;
		strcpy(found, "The value was not found.\n\0");
		return found;
	}
	else
	{
		if(( found = (char*) malloc(107) ) == NULL)
			return NULL;
		memset(found, '\0', 107);
		strcat(found, "| Address   | Value | Left Pointer | Right Pointer |\n");
		line = printNode(result);
		if(line == NULL)
		{
			free(found);
			return NULL;
		}
		strcat(found, line);
		free(line);
		line = NULL;
		found[106] = '\0';
		return found;
	}
}

int searchList(dat *pHead, int clientSock)
{
	int res = 0;
	char *found = NULL;
	
	if((found = findListStr(pHead, clientSock)) == NULL)
		return sendError(clientSock);
	else
	{
		res = sendText(clientSock, found);
		free(found);
		return res;
	}
}

char *findListStr(dat *pHead, int clientSock)
{
	int sVal = 0;
	char *found = NULL;
	char *line = NULL;
	dat *result = NULL;
	
	sVal = getSearchedValue(clientSock);
	if(pHead == NULL)
	{
		if(( found = (char*) malloc(20) ) == NULL)
			return NULL;
		strcpy(found, "The list is empty.\n\0");
		return found;
	}
	result = findNode(sVal, pHead);
	if(result == NULL)
	{
		if(( found = (char*) malloc(26) ) == NULL)
			return NULL;
		strcpy(found, "The value was not found.\n\0");
		return found;
	}
	else
	{
		if(( found = (char*) malloc(75) ) == NULL)
			return NULL;
		strcat(found, "| Address   | Value | Next Address |\n");
		if(result->next != NULL)
			sprintf(line, "| %-9p |  %0.3d  | %-12p |\n", result, result->val, result->next);
		else
			sprintf(line, "| %-9p |  %0.3d  | %-12s |\n", result, result->val, "NULL");
		strcat(found, line);
		found[74] = '\0';
		return found;
	}
}

int getSearchedValue(int clientSock)
{
	long sVal = 0;
	if(sendText(clientSock, "Enter searched value:\n\0") < 0)
	{
		perror("Error sending message");
		if(destroyList(getHead()) != 0 || destroyTree(getRootBin()) != 0 || destroyTree(getRootAVL()) != 0)
			memError();
		close(clientSock);
		exit(9);
	}
	if(read(clientSock, (char*) &sVal, 4) != 4)
	{
		perror("recieving error");
		if(destroyList(getHead()) != 0 || destroyTree(getRootBin()) != 0 || destroyTree(getRootAVL()) != 0)
			memError();
		close(clientSock);
		exit(9);
	}
	sVal = ntohl(sVal);
	return sVal;
}

node *newRandBinTree(node *pRootBin)
{
	if(destroyTree(pRootBin) != 0)
		memError();
	rootBin = NULL;
	pRootBin = genRandBinTree();
	if(pRootBin == NULL)
		printf("Error while generating tree.\n");
	else
		printf("Random tree successfully generated.\n");
	return pRootBin;
}

node *newAscendingBinTree(node *pRootBin)
{
	if(destroyTree(pRootBin) != 0)
		memError();
	rootBin = NULL;
	pRootBin = genAscendingBinTree();
	if(pRootBin == NULL)
		printf("Error while generating tree.\n");
	else
		printf("Ascending tree successfully generated.\n");
	return pRootBin;
}

node *newDescendingBinTree(node *pRootBin)
{
	if(destroyTree(pRootBin) != 0)
		memError();
	rootBin = NULL;
	pRootBin = genDescendingBinTree();
	if(pRootBin == NULL)
		printf("Error while generating tree.\n");
	else
		printf("Descending tree successfully generated.\n");
	return pRootBin;
}


node *newRandAVLTree(node *pRootAVL)
{
	if(destroyTree(pRootAVL) != 0)
		memError();
	rootAVL = NULL;
	pRootAVL = genRandAVLTree();
	if(pRootAVL == NULL)
		printf("Error while generating tree.\n");
	else
		printf("Random tree successfully generated.\n");
	return pRootAVL;
}

node *newAscendingAVLTree(node *pRootAVL)
{
	if(destroyTree(pRootAVL) != 0)
		memError();
	rootAVL = NULL;
	pRootAVL = genAscendingAVLTree();
	if(pRootAVL == NULL)
		printf("Error while generating tree.\n");
	else
		printf("Ascending tree successfully generated.\n");
	return pRootAVL;
}

node *newDescendingAVLTree(node *pRootAVL)
{
	if(destroyTree(pRootAVL) != 0)
		memError();
	rootAVL = NULL;
	pRootAVL = genDescendingAVLTree();
	if(pRootAVL == NULL)
		printf("Error while generating tree.\n");
	else
		printf("Descending tree successfully generated.\n");
	return pRootAVL;
}


dat *newList(dat *pHead)
{
	if(destroyList(pHead) != 0)
		memError();
	
	if(( pHead = generateList() ) == NULL)
	{
		printf("Error while generating list.\n");
		destroyList(pHead);
		return NULL;
	}
	printf("List created successfully.\n");
	return pHead;
}

void memError()
{
	printf("Error while clearing memory.\n");
	exit(100);
}


/*************************************************	LIST	************************************************/
dat *getHead()
{
	return list;
}

int destroyList(dat *listHead)
{
	dat *p;
	if(listHead == NULL)
		return 0;
	p = listHead;
	while(p != NULL)
	{
		p = p->next;
		free(listHead);
		listHead = p;
	}
	return 0;
}

dat *generateList()
{
	dat *p = NULL;
	dat *listHead = NULL;
	int i = 0;
	
	
	for(i = 0; i < NODECOUNT; i++)
	{
		if(p != NULL)
		{
			p->next = (dat *) malloc(sizeof(dat));
			if(p->next == NULL)
				return NULL;
			p = p->next;
		}
		else
		{
			p = (dat *) malloc(sizeof(dat));
			if(p == NULL)
				return NULL;
			listHead = p;
		}
		p->val = rand() % 1000;
		p->next = NULL;
	}
	
	return listHead;
}

int sendList(dat *listHead, int clientSock)
{
	int result = 0;
	char *listInfo = getList(listHead);
	if(listInfo == NULL)
		return sendError(clientSock);
	result = sendText(clientSock, listInfo);
	free(listInfo);
	listInfo = NULL;
	return result;
}

char *getList(dat *listHead)
{
	int width = 12;
	dat *listNode = listHead;
	char *strList = NULL;
	char *line = NULL;
	char *temp = NULL;
	
	if(listNode == NULL)
	{
		if(( strList = (char*) malloc(20) ) == NULL)
			return NULL;
		memset(strList, '\0', 19);
		strcat(strList, "There is no list.\n\0");
		return strList;
	}
	
	if(( line = (char*) malloc(40) ) == NULL)
		return NULL;
		
	memset(line,'\0',39);
	strcat(line, "| Address   | Value | Next Address |\n");
	
	if(( strList = (char*) malloc(strlen(line)) ) == NULL)
		return NULL;
	memset(strList,'\0',39);
	strcat(strList, line);
	
	while(listNode != NULL)
	{
		if(listNode == NULL)
			break;
		memset(line,'\0',39);
		if(listNode->next != NULL)
			sprintf(line, "| %-*p |  %0.3d  | %-*p |\n", width-3, listNode, listNode->val, width, listNode->next);
		else
			sprintf(line, "| %-*p |  %0.3d  | %-*s |\n", width-3, listNode, listNode->val, width, "NULL");
		
		
		if((temp = (char*) realloc( strList, (strlen(strList) + strlen(line) + 1) )) == NULL)
		{
		
			free(strList);
			strList = NULL;
			return NULL;
		}
		strList = temp;
		strcat(strList, line);
		listNode = listNode->next;
	}
	free(line);
	line = NULL;
	return strList;
}

dat *sortList(dat *listHead)
{
	
	dat *temp = NULL;
	dat *minVal = NULL;
	dat *prevMinVal = NULL;
	
	if(listHead == NULL || listHead->next == NULL)
		return listHead;
	
	if(getHead() == NULL)
		return NULL;
		
	prevMinVal = findPrevMin(listHead);
	if(prevMinVal == NULL)
	{
		minVal = listHead;
		minVal->next = sortList(minVal->next);
	}
	else
	{
		minVal = prevMinVal->next;
		prevMinVal->next = listHead;
		temp = listHead->next;
		listHead->next = minVal->next;
		minVal->next = temp;
		
		minVal->next = sortList(minVal->next);
	}
	
	
	return minVal;
}

dat *findPrevMin(dat *list)
{
	dat *curr = list;
	dat *minVal = list;
	dat *prevMinVal = NULL;
	dat *prev = list;
	
	while(curr != NULL)
	{
		if(minVal->val > curr->val)
		{
			prevMinVal = prev;
			minVal = curr;
		}
		prev = curr;
		curr = curr->next;
	}
	
	return prevMinVal;
}

dat *findNode(int value, dat *pList)
{
	if(pList == NULL)
		return NULL; /*the tree is empty */

	while(pList->val != value)
	{
		pList = pList->next;
		if(pList == NULL)
			return NULL;
	}

	return pList;	/* the searched value is not found */
}


/********************************************    BINARY:    *****************************************/

node *getRootBin()
{
	return rootBin;
}

int destroyTree(node *leaf)
{
	
	if( leaf != NULL )
	{
	  destroyTree(leaf->left);
	  destroyTree(leaf->right);
	  free( leaf );
	  leaf = NULL;
	}
	return 0;
}

int addToBinTree(int value, node **leaf)
{
	if( *leaf == NULL )
	{
		*leaf = (node*) malloc( sizeof(node) );
		if(*leaf == NULL)
			return 2;
		
		(*leaf)->val = value;
		(*leaf)->left = NULL;
		(*leaf)->right = NULL;
		return 0;
	}
	else
		if(value < (*leaf)->val)
		{
			addToBinTree(value, &(*leaf)->left );
		}
		else
			if(value > (*leaf)->val)
			{
				addToBinTree( value, &(*leaf)->right );
			}
			else
				return 1;	/* to know when duplicating was attempted */
}

node *genRandBinTree()
{
	node *pRootBin = getRootBin();
	int i = 0;
	int n;
	int check = 0;
	for(i = 0; i < NODECOUNT; i++)
	{
		for(;;)
		{
			n = rand() % 999;
			check = addToBinTree(n, &pRootBin);
			if( check == 0)
				break;
			if(check == 2)
			{
				if(destroyTree(pRootBin) != 0)
					memError();
				return NULL;
			}
		}
	}
	return pRootBin;
}

node *genAscendingBinTree()
{
	node *pRootBin = getRootBin();
	int i = 0;
	for(i = 0; i < NODECOUNT; i++)
	{
		if( addToBinTree( i * 5, &pRootBin ) != 0 )
		{
			if(destroyTree(pRootBin) != 0)
				memError();
			return NULL;
		}
	}
	return pRootBin;
}

node *genDescendingBinTree()
{
	node *pRootBin = getRootBin();
	int i = NODECOUNT;
	for(i = NODECOUNT; i > 0; i--)
	{
		if( addToBinTree( i * 5, &pRootBin ) != 0 )
		{
			if(destroyTree(pRootBin) != 0)
				memError();
			return NULL;
		}
	}
	return pRootBin;
}

/********************************************    Both Trees:    **********************************************/

int sendTree(node *leaf, int clientSock)
{
	int result = 0;
	char *treeStr = getTreeStr(leaf);
	if(treeStr == NULL)
		return sendError(clientSock);
		
	result = sendText(clientSock, treeStr);
	free(treeStr);
	treeStr = NULL;
	return result;
}

char *getTreeStr(node *root) //MorrisTraversal
{
	char *treeStr = NULL;
	char *line = NULL;
	node *current = NULL;
	node *pre = NULL;

	if(root == NULL)
	{
		if(( treeStr = (char*) malloc(20) ) == NULL)
			return NULL;
		memset(treeStr, '\0', 20);
		strcpy(treeStr, "The tree is empty.\n\0");
		return treeStr;
	}
	
		
	if(( treeStr = (char*) malloc(54) ) == NULL)
		return NULL;
	memset(treeStr, '\0', 54);
	strcat(treeStr, "| Address   | Value | Left Pointer | Right Pointer |\n");

	current = root;
	while(current != NULL)
	{                 
		if(current->left == NULL)
		{
			line = printNode(current);
			if(( treeStr = (char*) realloc(treeStr, strlen(line) + strlen(treeStr) + 1) ) == NULL)
			{
				free(treeStr);
				treeStr = NULL;
				free(line);
				line = NULL;
				return NULL;
			}
			strcat(treeStr, line);
			free(line);
			line = NULL;
			
			current = current->right;      
		}    
		else
		{
			/* Find the inorder predecessor of current */
			pre = current->left;
			while(pre->right != NULL && pre->right != current)
				pre = pre->right;

			/* Make current as right child of its inorder predecessor */
			if(pre->right == NULL)
			{
				pre->right = current;
				current = current->left;
			} 
			else // Revert the changes made in if part to restore the original tree i.e., fix the right child of predecssor
			{
				pre->right = NULL;
				line = printNode(current);
				if(( treeStr = (char*) realloc(treeStr, strlen(line) + strlen(treeStr) + 1) ) == NULL)
				{
					free(treeStr);
					treeStr = NULL;
					free(line);
					line = NULL;
					return NULL;
				}
				strcat(treeStr, line);
				free(line);
				line = NULL;
				
				current = current->right;      
			} /* End of if condition pre->right == NULL */
		} /* End of if condition current->left == NULL*/
	} /* End of while */
	return treeStr;
}

char *printNode(node *leaf)
{
	char *nodeStr = NULL;
	int width = 12;
	if((nodeStr = (char*) malloc(54)) == NULL)
		return NULL;
	memset(nodeStr, '\0', 54);
		
	if(leaf->left != NULL && leaf->right != NULL)
		sprintf(nodeStr, "| %-*p |  %0.3d  | %-*p | %-*p |\n", width-3, leaf, leaf->val, width, leaf->left, width+1, leaf->right);
		
	if(leaf->left == NULL && leaf->right == NULL)
		sprintf(nodeStr, "| %-*p |  %0.3d  | %-*s | %-*s |\n", width-3, leaf, leaf->val, width, "NULL", width+1, "NULL");
		
	if(leaf->left == NULL && leaf->right != NULL)
			sprintf(nodeStr, "| %-*p |  %0.3d  | %-*s | %-*p |\n", width-3, leaf, leaf->val, width, "NULL", width+1, leaf->right);
			
	if(leaf->left != NULL && leaf->right == NULL)
		sprintf(nodeStr, "| %-*p |  %0.3d  | %-*p | %-*s |\n", width-3, leaf, leaf->val, width, leaf->left, width+1, "NULL");
	return nodeStr;
}

node *findLeaf(int value, node *leaf)
{
	if(leaf == NULL)
		return NULL; /*the tree is empty */

	if(value == leaf->val)
	  return leaf;
	else
		if(value < leaf->val)
			return findLeaf(value, leaf->left);
		else
	  		return findLeaf(value, leaf->right);

	return NULL;	/* the searched value is not found */
}
/************************************************** NETWORK *******************************************************/

int getListenSock()
{
	int optval = 1;
	struct sockaddr_in servAddr;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
      perror("ERROR opening socket");
      exit(1);
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	bzero((char *) &servAddr, sizeof(servAddr));
   
	servAddr.sin_family = AF_UNSPEC;
	servAddr.sin_addr.s_addr = INADDR_ANY;
	servAddr.sin_port = htons(MYPORT);
	
	if (bind(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
	{
      perror("ERROR on binding");
      exit(2);
	}
	
	return sockfd;
}

int getRequest(int clientSock)
{
	long choice;
	for(;;)
	{
		if( read(clientSock, &choice, 4) != 4)
		{
			perror("recieving error");
			close(clientSock);
			exit(7);
		}
		choice = ntohl(choice);
		if(choice < 1 || choice > 15)
			sendText(clientSock, "Choice must be a number between 1 and 15.\n");
		else
			break;
	}
	return choice;
}

int serve(int clientSock)
{
	dat *pHead = getHead();
	node *pRootBin = getRootBin();
	node *pRootAVL = getRootAVL();
	int choice = 0;
	int res = 0;
	
	choice = getRequest(clientSock);
	if(choice < 1)
	{
		perror("Error recieving");
		return 0;
	}
	
	return menu(choice, clientSock);
}

int sendText(int clientSock, char *buf)
{
	long len = strlen(buf);
	len = htonl(len);
	if( write(clientSock, &len, 4) != 4)
		perror("Sending Error");
	
	return sendAll(clientSock, buf);
}

int sendAll(int clientSock, char *buf)
{
	int len = strlen(buf);
    int bytesSent = 0;
    int bytesLeft = len;
    int n = 0;

    while(bytesSent < len)
    {
        n = write(clientSock, buf+bytesSent, bytesLeft);
        if (n < 0)
        	return 0;
        bytesSent += n;
        bytesLeft -= n;
    }
    return bytesSent;
}

int sendError(int clientSock)
{
	return sendText(clientSock, "Error. Request not accomplished.\n");
}
	
int sendSuccess(int clientSock)
{
	return sendText(clientSock, "Success. Request accomplished.\n");
}

int main()
{
	int listenSock = 0;
	int clientSock = 0;
	int clientLen = 0;
	int pid = 0;
	struct sockaddr_in clientAddr;

	listenSock = getListenSock();
	listen(listenSock,BACKLOG);
   	
	while (1)
	{
		printf("Parent: Listening for connections...\n");
		clientSock = accept(listenSock, (struct sockaddr *) &clientAddr, &clientLen);

		if (clientSock < 0)
		{
			perror("ERROR on accept");
			close(listenSock);
			exit(3);
		}

		pid = fork();
		if (pid < 0)
		{
			perror("ERROR on fork");
			close(clientSock);
			close(listenSock);
			exit(4);
		}
		if (pid == 0) //child
		{
			close(listenSock);
			printf("Child: Succesfully connected.\n");
			if(sendOptions(clientSock) == 0)
				perror("Error sending");
				
			for(;;)
			{
				if( serve(clientSock) == 0 )
					break;
			}
			
			if(destroyList(getHead()) != 0 || destroyTree(getRootBin()) != 0 || destroyTree(getRootAVL()) != 0)
				memError();
			close(clientSock);
			exit(0);
		}
		else
		{
			printf("Parent: Succesfully connected.\n");
			close(clientSock);
		}
   }
	return 0;
}



