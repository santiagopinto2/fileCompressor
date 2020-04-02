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
typedef struct minHeapNode {
	char* token;
	int count;
} heapNode;
typedef struct binaryTreeNode {
	char* data;
	treeNode* left;   
	treeNode* right;
} treeNode;
struct tokenNode* countOccurrences(struct tokenNode* head, char* newToken){
	if(head->token==NULL){
		head->token=newToken;
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
void getTokens(int file){
	int flag, start=0, count=0;
	char buffer;
	struct tokenNode* firstTokenNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
	struct readNode* firstReadNode=(struct readNode*)malloc(sizeof(struct readNode));
	while((flag=read(file, &buffer, sizeof(buffer)))>0){
		//if the token hasn't started yet
		if(start==0){
			//if it is an empty token
			if(buffer==' ')
				firstTokenNode=countOccurrences(firstTokenNode, " ");
			else if(buffer=='\n')
				firstTokenNode==countOccurrences(firstTokenNode, "\\n");
			else if(buffer=='\t')
				firstTokenNode==countOccurrences(firstTokenNode, "\\t");
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
				if(buffer==' ')
					firstTokenNode=countOccurrences(firstTokenNode, " ");
				else if(buffer=='\n')
					firstTokenNode==countOccurrences(firstTokenNode, "\\n");
				else if(buffer=='\t')
					firstTokenNode==countOccurrences(firstTokenNode, "\\t");
				start=0;
				char* s=malloc(count);
				count=0;
				struct readNode* tempReadNode=firstReadNode;
				while(tempReadNode->next!=NULL){
					s[count]=tempReadNode->c;
					count++;
					tempReadNode=tempReadNode->next;
				}
				s[count]=tempReadNode->c;
				firstTokenNode=countOccurrences(firstTokenNode, s);
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
	struct tokenNode* tmp=firstTokenNode;
	while(tmp!=NULL){
		printf("%s: %d\n", tmp->token, tmp->count);
		tmp=tmp->next;
	}
}
void printFiles(char *directory){
    struct dirent *dp;
    DIR *dir = opendir(directory);
    if (!dir)
        return;
    while ((dp=readdir(dir))!=NULL){
		char* dName=dp->d_name;
        if (strcmp(dName, "..")!=0&&strcmp(dName, ".")!=0){
			printf("%s\n", dName);
			char pathName[10000];
			strcpy(pathName, directory);
            strcat(pathName, "/");
            strcat(pathName, dName);
			char buffer;
			int flag, dNameLength=strlen(dName);
			int file=open(pathName, O_RDONLY);
			if(file!=-1&&dName[dNameLength-3]=='t'&&dName[dNameLength-2]=='x'&&dName[dNameLength-1]=='t'){
				getTokens(file);
				/*while((flag=read(file, &buffer, sizeof(buffer)))>0){
					printf("%c", buffer);
				}*/
			}
			close(file);
            printFiles(pathName);
        }
    }
    closedir(dir);
}
char* getToken(int file){
	int flag, start=0, count=0;
	char buffer;
	struct readNode* firstReadNode=(struct readNode*)malloc(sizeof(struct readNode));
	while((flag=read(file, &buffer, sizeof(buffer)))>0){
		//if the token hasn't started yet
		if(start==0){
			//if it is an empty token
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
				char* s=malloc(count+2);
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
	/*
	struct tokenNode* tmp=firstTokenNode;
	while(tmp!=NULL){
		printf("%s: %d\n", tmp->token, tmp->count);
		tmp=tmp->next;
	}
	*/
}
void compress(char *fileName, char* codeBookFile){
	int file=open(fileName, O_RDONLY);
	int codeBook=open(fileName, O_RDONLY);
	int newFile=creat(strcat(fileName, ".hcz"), S_IRWXG|S_IRWXO|S_IRWXU);
	while(getToken(file)!=NULL){
		printf("%s\n", getToken(file));
	}
	close(file);
	close(codeBook);
	close(newFile);
}
int isLeaf(treeNode* root) {
	if (root == NULL) return 0;
	if (root->left != NULL || root->right != NULL) return 0;
	return 1;
}
int tokenNodeLength(struct tokenNode* front) {
	struct tokenNode* ptr = front;
	int length == 0;
	while (ptr != NULL) {
		ptr = ptr->next;
		length++;
	}
	return length;
}
heapNode* makeMinHeap(struct tokenNode* list, int length) {
	heapNode* heap = (heapNode* ) malloc(length * sizeof(heapNode));
	if (length == 0 || heap == NULL || list == NULL) return NULL;
	heap[0].token = (char* ) malloc( strlen(list[0].token) * sizeof(char) );
	strcpy(heap[0].token, list[0].token);
	heap[0].count = list[0].count;
	if (length == 1) return heap;
	int place = 1;
	heapNode* current = heap[1];
	struct tokenNode* ptr = list[1];
	while (ptr != NULL) {
		heap[place].token = (char* ) malloc( strlen(list[place].token) * sizeof(char) );
		strcpy(heap[place].token, list[place].token);
		heap[place].count = list[place].count;
		siftUp(heap, place);
		siftDown(heap, length);
		place++;
	}
	return heap;
}
void siftUp(heapNode* heap, int place) {
	
}
void siftDown(heapNode* heap, int length) {

}
int main(int argc, char **argv){
	//going through directories recursively
	//printFiles(argv[1]);
	//compress a file given a codebook
	compress(argv[1], argv[2]);
	return EXIT_SUCCESS;
}
