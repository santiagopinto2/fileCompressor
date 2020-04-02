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
struct tokenNode* countOccurrences(struct tokenNode* head, char* newToken){
	if(head->token==NULL){
		head->token=malloc(strlen(newToken)+1);
		strcpy(head->token, newToken);
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
						//printf("%s\n", string);
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
		//printf("%s\n", string);
	}
	return firstTokenNode;
}
void compress(char *fileName, char* codebookFile){
	char* string;
	int start=0, counter=1, num, spaceCheck=0, i;
	int file=open(fileName, O_RDONLY);
	int codebook=open(codebookFile, O_RDONLY);
	int newFile=creat(strcat(fileName, ".hcz"), S_IRWXG|S_IRWXO|S_IRWXU);
	struct tokenNode* firstTokenNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
	firstTokenNode=parseCodebook(firstTokenNode, codebook);
	/*
	struct tokenNode* tmp1=firstTokenNode;
	while(tmp1!=NULL){
		//printf("Token %s with code %d\n", tmp1->token, tmp1->count);
		tmp1=tmp1->next;
	}*/
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
		//printf("%s\n", string);
	}
	close(file);
	close(codebook);
	close(newFile);
}
void decompress(char* fileName, char* codebookFile){
	//char codeStart[0];
	int fileLen=strlen(fileName);
	char newFileName[fileLen-3];
	int start=0, count=1, num, spaceCheck=0, i, flag, writeInt;
	char buffer;
	newFileName[fileLen-4]='\0';
	int file=open(fileName, O_RDONLY);
	int codebook=open(codebookFile, O_RDONLY);
	for(i=0;i<fileLen-4;i++)
		newFileName[i]=fileName[i];
	int newFile=creat(newFileName, S_IRWXG|S_IRWXO|S_IRWXU);
	struct tokenNode* firstTokenNode=(struct tokenNode*)malloc(sizeof(struct tokenNode));
	firstTokenNode=parseCodebook(firstTokenNode, codebook);
	struct readNode* firstReadNode=(struct readNode*)malloc(sizeof(struct readNode));
	firstReadNode->next=NULL;
	while((flag=read(file, &buffer, sizeof(buffer)))>0){
		count++;
		char code[count];
		//printf("%d\t%d\n", strlen(code), count);
		code[count-1]='\0';
		//printf("%d\n", strlen(code));
		if(firstReadNode->c==0){
			firstReadNode->c=buffer;
			code[0]=buffer;
			//printf("%s\n", code);
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
	close(codebook);
	close(newFile);
}
int main(int argc, char **argv){
	//going through directories recursively
	//printFiles(argv[1]);
	
	//compress a file given a codebook
	//compress(argv[1], argv[2]);
	
	//decompress a file given a codebook
	//decompress(argv[1], argv[2]);
	return EXIT_SUCCESS;
}
