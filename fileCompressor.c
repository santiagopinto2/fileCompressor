#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
struct readNode{
	char c;
	struct readNode* next;
};
struct tokenNode{
	char* token;
	int count;
	struct tokenNode* next;
};
struct huff {
	char* token;
	int count;
	struct huff* left;   
	struct huff* right;
};
struct bst {
	char* token;
	char* code;
	struct bst* left;
	struct bst* right;
};
void freeReadNodeList(struct readNode* firstReadNode){
	struct readNode* tmp;
	while(firstReadNode!=NULL){
		tmp=firstReadNode;
		firstReadNode=firstReadNode->next;
		free(tmp);
	}
}
void freeTokenNodeList(struct tokenNode* firstTokenNode){
	struct tokenNode* tmp;
	while(firstTokenNode!=NULL){
		tmp=firstTokenNode;
		firstTokenNode=firstTokenNode->next;
		free(tmp);
	}
}
void freeHuffman(struct huff* firstHuff){
	if(firstHuff!=NULL){
		freeHuffman(firstHuff->left);
		freeHuffman(firstHuff->right);
		free(firstHuff);
	}
}
void freeBST(struct bst* firstBST){
	if(firstBST!=NULL){
		freeBST(firstBST->left);
		freeBST(firstBST->right);
		free(firstBST);
	}
}
//gets length of an integer
int getIntLen(int num){
	int a=1;
	while(num>9){
		a++;
		num/=10;
	}
	return a;
}
//adds new tokens to a linked list and counts how many times it occurs in a file
struct tokenNode* countOccurrences(struct tokenNode* head, char* newToken){
	//if list is empty
	if(head->token==NULL){
		int newTokenLen=strlen(newToken)+1;
		head->token=malloc(newTokenLen);
		strcpy(head->token, newToken);
		head->token[newTokenLen-1]='\0';
		head->count=1;
		return head;
	}
	int dup=0;
	struct tokenNode* tmp=head;
	//checks rest of list
	while(tmp->next!=NULL){
		if(strcmp(tmp->token, newToken)==0){
				tmp->count++;
				dup=1;
				break;
		}
		tmp=tmp->next;
	}
	if(dup==0&&strcmp(tmp->token, newToken)==0){
		tmp->count++;
		dup=1;
	}
	//if the token wasn't new, return
	if(dup==1)
		return head;
	//if new node needs to be made
	struct tokenNode* newTokenNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
	newTokenNode->token=malloc(strlen(newToken)+1);
	strcpy(newTokenNode->token, newToken);
	newTokenNode->count=1;
	newTokenNode->next=NULL;
	tmp->next=newTokenNode;
	return head;
}
//parses through a file and returns a string for each token in it
//returns with the delimiter attached to the end of the token and is extracted later
char* getToken(int file){
	int flag, start=0, count=0;
	char buffer;
	struct readNode* firstReadNode=(struct readNode*)malloc(sizeof(struct readNode));
	while((flag=read(file, &buffer, sizeof(buffer)))>0){
		//if the token hasn't started yet
		if(start==0){
			if(buffer==' ')
				return " ";
			else if(buffer=='\n')
				return "\\n";
			else if(buffer=='\t')
				return "\\t";
			else{
				start=1;
				count++;
				firstReadNode->c=buffer;
				firstReadNode->next=NULL;
			}
		}
		//if the token has started
		else{
			//if the token is ending
			if(buffer==' '||buffer=='\n'||buffer=='\t'){
				start=0;
				char* s=malloc(count+3);
				s[count+2]='\0';
				if(buffer==' '){
					s[count]=' ';
					s[count+1]=' ';
				}
				else if(buffer=='\n'){
					s[count]='\\';
					s[count+1]='n';
				}
				else if(buffer=='\t'){
					s[count]='\\';
					s[count+1]='t';
				}
				count=0;
				struct readNode* tempReadNode=firstReadNode;
				while(tempReadNode->next!=NULL){
					s[count]=tempReadNode->c;
					count++;
					tempReadNode=tempReadNode->next;
				}
				s[count]=tempReadNode->c;
				freeReadNodeList(firstReadNode);
				return s;
				count=0;
			}
			//add to the token
			else{
				count++;
				struct readNode* newReadNode=(struct readNode*)malloc(sizeof(struct readNode));
				newReadNode->c=buffer;
				newReadNode->next=NULL;
				struct readNode* tempReadNode=firstReadNode;
				while(tempReadNode->next!=NULL)
					tempReadNode=tempReadNode->next;
				tempReadNode->next=newReadNode;
			}
		}
	}
	freeReadNodeList(firstReadNode);
	return NULL;
}
//parses through codebook and makes a linked list of every token and their code
struct tokenNode* parseCodebook(struct tokenNode* firstTokenNode, int codebook){
	int spaceCheck=0, start=0, counter=1, num, i;
	char* string;
	while((string=getToken(codebook))!=NULL){
		//space char is unique since it confuses the parser
		if(spaceCheck==0){
			int stringLen=strlen(string);
			char firstString[stringLen-1];
			firstString[stringLen-2]='\0';
			for(i=0;i<stringLen-2;i++)
				firstString[i]=string[i];
			//gets number first
			if(start==0){
				if(stringLen>2&&string[stringLen-1]=='t'){
					start=1;
					num=atoi(firstString);
				}
			}
			//gets token and creates node
			else{
				if(counter==0){
					counter=1;
					num=atoi(firstString);
				}
				else if(counter==1){
					counter=0;
					//if list is empty
					if(firstTokenNode->token==NULL){
						firstTokenNode->token=malloc(strlen(firstString)+1);
						strcpy(firstTokenNode->token, firstString);
						firstTokenNode->count=num;
					}
					//if list is not empty
					else{
						struct tokenNode* tmp=firstTokenNode;
						while(tmp->next!=NULL)
							tmp=tmp->next;
						struct tokenNode* newTokenNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
						if(stringLen==1){
							spaceCheck=1;
							newTokenNode->token=malloc(stringLen+1);
							newTokenNode->token=" ";
						}
						else{
							newTokenNode->token=malloc(strlen(firstString)+1);
							strcpy(newTokenNode->token, firstString);
						}
						newTokenNode->count=num;
						newTokenNode->next=NULL;
						tmp->next=newTokenNode;
					}
				}
			}
		}
		else
			spaceCheck=0;
	}
	return firstTokenNode;
}
//compress a file given a codebook in the form of a linked list
void compress(char* fileName, struct tokenNode* firstTokenNode){
	char* string;
	int i;
	int file=open(fileName, O_RDONLY);
	int newFile=creat(strcat(fileName, ".hcz"), S_IRWXG|S_IRWXO|S_IRWXU);
	//goes through every token
	while((string=getToken(file))!=NULL){
		int stringLen=strlen(string), writeInt, countLen;
		struct tokenNode* tmp=firstTokenNode;
		//if token is a space
		if(stringLen==1){
			while(tmp!=NULL){
				if(strcmp(tmp->token, " ")==0){
					countLen=getIntLen(tmp->count);
					char buffer[countLen+1];
					sprintf(buffer, "%d", tmp->count);
					writeInt=write(newFile, buffer, countLen);
				}
				tmp=tmp->next;
			}
		}
		//if token is \n or \t
		else if(stringLen==2){
			if(string[1]=='n'){
				while(tmp!=NULL){
					if(strcmp(tmp->token, "\\n")==0){
						countLen=getIntLen(tmp->count);
						char buffer[countLen+1];
						sprintf(buffer, "%d", tmp->count);
						writeInt=write(newFile, buffer, countLen);
					}
					tmp=tmp->next;
				}
			}
			else if(string[1]=='t'){
				while(tmp!=NULL){
					if(strcmp(tmp->token, "\\t")==0){
						countLen=getIntLen(tmp->count);
						char buffer[countLen+1];
						sprintf(buffer, "%d", tmp->count);
						writeInt=write(newFile, buffer, countLen);
					}
					tmp=tmp->next;
				}
			}
		}
		//if token is anything else
		else{
			char firstString[stringLen-1], secondString[3];
			firstString[stringLen-2]='\0';
			secondString[2]='\0';
			for(i=0;i<stringLen;i++){
				if(i<stringLen-2)
					firstString[i]=string[i];
				else
					secondString[stringLen-i-1]=string[i];
			}
			while(tmp!=NULL){
				if(strcmp(tmp->token, firstString)==0){
					countLen=getIntLen(tmp->count);
					char buffer[countLen+1];
					sprintf(buffer, "%d", tmp->count);
					writeInt=write(newFile, buffer, countLen);
				}
				else if((secondString[0]==' '&&strcmp(tmp->token, " ")==0)||(secondString[0]=='n'&&strcmp(tmp->token, "\\n")==0)||(secondString[0]=='t'&&strcmp(tmp->token, "\\t")==0)){
					countLen=getIntLen(tmp->count);
					char buffer[countLen+1];
					sprintf(buffer, "%d", tmp->count);
					writeInt=write(newFile, buffer, countLen);
				}
				tmp=tmp->next;
			}
		}
	}
	close(file);
	close(newFile);
}
//decompress a file given a codebook in the form of a linked list
void decompress(char* fileName, struct tokenNode* firstTokenNode){
	int fileLen=strlen(fileName);
	char newFileName[fileLen-3];
	int count=1, i, flag, writeInt;
	char buffer;
	newFileName[fileLen-4]='\0';
	int file=open(fileName, O_RDONLY);
	for(i=0;i<fileLen-4;i++)
		newFileName[i]=fileName[i];
	int newFile=creat(newFileName, S_IRWXG|S_IRWXO|S_IRWXU);
	struct readNode* firstReadNode=(struct readNode*)malloc(sizeof(struct readNode));
	firstReadNode->next=NULL;
	firstReadNode->c=0;
	while((flag=read(file, &buffer, sizeof(buffer)))>0){
		count++;
		char code[count];
		code[count-1]='\0';
		//if it's a new number representing a token
		if(firstReadNode->c==0){
			firstReadNode->c=buffer;
			code[0]=buffer;
		}
		//otherwise create a new node
		else{
			int count2=0;
			struct readNode* tmpReadNode=firstReadNode;
			while(tmpReadNode->next!=NULL){
				code[count2]=tmpReadNode->c;
				count2++;
				tmpReadNode=tmpReadNode->next;
			}
			code[count2]=tmpReadNode->c;
			code[count2+1]=buffer;
			struct readNode* newReadNode=(struct readNode*)malloc(sizeof(struct readNode));
			newReadNode->c=buffer;
			newReadNode->next=NULL;
			tmpReadNode->next=newReadNode;
		}
		int codeNum=atoi(code);
		struct tokenNode* tmpTokenNode=firstTokenNode;
		while(tmpTokenNode!=NULL){
			//if it finds a match in the huffman code
			if(tmpTokenNode->count==codeNum){
				if(strcmp(tmpTokenNode->token, "\\n")==0)
					writeInt=write(newFile, "\n", 1);
				else if(strcmp(tmpTokenNode->token, "\\t")==0)
					writeInt=write(newFile, "\t", 1);
				else
					writeInt=write(newFile, tmpTokenNode->token, strlen(tmpTokenNode->token));
				count=1;
				firstReadNode->c=0;
				firstReadNode->next=NULL;
				break;
			}
			tmpTokenNode=tmpTokenNode->next;
		}
	}
	freeReadNodeList(firstReadNode);
	close(file);
	close(newFile);
}
//shell to find the number of times a token occurs
struct tokenNode* getOccurrences(char* fileName, struct tokenNode* firstCountNode){
	int file=open(fileName, O_RDONLY);
	int flag, i;
	char buffer;
	char* string;
	while((string=getToken(file))!=NULL){
		int stringLen=strlen(string);
		//if the token is just a space, \n, or \t
		if(stringLen==1||stringLen==2){
			firstCountNode=countOccurrences(firstCountNode, string);
		}
		//if the token is anything and then has a space, \n, or \t at the end
		else{
			char firstString[stringLen-1], secondString[3];
			firstString[stringLen-2]='\0';
			secondString[2]='\0';
			for(i=0;i<stringLen;i++){
				if(i<stringLen-2)
					firstString[i]=string[i];
				else
					secondString[stringLen-i-1]=string[i];
			}
			firstCountNode=countOccurrences(firstCountNode, firstString);
			if(secondString[0]==' ')
				firstCountNode=countOccurrences(firstCountNode, " ");
			else if(secondString[0]=='n')
				firstCountNode=countOccurrences(firstCountNode, "\\n");
			else if(secondString[0]=='t')
				firstCountNode=countOccurrences(firstCountNode, "\\t");
		}
	}
	close(file);
	return firstCountNode;
}
//shell to perform compress and decompress recursively
void recursiveComDecom(char* command, char* directory, struct tokenNode* firstTokenNode){
    struct dirent *dp;
    DIR *dir = opendir(directory);
    if (!dir)
        return;
    while ((dp=readdir(dir))!=NULL){
		char* dName=dp->d_name;
        if (strcmp(dName, "..")!=0&&strcmp(dName, ".")!=0){
			char pathName[10000];
			strcpy(pathName, directory);
            strcat(pathName, "/");
            strcat(pathName, dName);
			char buffer;
			int flag, dNameLength=strlen(dName);
			int file=open(pathName, O_RDONLY);
			if(file!=-1){
				if(strcmp(command, "-c")==0&&dName[dNameLength-3]=='t'&&dName[dNameLength-2]=='x'&&dName[dNameLength-1]=='t')
					compress(pathName, firstTokenNode);
				else if(strcmp(command, "-d")==0&&dName[dNameLength-3]=='h'&&dName[dNameLength-2]=='c'&&dName[dNameLength-1]=='z')
					decompress(pathName, firstTokenNode);
			}
			close(file);
            recursiveComDecom(command, pathName, firstTokenNode);
        }
    }
    closedir(dir);
}
//Returns 1 if the huffman tree is a leaf node and 0 if it is an internal node
int isLeaf(struct huff* root) {
	if (root == NULL) return 0;
	if (root->left != NULL || root->right != NULL) return 0;
	return 1;
}
//This takes a pointer to a tokenNode as input and returns the amount of nodes until null is reached, from the front of a linked list, it will return the length
int tokenNodeLength(struct tokenNode* front) {
	struct tokenNode* ptr = front;
	int length = 0;
	while (ptr != NULL) {
		ptr = ptr->next;
		length++;
	}
	return length;
}
//This copies the data from a pointer to the src huffman tree to a pointer to the destination tree
void huffcpy(struct huff* dest, struct huff* src) {
	dest->token = (char* ) malloc(strlen(src->token) * sizeof(char));
	strcpy(dest->token, src->token);
	dest->count = src->count;
	dest->left = src->left;
	dest->right = src->right;
}
//Utilizes huffcpy to swap the values of two huffman trees between two pointers
void huffswap(struct huff* one, struct huff* two) {
	struct huff temp;
	huffcpy(&temp, one);
	huffcpy(one, two);
	huffcpy(two, &temp);
}
//When given a pointer to a heap of huffman tree nodes and the location of the inserted node, sift up is performed, all necessary swaps are performed
void siftUp(struct huff* heap, int place) { //parent formula is (i-1)/2
	int i = place;
	while ((i-1)/2 >= 0) {
		if (heap[i].count < heap[(i-1)/2].count) {
			huffswap(heap + i, heap + (i-1)/2);
			i = (i-1)/2;
		} else {
			break;
		}
	}
}
//When given a pointer to a heap of huffman tree nodes and its length, sift down is performed with all of its necessary swaps
void siftDown(struct huff* heap, int length) { //child formula is 2i+1 and 2i+2
	int i = 0;
	int min;
	while (i < min) {
		min = i;
		if (2*i + 1 < length) {
			min = 2*i + 1;
			if (2*i + 2 < length) {
				if (heap[2*i + 2].count < heap[min].count) {
					min = 2*i + 2;
				}
			}
		} else {
			return;
		}
		if (heap[i].count > heap[min].count) {
			huffswap(heap + i, heap + min);
			i = min;
		} else {
			return;
		}
	}
}
//When given a pointer to a linked list of tokens and its length, returns a pointer to the corresponding heap of huffman tree nodes
struct huff* makeMinHeap(struct tokenNode* list, int length) {
	struct huff* heap = (struct huff* ) malloc(length * sizeof(struct huff));
	if (length == 0 || heap == NULL || list == NULL) return NULL;
	heap[0].token = (char* ) malloc( strlen(list->token) * sizeof(char) );
	strcpy(heap[0].token, list[0].token);
	heap[0].count = list[0].count;
	heap[0].left = NULL;
	heap[0].right = NULL;
	if (length == 1) return heap;
	int place = 1;
	struct huff* current = &(heap[1]);
	struct tokenNode* ptr = list->next;
	while (ptr != NULL) {
		heap[place].token = (char* ) malloc( strlen(ptr->token) * sizeof(char) );
		strcpy(heap[place].token, ptr->token);
		heap[place].count = ptr->count;
		heap[place].left = NULL;
		heap[place].right = NULL;
		siftUp(heap, place);
		place++;
		ptr = ptr->next;
	}
	return heap;
}
//When given a pointer to a heap of huffman tree nodes and a pointer to its length, popMin moves the last element to the front and performs siftDown, then decreases the reference of the length pointer and returns the original first element of the heap
struct huff* popMin(struct huff* heap, int* length) {
	if (heap == NULL) return NULL;
	struct huff* popped = (struct huff* ) malloc(sizeof(struct huff));
	huffcpy(popped, heap);
	huffcpy(heap, heap + (*length)-1);
	*length = *length - 1;
	siftDown(heap, *length);
	return popped;
}
//For testing purposes: When given a pointer to a heap (or regular array) of huffman tree nodes and its length, traverses the list of huffman tree nodes and prints each node's count and token separated by a space
void printHeap(struct huff* heap, int length) {
	int i = 0;
	if (heap == NULL) return;
	while (i < length) {
		printf("%d %s   ", heap[i].count, heap[i].token);
		i++;
	}
	printf("\n");
}
//When given a pointer to a heap of huffman tree nodes, a pointer to the to-be-added huffman tree node, and a pointer to the heap's length, adds an element to the end of the heap and performs a sift up, then increases the reference of the length pointer
void heapAppend(struct huff* heap, struct huff* newOne, int* length) {
	if (heap->token == NULL) *heap = *newOne;
	huffcpy(heap + *length, newOne);
	siftUp(heap, *length);
	*length = *length + 1;
}
//For testing purposes: When given a pointer to a huffman tree node, traverses the tree and prints all of the nodes' tokens and counts with parenthesis showing the outer bounds of each subtree, does not print new line character at the end
void printHuff(struct huff* root) {
	if (root == NULL) return;
	printf(" ( ");
	printHuff(root->left);
	printf(" %s %d ", root->token, root->count);
	printHuff(root->right);
	printf(" ) ");
}
//When given pointers to two huffman tree nodes, return a pointer to a new huffman tree node with "" as the token, and the count of the first and second, its left tree will be the first huffman tree, and its right will be the second huffman tree
struct huff* combine(struct huff* first, struct huff* second) {
	struct huff* newTree = (struct huff* ) malloc(sizeof(struct huff));
	newTree->token = "";
	newTree->count = first->count + second->count;
	newTree->left = (struct huff* ) malloc(sizeof(struct huff));
	newTree->right = (struct huff* ) malloc(sizeof(struct huff));
	huffcpy(newTree->left, first);
	huffcpy(newTree->right, second);
	return newTree;
}
//When given a pointer to a heap of huffman tree nodes, and a pointer to its length, utilizes popMin and heapAppend to construct the corresponding huffman tree, returns a pointer to this huffman tree
struct huff* makeHuffmanTree(struct huff* heap, int* length) {
	if (*length == 0 || heap == NULL) return NULL;
	if (*length == 1) return heap;
	struct huff* first = popMin(heap, length);
	struct huff* second = popMin(heap, length);
	struct huff* newest = combine(first, second);
	heapAppend(heap, newest, length);
	while (*length > 1) {
		first = popMin(heap, length);
		second = popMin(heap, length);
		newest = combine(first, second);
		heapAppend(heap, newest, length);
	}
	return heap;
}
//When given a pointer to a binary search tree, returns the max height of its two child tree nodes plus one
int getHeight(struct bst* root) {
	int max = 0;
	if (root == NULL) return -1;
	int leftHeight = getHeight(root->left);
	int rightHeight = getHeight(root->right);
	if (leftHeight >= rightHeight) {
		max = leftHeight;
	} else {
		max = rightHeight;
	}
	return max + 1;
}
//When given a pointer to a binary search tree node, perform inorder traversal and print each node's token and huffman code, each node will be sorrounded by parenthesis
void printBst(struct bst* tree) {
	if (tree == NULL) return;
	printf(" ( ");
	if (tree->left != NULL) printBst(tree->left);
	printf("%s %s", tree->token, tree->code);
	if (tree->right != NULL) printBst(tree->right);
	printf(" ) ");
}
//When given a pointer to a pointer to a binary search tree, a pointer to the huffman tree node to-be-added, and the code of this new node, the new node is added to the binary search tree in the correct place based on the value of its token
void addBst(struct bst** tree, struct huff* huffPlace, char* code) {
	if ((*tree)->token == NULL) {
		(*tree) = (struct bst* ) malloc(sizeof(struct bst));
		(*tree)->token = (char* ) malloc(strlen(huffPlace->token) * sizeof(char));
		strcpy((*tree)->token, huffPlace->token);
		(*tree)->code = (char* ) malloc(strlen(code) * sizeof(char));
		strcpy((*tree)->code, code);
		(*tree)->left = NULL;
		(*tree)->right = NULL;
		return;
	}
	struct bst* ptr = *tree;
	while (ptr != NULL) {
		if (strcmp(huffPlace->token, ptr->token) > 0) {
			if (ptr->right == NULL) {
				ptr->right = (struct bst* ) malloc(sizeof(struct bst));
				ptr->right->token = (char* ) malloc(strlen(huffPlace->token) * sizeof(char));
				strcpy(ptr->right->token, huffPlace->token);
				ptr->right->code = (char* ) malloc(strlen(code) * sizeof(char));
				strcpy(ptr->right->code, code);
				ptr->right->left = NULL;
				ptr->right->right = NULL;
				return;
			}
			ptr = ptr->right;
		} else {
			if (ptr->left == NULL) {
				ptr->left = (struct bst* ) malloc(sizeof(struct bst));
				ptr->left->token = (char* ) malloc(strlen(huffPlace->token) * sizeof(char));
				strcpy(ptr->left->token, huffPlace->token);
				ptr->left->code = (char* ) malloc(strlen(code) * sizeof(char));
				strcpy(ptr->left->code, code);
				ptr->left->left = NULL;
				ptr->left->right = NULL;
				return;
			}
			ptr = ptr->left;
		}
	}
}
//When given a pointer to a pointer to a binary search tree, a pointer to the current huffman tree node to be checked, and the current code, this function recursively traverses the huffman tree, building up the code for the nodes as it goes, and performs addBst on each leaf node of the huffman tree
void findtokens(struct bst** tree, struct huff* huffPlace, char* code) {
	//printf("finding %s\n", code);
	if (isLeaf(huffPlace)) {
		addBst(tree, huffPlace, code);
	} else {
		char* nextCode = malloc((strlen(code)+1) * sizeof(char));
		if (huffPlace->left != NULL) {
			strcpy(nextCode, code);
			strcat(nextCode, "0");
			findtokens(tree, huffPlace->left, nextCode);
		}
		if (huffPlace->right != NULL) {
			strcpy(nextCode, code);
			strcat(nextCode, "1");
			findtokens(tree, huffPlace->right, nextCode);
		}
	}
}
//When given a pointer to a huffman tree, this generates the codes for each element, and returns a pointer to the corresponding binary search tree, node lookup is to be done with the value of the token
struct bst* makeBst(struct huff* huffmanTree) {
	struct bst* tree = (struct bst* ) malloc(sizeof(struct bst));
	tree->token = NULL;
	if (huffmanTree == NULL) return NULL;
	if (isLeaf(huffmanTree)) {
		tree->token = (char* ) malloc(strlen(huffmanTree->token) * sizeof(char));
		strcpy(tree->token, huffmanTree->token);
		tree->code = "0";
		tree->left = NULL;
		tree->right = NULL;
	} else {
		if (huffmanTree->left != NULL) findtokens(&tree, huffmanTree->left, "0");
		if (huffmanTree->right != NULL) findtokens(&tree, huffmanTree->right, "1");
	}
	return tree;
}
//When given a pointer to the root of a binary search tree, returns 1 if the tree is balanced and 0 if the tree is not
int isBalanced(struct bst* root) {
	if (root == NULL) return 1;
	int dif = getHeight(root->right) - getHeight(root->left);
	if (dif > 1 || dif < -1) return 0;
	if (isBalanced(root->right) && isBalanced(root->left)) {
		return 1;
	} else {
		return 0;
	}
}
//When given a pointer to a pointer to a balanced binary search tree, a pointer to an array of binary search trees, and ints corresponding to the current start and end of the subarray, recursively adds midpoints of the array to the balanced binary search tree
void addNodes(struct bst** balancedPlace, struct bst* array, int start, int end) {
	if (start > end) return;
	int mid = (start + end)/2;
	//printf("%d %d %d\n", start, mid, end);
	*balancedPlace = (struct bst* ) malloc(sizeof(struct bst));
	**balancedPlace = array[mid];
	(*balancedPlace)->left = NULL;
	(*balancedPlace)->right = NULL;
	addNodes(&((*balancedPlace)->left), array, start, mid-1);
	addNodes(&((*balancedPlace)->right), array, mid+1, end);
}
//When given a pointer to an array of binary search tree nodes, a pointer to the root of the original binary search tree, a pointer to the current spot in the array, and the length of the final array, inorder traversal is performed on the bst to transform the pointer to the array into a pointer to a sorted array of binary search tree nodes
void addToArray(struct bst* array, struct bst* root, int* place, int length) {
	if (root == NULL) {
		return;
	}
	
	if (root->left != NULL) addToArray(array, root->left, place, length);
	*(array + *place) = *root;
	*place = *place + 1;
	if (root->right != NULL) addToArray(array, root->right, place, length);
}
//When given a pointer to the root of a binary search tree with the total token count, this returns a pointer to the root of a balanced binary search tree corresponding to the input
struct bst* makeBalancedTree(struct bst* root, int tokenCount) {
	//CHECK IF BALANCED FIRST
	if (isBalanced(root)) return root;
	if (root->right == NULL && root->left == NULL) {
		return root;
	}
	struct bst* balancedTree = (struct bst* ) malloc(sizeof(struct bst));
	balancedTree->token = NULL;
	struct bst* array = (struct bst* ) malloc(sizeof(struct bst) * tokenCount);
	int place = 0;
	addToArray(array, root, &place, tokenCount);
	addNodes(&balancedTree, array, 0, tokenCount-1);
	return balancedTree;
}
//creates an avl tree from a linked list of tokens to find their huffman code
struct bst* buildBst(struct tokenNode* firstCountNode){
	int firstCountNodeLen=tokenNodeLength(firstCountNode);
	struct huff* heap=makeMinHeap(firstCountNode, firstCountNodeLen);
	struct huff* huffTree=makeHuffmanTree(heap, &firstCountNodeLen);
	struct bst* tree = makeBst(huffTree);
	struct bst* balancedTree = makeBalancedTree(tree, tokenNodeLength(firstCountNode));
	freeHuffman(heap);
	return balancedTree;
}
//recusrively goes through a tree and writes it to a file
void writeHuffmanHelper2(int huffFile, struct bst* tree){
	if(tree==NULL)
		return;
	if(tree->left!=NULL)
		writeHuffmanHelper2(huffFile, tree->left);
	if(tree->right!=NULL)
		writeHuffmanHelper2(huffFile, tree->right);
	int treeTokenLen=strlen(tree->token), treeCodeLen=strlen(tree->code);
	char string[treeTokenLen+treeCodeLen+3];
	string[treeTokenLen+treeCodeLen+2]='\0';
	strcpy(string, tree->code);
	strcat(string, "\t");
	strcat(string, tree->token);
	strcat(string, "\n");
	int writeInt=write(huffFile, string, strlen(string));
}
//setups a file to write to and is the shell to recurse through a tree and write it
void writeHuffmanHelper(char* fileName, struct tokenNode* firstCountNode){
	struct bst* bsTree=buildBst(firstCountNode);
	int fileNameLen=strlen(fileName), rchrFileNameLen=strlen(strrchr(fileName, '/'));
	int codebookPathLen=fileNameLen-rchrFileNameLen+17;
	char codebookPath[codebookPathLen];
	strcpy(codebookPath, fileName);
	codebookPath[codebookPathLen-16]='\0';
	strcat(codebookPath, "HuffmanCodebook");
	int huffFile=creat(codebookPath, S_IRWXG|S_IRWXO|S_IRWXU);
	writeHuffmanHelper2(huffFile, bsTree);
	int writeInt=write(huffFile, "\n", 1);
	freeBST(bsTree);
	close(huffFile);	
}
//gets occurrences and sends it off
void writeHuffman(struct tokenNode* firstCountNode, char* fileName){
	firstCountNode=getOccurrences(fileName, firstCountNode);
	writeHuffmanHelper(fileName, firstCountNode);
}
//gets occurrences from all files in a directory
struct tokenNode* recursiveWriteHuffmanHelper(char* directory, struct tokenNode* firstCountNode){
	struct dirent *dp;
    DIR *dir = opendir(directory);
    if (!dir)
        return;
    while ((dp=readdir(dir))!=NULL){
		char* dName=dp->d_name;
        if (strcmp(dName, "..")!=0&&strcmp(dName, ".")!=0){
			char pathName[10000];
			strcpy(pathName, directory);
            strcat(pathName, "/");
            strcat(pathName, dName);
			char buffer;
			int flag, dNameLength=strlen(dName);
			int file=open(pathName, O_RDONLY);
			if(file!=-1&&dName[dNameLength-3]=='t'&&dName[dNameLength-2]=='x'&&dName[dNameLength-1]=='t')
				firstCountNode=getOccurrences(pathName, firstCountNode);
			close(file);
            recursiveWriteHuffmanHelper(pathName, firstCountNode);
        }
    }
    closedir(dir);
    return firstCountNode;
}
//gets occurrences and sends it off
void recursiveWriteHuffman(char* directory, struct tokenNode* firstCountNode){
	firstCountNode=recursiveWriteHuffmanHelper(directory, firstCountNode);
	writeHuffmanHelper(directory, firstCountNode);
}
int main(int argc, char **argv){
	if(strcmp(argv[1], "-R")==0){
		if(strcmp(argv[2], "-c")==0||strcmp(argv[2], "-d")==0){
			int codebook=open(argv[4], O_RDONLY);
			struct tokenNode* firstTokenNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
			firstTokenNode=parseCodebook(firstTokenNode, codebook);
			recursiveComDecom(argv[2], argv[3], firstTokenNode);
			close(codebook);
			freeTokenNodeList(firstTokenNode);
		}
		else if(strcmp(argv[2], "-b")==0){
			struct tokenNode* firstCountNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
			firstCountNode->token=NULL;
			firstCountNode->count=0;
			firstCountNode->next=NULL;
			recursiveWriteHuffman(argv[3], firstCountNode); 
			freeTokenNodeList(firstCountNode);
		}
		else{
			printf("There was an error in the input\n");
			return EXIT_FAILURE;
		}
	}
	else if(strcmp(argv[1], "-c")==0||strcmp(argv[1], "-d")==0){
		int codebook=open(argv[3], O_RDONLY);
		struct tokenNode* firstTokenNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
		firstTokenNode=parseCodebook(firstTokenNode, codebook);
		if(strcmp(argv[1], "-c")==0)
			compress(argv[2], firstTokenNode);
		else
			decompress(argv[2], firstTokenNode);
		close(codebook);
		freeTokenNodeList(firstTokenNode);
	}
	else if(strcmp(argv[1], "-b")==0){
		struct tokenNode* firstCountNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
		firstCountNode->token=NULL;
		firstCountNode->count=0;
		firstCountNode->next=NULL;
		writeHuffman(firstCountNode, argv[2]);
		freeTokenNodeList(firstCountNode);
	}
	else{
		printf("There was an error in the input\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
