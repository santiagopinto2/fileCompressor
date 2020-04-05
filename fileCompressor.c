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
struct tokenNode* countOccurrences(struct tokenNode* head, char* newToken){
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
	if(dup==1)
		return head;
	struct tokenNode* newTokenNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
	newTokenNode->token=malloc(strlen(newToken)+1);
	strcpy(newTokenNode->token, newToken);
	newTokenNode->count=1;
	newTokenNode->next=NULL;
	tmp->next=newTokenNode;
	return head;
}
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
	return NULL;
}
int getIntLen(int num){
	int a=1;
	while(num>9){
		a++;
		num/=10;
	}
	return a;
}
struct tokenNode* parseCodebook(struct tokenNode* firstTokenNode, int codebook){
	int spaceCheck=0, start=0, counter=1, num, i;
	char* string;
	while((string=getToken(codebook))!=NULL){
		if(spaceCheck==0){
			int stringLen=strlen(string);
			char firstString[stringLen-1];
			firstString[stringLen-2]='\0';
			for(i=0;i<stringLen-2;i++)
				firstString[i]=string[i];
			if(start==0){
				if(stringLen>2&&string[stringLen-1]=='t'){
					start=1;
					num=atoi(firstString);
				}
			}
			else{
				if(counter==0){
					counter=1;
					num=atoi(firstString);
				}
				else if(counter==1){
					counter=0;
					if(firstTokenNode->token==NULL){
						firstTokenNode->token=malloc(strlen(firstString)+1);
						strcpy(firstTokenNode->token, firstString);
						firstTokenNode->count=num;
					}
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
void compress(char* fileName, struct tokenNode* firstTokenNode){
	char* string;
	int i;
	int file=open(fileName, O_RDONLY);
	int newFile=creat(strcat(fileName, ".hcz"), S_IRWXG|S_IRWXO|S_IRWXU);
	while((string=getToken(file))!=NULL){
		int stringLen=strlen(string), writeInt, countLen;
		struct tokenNode* tmp=firstTokenNode;
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
		if(firstReadNode->c==0){
			firstReadNode->c=buffer;
			code[0]=buffer;
		}
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
	close(file);
	close(newFile);
}
struct tokenNode* getOccurrences(char* fileName, struct tokenNode* firstCountNode){
	int file=open(fileName, O_RDONLY);
	int flag, i;
	char buffer;
	char* string;
	while((string=getToken(file))!=NULL){
		int stringLen=strlen(string);
		if(stringLen==1||stringLen==2)
			firstCountNode=countOccurrences(firstCountNode, string);
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
	return firstCountNode;
}
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
int isLeaf(struct huff* root) {
	if (root == NULL) return 0;
	if (root->left != NULL || root->right != NULL) return 0;
	return 1;
}
int tokenNodeLength(struct tokenNode* front) {
	struct tokenNode* ptr = front;
	int length = 0;
	while (ptr != NULL) {
		ptr = ptr->next;
		length++;
	}
	return length;
}
void huffcpy(struct huff* dest, struct huff* src) {
	dest->token = (char* ) malloc(strlen(src->token) * sizeof(char));
	strcpy(dest->token, src->token);
	dest->count = src->count;
	dest->left = src->left;
	dest->right = src->right;
}
void huffswap(struct huff* one, struct huff* two) {
	struct huff temp;
	huffcpy(&temp, one);
	huffcpy(one, two);
	huffcpy(two, &temp);
}
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
struct huff* popMin(struct huff* heap, int* length) {
	if (heap == NULL) return NULL;
	struct huff* popped = (struct huff* ) malloc(sizeof(struct huff));
	huffcpy(popped, heap);
	huffcpy(heap, heap + (*length)-1);
	*length = *length - 1;
	siftDown(heap, *length);
	return popped;
}
void printHeap(struct huff* heap, int length) {
	int i = 0;
	if (heap == NULL) return;
	while (i < length) {
		printf("%d %s   ", heap[i].count, heap[i].token);
		i++;
	}
	printf("\n");
}
void heapAppend(struct huff* heap, struct huff* newOne, int* length) {
	if (heap->token == NULL) *heap = *newOne;
	huffcpy(heap + *length, newOne);
	siftUp(heap, *length);
	*length = *length + 1;
}
void printHuff(struct huff* root) {
	if (root == NULL) return;
	printf(" ( ");
	printHuff(root->left);
	printf(" %s %d ", root->token, root->count);
	printHuff(root->right);
	printf(" ) ");
}
struct huff* combine(struct huff* first, struct huff* second) {
	struct huff* newTree = (struct huff* ) malloc(sizeof(struct huff));
	newTree->token = "";
	newTree->count = first->count + second->count;
	newTree->left = (struct huff* ) malloc(sizeof(struct huff));
	newTree->right = (struct huff* ) malloc(sizeof(struct huff));
	huffcpy(newTree->left, first);
	huffcpy(newTree->right, second);
	//printHuff(newTree);
	return newTree;
}
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
	return max;
}
void printBst(struct bst* tree) {
	if (tree == NULL) return;
	printf(" ( ");
	if (tree->left != NULL) printBst(tree->left);
	printf("%s %s", tree->token, tree->code);
	if (tree->right != NULL) printBst(tree->right);
	printf(" ) ");
}
void addBst(struct bst** tree, struct huff* huffPlace, char* code) {
	//printf("adding %s\n", code);
	if ((*tree)->token == NULL) {
		//printf("first one\n");
		(*tree) = (struct bst* ) malloc(sizeof(struct bst));
		(*tree)->token = (char* ) malloc(strlen(huffPlace->token) * sizeof(char));
		strcpy((*tree)->token, huffPlace->token);
		(*tree)->code = (char* ) malloc(strlen(code) * sizeof(char));
		strcpy((*tree)->code, code);
		(*tree)->left = NULL;
		(*tree)->right = NULL;
		//printBst((*tree));
		//printBst(*tree);
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
void addToArray(struct bst* array, struct bst* root, int* place, int length) {
	if (root == NULL) {
		return;
	}
	
	if (root->left != NULL) addToArray(array, root->left, place, length);
	*(array + *place) = *root;
	*place = *place + 1;
	if (root->right != NULL) addToArray(array, root->right, place, length);
}
struct bst* makeBalancedTree(struct bst* root, int tokenCount) {
	//CHECK IF BALANCED FIRST
	//if (isBalanced(root)) return root;
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
	/*int i = 0;
	printf("printing sorted list\n");
	for (i = 0; i < tokenCount; i++) {
		printf("%s %s\n", array[i].token, array[i].code);
	}*/
}
struct bst* buildBst(struct tokenNode* firstCountNode){
	int firstCountNodeLen=tokenNodeLength(firstCountNode);
	struct huff* heap=makeMinHeap(firstCountNode, firstCountNodeLen);
	struct huff* huffTree=makeHuffmanTree(heap, &firstCountNodeLen);
	struct bst* tree = makeBst(huffTree);
	//printBst(tree);
	//printf("Balanced: %d\n", isBalanced(tree));
	struct bst* balancedTree = makeBalancedTree(tree, tokenNodeLength(firstCountNode));
	//printBst(balancedTree);
	//printf("Balanced: %d\n", isBalanced(balancedTree));
	return balancedTree;
}
void writeHuffmanHelper2(int huffFile, struct bst* tree){
	if(tree==NULL)
		return;
	if(tree->left!=NULL)
		writeHuffmanHelper2(huffFile, tree->left);
	if(tree->right!=NULL)
		writeHuffmanHelper2(huffFile, tree->right);
	int treeTokenLen=strlen(tree->token), treeCodeLen=strlen(tree->code);
	char string[treeTokenLen+treeCodeLen+6];
	strcpy(string, tree->code);
	strcat(string, "\t");
	strcat(string, tree->token);
	strcat(string, "\n");
	string[treeTokenLen+treeCodeLen+5]='\0';
	
	
	
	
	
	printf("%s", string);
	
	
	
	
	
	int writeInt=write(huffFile, string, strlen(string)+1);
}
void writeHuffmanHelper(char* fileName, struct tokenNode* firstCountNode){
	struct bst* bsTree=buildBst(firstCountNode);
	int fileNameLen=strlen(fileName), rchrFileNameLen=strlen(strrchr(fileName, '/'));
	int codebookPathLen=fileNameLen-rchrFileNameLen+17;
	char codebookPath[codebookPathLen];
	strcpy(codebookPath, fileName);
	codebookPath[codebookPathLen-16]='\0';
	strcat(codebookPath, "HuffmanCodebook");
	int huffFile=creat(codebookPath, S_IRWXG|S_IRWXO|S_IRWXU);
	
	
	
	
	
	
	struct tokenNode* tmp=firstCountNode;
	/*while(tmp!=NULL){
		printf("%s\t%d\n", tmp->token, tmp->count);
		tmp=tmp->next;
	}
	printf("\n");
	printBst(bsTree);
	printf("\n\n");*/
	
	
	
	
	
	
	
	writeHuffmanHelper2(huffFile, bsTree);
	int writeInt=write(huffFile, "\n", 2);
	close(huffFile);	
}
void writeHuffman(struct tokenNode* firstCountNode, char* fileName){
	firstCountNode=getOccurrences(fileName, firstCountNode);
	writeHuffmanHelper(fileName, firstCountNode);
}
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
		}
		else if(strcmp(argv[2], "-b")==0){
			struct tokenNode* firstCountNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
			firstCountNode->token=NULL;
			firstCountNode->count=0;
			firstCountNode->next=NULL;
			recursiveWriteHuffman(argv[3], firstCountNode); 
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
	}
	else if(strcmp(argv[1], "-b")==0){
		struct tokenNode* firstCountNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
		firstCountNode->token=NULL;
		firstCountNode->count=0;
		firstCountNode->next=NULL;
		writeHuffman(firstCountNode, argv[2]);
	}
	
	
	else{
		/*
		 * 
		 * input error
		 * 
		 * 
		 */
		 
	}
	return EXIT_SUCCESS;
	
	
	/*
	struct tokenNode five = {.token = "five", .count = 1, .next = NULL};
	struct tokenNode four = {.token = "four", .count = 2, .next = &five};
	struct tokenNode three = {.token = "three", .count = 3, .next = &four};
	struct tokenNode two = {.token = "two", .count = 4, .next = &three};
	struct tokenNode one = {.token = "one", .count = 5, .next = &two};
	int length = 5;
	struct huff* heap = makeMinHeap(&one, length);
	
	struct huff six = {.token = "six", .count = 10, .left = NULL, .right = NULL};
	heapAppend(heap, &six, &length);
	
	struct huff* huffmanTree = makeHuffmanTree(heap, &length);
	printHuff(huffmanTree);
	printf("\n\n\n");
	
	struct bst* bintree = makeBst(huffmanTree);
	printBst(bintree);
	printf("\n");
	
	return EXIT_SUCCESS;
	* */
	
}
