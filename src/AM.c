#include "AM.h"
#include "bf.h"
#include "defn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


////////////////////////////////////////////////////////////////////////////////////////////////
/*
index block:
+----+-------+-------+-----+-------+
|char|counter|pointer|value|pointer|
+----+-------+-------+-----+-------+

data block:

+----+-------+---------+------+------+
|char|counter|ptr->next|value1|value2|
+----+-------+---------+------+------+


*/
///////////////////////////////////////////////////////////////////////////////////////////////
int AM_errno = AME_OK;

int fdno;
int opencnt;
int scanscnt;
int mycounter;
int mycflag;
int stacki;
int notEqualFlag;
int greaterFlag;
int counterNE;
int ft;
int x;
int leftmostptr,blockptr;

typedef struct{				//to struct gia to stack
	int stackArray[1000];
	int top;
}stackT;

typedef struct{				//to struct gia to ka8e stoixeio tou pinaka anoixtwn arxeiwn
	char attrType1;
	int attrLength1;
	char attrType2;
	int attrLength2;
	int fileDesc;
	int printc;
	stackT stack;
}Open;

typedef struct{				//to struct gia to ka8e stoixeio tou pinaka sarwsewn
	int fileDesc;
	int offset;
	int blockNum;
	int operation;
}Scans;

Open *OpenTable = NULL;
Scans *ScansTable = NULL;


void AM_Init() {

	BF_ErrorCode init_err;
	if ( (init_err = BF_Init(MRU)) != BF_OK){		//arxikopoihsh epipedou BF
		BF_PrintError(init_err);
		return;
	}
	mycounter=1; //iparxei idi mia timi sto indexblock kai meta mpainei kate fora pou ginetai split
	mycflag=0;
	x=0;
	fdno = 0;
	opencnt = 0;
	scanscnt = 0;
	notEqualFlag=0;
	greaterFlag=0;
	counterNE = 0;
	ft=0;
	OpenTable = malloc(20*sizeof(Open));
	int i;
	for(i=0; i<20; i++)	{
						//arxikopoihsh pinaka anoiktwn arxeiwn
		OpenTable[i].fileDesc=-1; 	//to -1 simainei pos einai adeio auth h thesh
		OpenTable[i].printc=0;
		OpenTable[i].stack.top = -1;
	}
		

	ScansTable = malloc(20*sizeof(Scans));
	for(i=0; i<20; i++)	{		//arxikopoihsh pinaka scan arxeiwn
		ScansTable[i].fileDesc=-1;	//to -1 simainei pos einai adeio auth h thesh
	}		
	
	return;

}

void pushStack (stackT *stack,int i ){
	
	if(stack->top < 999){			//eisagwgh timhs sto stack
		stack->top++;		
		stack->stackArray[stack->top]=i;		
	}
	else printf("STACK IS FULL!!\n");
		
}

int popStack(stackT *stack){			//pop timhs apo to stack

	if(stack->top >= 0){
		int x = stack->stackArray[stack->top];
		stack->top--;
		return x;
	}
	else {
		printf("STACK IS EMPTY!! \n");
		return -1;
	}
}

void print(char* data ,char type,int size,int offset){	//sunarthsh ektupwshs timhs analoga me ton typo
	
	if(type == INTEGER){
		int x;
		memcpy(&x,&data[offset],sizeof(int));
		printf(":%d|",x);
	}
	else if(type == FLOAT){
		float y;
		memcpy(&y,&data[offset],sizeof(float));
		printf(":%.1f|",y);	
	}
	else{ //string
		char* z;
		z = malloc(size);
		memcpy(z,&data[offset],size);
		printf(":%s|",z);
	}
}

void printAll(int fileDesc,int size1,int size2,char type1,char type2){	//sunarthsh ektupwshs olou tou B+ dentrou

	BF_Block *block;
	BF_Block_Init(&block);
	BF_GetBlock(fileDesc,0,block);			//pairnw ton plhroforiako kombo tou arxeiou
	char* infoData=BF_Block_GetData(block);
	int root;
	memcpy(&root,infoData+sizeof(char)+sizeof(int)+sizeof(char)+sizeof(int)+sizeof(int),sizeof(int));	//pairnw th riza tou arxeiou
	BF_UnpinBlock(block);	
	pushStack(&OpenTable[stacki].stack,root);		//kanw push sto stack th riza
	char c;
	int counter,ptr,offset1,offset2;
	while(OpenTable[stacki].stack.top != -1){
		offset1=0;
		offset2=0;
		int blockNum = popStack(&OpenTable[stacki].stack);  //kanw pop apo thn korufh tou stack
		BF_GetBlock(fileDesc,blockNum,block);		    //pairnw to block pou ekana pop	
		char* data = BF_Block_GetData(block);		    //pairnw ta dedomena tou
		memcpy(&c,data,sizeof(char));
		memcpy(&counter,data+sizeof(char),sizeof(int));
		if(c == 'i'){					//an einai indexblock
			printf(">>INDEXBLOCK %d\n",blockNum);
			printf("	>>indexblock info:");
			printf("char:%c",c);
			printf("|counter:%d|",counter);
			memcpy(&ptr,data+sizeof(char)+sizeof(int),sizeof(int));
			if(ptr != -1){				//o prwtos pointer mpainei sto stack
				pushStack(&OpenTable[stacki].stack,ptr);
			}
			printf("ptr1:%d|\n",ptr);
			offset1 = sizeof(char)+2*sizeof(int);
			int i;
			printf("	>>format[|key|pointer|]\n");
			for(i=0;i<counter;i++){				//ektupwnw ta stoixeia tou
				printf("		KEY");
				print(data,type1,size1,offset1);
				offset1+= size1;
				memcpy(&ptr,&data[offset1],sizeof(int));
				printf("ptr:%d|\n",ptr);
				pushStack(&OpenTable[stacki].stack,ptr);//kathe pointer pou diabazw mpainei sto stack
				offset1+=sizeof(int);
				
			}
		}
		else{							//an einai datablock
			printf(">>DATABLOCK---[%d]	\n",blockNum);
			int nextDblock;
			memcpy(&nextDblock,data+sizeof(char)+sizeof(int),sizeof(int));
			printf("	>>datablock info:");
			printf("char:%c",c);
			printf("|counter:%d|",counter);
			printf("next datablock:%d|\n",nextDblock);
			offset2+= sizeof(char) + 2*sizeof(int);
			printf("	>>format[|key|value|]");
			int j;
			for(j=0;j<counter;j++){				//ektupwnw sta stoixeia tou
				printf("\n");
				printf("		KEY");
				print(data,type1,size1,offset2);
				offset2+= size1;
				printf("VALUE");
				print(data,type2,size2,offset2);
				offset2+=size2;
				if(j==counter-1){printf("\n");}
			}
		}
	}
	BF_UnpinBlock(block);			//kanw unpin to block
}


int compareTypes(void * val1,char type,char *data,int offset,int size1){ //sunarthsh sugkrishs analoga me ton typo gia < kai >=
//to return 1 einai < 
//to return 2 >=
	
	switch(type){
		case STRING:
		{
			char *comparingValue = malloc(sizeof(char)*size1);
			memcpy(comparingValue,data+offset,size1);
			if((strcmp(val1,comparingValue))<0){
				return 1; // val1 < comparing value
			}	
			else {
				return 2; //val1 >= comparingvalue
			}break;
		}
		case INTEGER:
		{
			void* comparingValue=malloc(sizeof(int));
			memcpy(comparingValue,data+offset,sizeof(int));
			if(*(int*)val1 < *(int*)comparingValue){
				return 1;			
			}
			else {
				return 2;
			}break;
		}
		case FLOAT:
		{
			void* comparingValue=malloc(sizeof(float));
			memcpy(comparingValue,data+offset,sizeof(float));
			if(*(float*)val1 < *(float*)comparingValue){
				return 1;			
			}
			else {
				return 2;
			}break;
		}
	}
}



int compareTypesEqual(void * val1,char type,char *data,int offset,int size1){ //sunarthsh sugkrishs analoga me ton typo gia = kai !=
//to return 1 einai ==
//to return 0 !=
	
	switch(type){
		case STRING:
		{
			char *comparingValue = malloc(sizeof(char)*size1);
			memcpy(comparingValue,data+offset,size1);
			if((strcmp(val1,comparingValue))==0){
				return 1; // val1 = comparing value

			}	
			else {
				return 0; //val1 != comparingvalue
			}break;
		}
		case INTEGER:
		{
			void* comparingValue=malloc(sizeof(int));
			memcpy(comparingValue,data+offset,sizeof(int));
			if(*(int*)val1 == *(int*)comparingValue){
				return 1;			
			}
			else {
				return 0;
			}break;
		}
		case FLOAT:
		{
			void* comparingValue=malloc(sizeof(float));
			memcpy(comparingValue,data+offset,sizeof(float));
			if(*(float*)val1 == *(float*)comparingValue){
				return 1;			
			}
			else {
				return 0;
			}break;
		}
	}
}

int checkFull(int counter,int siz1,int siz2){ //sunarthsh gia ton an xwraei allh eggrafh mesa sto block eite datablock eite indexblock
	
	int freeSpace = BF_BLOCK_SIZE - counter*((siz1)+(siz2)) - sizeof(char) - 2*sizeof(int) ;
	if(((siz1)+(siz2))<=freeSpace)
		return 0; //xoraei
	else 
		return 1; //den xoraei
	
}

int returnOffset(char* datab,void* value1,int counter,int size1,int size2,char type1){ //sunarthsh pou epistrefei to offset pou prepei na mpei h eggrafh mesa sto block
	int offset1 = sizeof(char)+sizeof(int)+sizeof(int);
	int offset2 = size1 + size2;
	int offset = offset1;
	
	int i=0;
	for(i=0;i<counter;i++){
		if(compareTypes(value1,type1,datab,offset,size1)==1){ // <
			return offset;
		}
		else{ // >=
			offset += offset2; 
		}
	}
	return offset;
	
}


int returnOffsetScan(char* datab,void* value1,int counter,int size1,int size2,char type1){
	int offset1 = sizeof(char)+sizeof(int)+sizeof(int);
	int offset2 = size1 + size2;
	int offset = offset1;
	
	int i=0;
	for(i=0;i<counter;i++){
		if(compareTypes(value1,type1,datab,offset,size1)==1){ // <
			return offset;
		}
		else{ // >=
			if(compareTypesEqual(value1,type1,datab,offset,size1)==1){
				return offset;
			}
			else{
				offset += offset2; 
			}
		}
	}
	return offset;
	
}


int searchRoot (int fileDesc,int root,void * val1,char type,int size1){ //sunarthsh pou 3ekinaei apo to root kai epistrefei to datablock pou prepei na mpei h brisketai 
									//h eggrafh val1
	char c;
	int counter;
	BF_Block* Block;
	BF_Block_Init(&Block);
	BF_ErrorCode getBlock_err;
	if( (getBlock_err = BF_GetBlock(fileDesc,root,Block)) != BF_OK ){
		BF_PrintError(getBlock_err);
		AM_errno = AME_GETBLOCK_ERROR;
		return AM_errno;
	}
		
	//vazoyme to index block sto stack
	pushStack(&OpenTable[stacki].stack,root);

	int offset1 = sizeof(char)+sizeof(int)+sizeof(int); //to char kai to counter kai o deiktis;	
	int offset2 = size1+sizeof(int); //value kai deiktis
	int offset = offset1;
	char* indexData = BF_Block_GetData(Block);
	memcpy(&c,indexData,sizeof(char));
	memcpy(&counter,indexData+sizeof(char),sizeof(int));
	int blockNum;
	while(c == 'i'){			
		offset = offset1;
		int i;
		for(i=0;i<counter;i++){
			if(compareTypes(val1,type,indexData,offset,size1)==1){ //<
				memcpy(&blockNum,indexData+offset-sizeof(int),sizeof(int));
				//exoume to blockNum
			}	
			else if(compareTypes(val1,type,indexData,offset,size1)==2){ //>=
				if(i==counter-1){
					memcpy(&blockNum,indexData+offset+size1,sizeof(int));
				}
				offset += offset2;
			}
			
		}

		if(blockNum != -1){
			BF_ErrorCode unpin_err44;
			if( (unpin_err44 = BF_UnpinBlock(Block)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err44);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
			}
			
			BF_ErrorCode getBlock_err1;
			if( (getBlock_err1 = BF_GetBlock(fileDesc,blockNum,Block)) != BF_OK ){
				BF_PrintError(getBlock_err);
				AM_errno = AME_GETBLOCK_ERROR;
				return AM_errno;
			}
			indexData = BF_Block_GetData(Block);
			memcpy(&c,indexData,sizeof(char));
			memcpy(&counter,indexData+sizeof(char),sizeof(int));
			
			if(c=='i'){
				pushStack(&OpenTable[stacki].stack,blockNum);			
			}
		}else{				//an to blockNum einai -1 dhmiourgw kainourio datablock
			BF_Block *BlockNew;
			BF_Block_Init(&BlockNew);

			BF_ErrorCode alloc_err;
			if( (alloc_err = BF_AllocateBlock(fileDesc, BlockNew)) != BF_OK ){
				BF_PrintError(alloc_err);
				AM_errno = AM_ALLOC_ERROR;
				return AM_errno;
			}
			
			char *blockNewData = BF_Block_GetData(BlockNew);
			char datab = 'd';
			memcpy(blockNewData, &datab, sizeof(char));
			int block_cnt = 0;
			int ptr;
			memcpy(&ptr,indexData+offset+size1,sizeof(int));
			memcpy(blockNewData+sizeof(char), &block_cnt, sizeof(int));
			memcpy(blockNewData+sizeof(char)+sizeof(int),&ptr,sizeof(int));
			
			int blocks_no;
			BF_ErrorCode getcnt_err;
			if( (getcnt_err = BF_GetBlockCounter(fileDesc, &blocks_no)) != BF_OK ){
				BF_PrintError(getcnt_err);
				AM_errno = AME_GETBLOCK_COUNTER_ERROR;
				return AM_errno;
			}
			
			blocks_no--;
			memcpy(indexData+sizeof(char)+sizeof(int), &blocks_no, sizeof(int));
			BF_Block_SetDirty(Block);
			
			
			BF_Block_SetDirty(BlockNew);
			BF_ErrorCode unpin_err33;
			if( (unpin_err33 = BF_UnpinBlock(BlockNew)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err33);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
			}
		
			BF_ErrorCode unpin_err;
			if( (unpin_err = BF_UnpinBlock(Block)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
			}
			
			return blocks_no;
		}
	}
		
	BF_ErrorCode unpin_err11;
	if( (unpin_err11 = BF_UnpinBlock(Block)) != BF_OK ){		//kanw unpin to block
		BF_PrintError(unpin_err11);
		AM_errno = AM_UNPIN_ERROR;
		return AM_errno;
	}
	return blockNum;	


}

int insertIndexBlock(int fileDesc,int size1,int size2,void * value1,char type1,int newBlockPtr){ //sunarthsh eisagwghs timhs sto epipedo eurethriou

	BF_Block* Block;
	BF_Block_Init(&Block);
	//anevazoume sto index block th left most pou einai h proti thesi tou newDATAblock
	//vriskoume to index num
	
	//prepei na elenksoume an xoraei allh eggrafh
	int i; 
	while(OpenTable[stacki].stack.top >= 0){
		int indexnum;
		if( (indexnum = popStack(&OpenTable[stacki].stack)) != -1 ){/*poped element*/}
		int counter=0;
		BF_ErrorCode getBlock_err;
		if( (getBlock_err = BF_GetBlock(fileDesc,indexnum,Block)) != BF_OK ){
			BF_PrintError(getBlock_err);
			AM_errno = AME_GETBLOCK_ERROR;
			return AM_errno;
		}	

		char* indexdata = BF_Block_GetData(Block);
		memcpy(&counter,indexdata+sizeof(char),sizeof(int));
		//printf("COUNTER!!!!!!!!!!!!!! %d\n",counter);
		
		if(checkFull(counter,size1,sizeof(int))==0){ //an xoraei allh eggrafh to indexblock
			int positionOffset=returnOffset(indexdata,value1,counter,size1,sizeof(int),type1); //exoume th thesi 
			int remainingPairs = counter - ( ( positionOffset - (sizeof(char)+(2*sizeof(int))) ) / ( size1 + sizeof(int) ) ) ;
			char * helpBlock = malloc(sizeof(char)*remainingPairs*(size1+sizeof(int)) );		
			memcpy(helpBlock,indexdata+positionOffset,sizeof(char)*remainingPairs*(size1+sizeof(int))); //antigrafoume apo positionOffset mexri counter ta pair value1 value2			
			memcpy(indexdata+positionOffset,value1,size1); //vazoume sto datab tis kainourgies times
			memcpy(indexdata+positionOffset+size1,&newBlockPtr,sizeof(int));
			counter++;
			memcpy(indexdata+sizeof(char),&counter,sizeof(int)); //auksanoume to counter
			memcpy(indexdata+positionOffset+size1+sizeof(int),helpBlock,sizeof(char)*remainingPairs*(size1+sizeof(int))); //kai vazoume piso +1 thesi ta epomena value1 value2		
			
			
			OpenTable[stacki].stack.top=-1; //adeiazoume to stackb 
				
		}
		else{ //an den xoarei kai tha ginei split
			int middleOffset;
			int helpcnt = counter;
			char* helpnewblock=malloc(sizeof(char)+2*sizeof(int)+(helpcnt+1)*(size1+sizeof(int)));
			memcpy(helpnewblock,indexdata,sizeof(char)+2*sizeof(int)+(helpcnt)*(size1+sizeof(int)));
			int positionOffset=returnOffset(helpnewblock,value1,helpcnt,size1,sizeof(int),type1);
			//int remainingPairs = (helpcnt+1) - ( ( positionOffset - (sizeof(char)+(2*sizeof(int))) ) / ( size1 + sizeof(int) ) ) ;
			int remainingPairs = (helpcnt) - (  (positionOffset - sizeof(char) -2*sizeof(int))/(size1+size2)  );
			char * helpBlock = malloc(remainingPairs*(size1+sizeof(int)) );		
			memcpy(helpBlock,helpnewblock+positionOffset ,remainingPairs*(size1+sizeof(int))); //antigrafoume apopositionOffset mexri counter ta pair value1 value2		
			memcpy(helpnewblock+positionOffset,value1,size1); //vazoume sto helpnewblock tis kainourgies times
			memcpy(helpnewblock+positionOffset+size1,&newBlockPtr,sizeof(int));
			memcpy(helpnewblock+positionOffset+size1+sizeof(int),helpBlock,remainingPairs*(size1+sizeof(int))); //kai vazoume piso +1 thesi ta epomena value1 value2		
			middleOffset = ((helpcnt+1)/2)*(size1+sizeof(int)); //exoume to offset apo ti timi pou psaxnoume	
			
			int newCounter = (counter+1)/2;
			memcpy(indexdata+sizeof(char),&newCounter,sizeof(int));
			memcpy(indexdata+sizeof(char)+2*sizeof(int),helpnewblock+sizeof(char)+2*sizeof(int),newCounter*(size1+size2));

		
			//ftiksame to index data

			BF_Block* newIndexBlock;
			BF_Block_Init(&newIndexBlock);
			BF_ErrorCode alloc_err1;
			if( (alloc_err1 = BF_AllocateBlock(fileDesc, newIndexBlock)) != BF_OK ){
				BF_PrintError(alloc_err1);
				AM_errno = AM_ALLOC_ERROR;
				return AM_errno;
			}
			int newblockindex;
			BF_ErrorCode getBlockCounter_err;
			if((getBlockCounter_err=BF_GetBlockCounter(fileDesc,&newblockindex))!=BF_OK){
				BF_PrintError(getBlockCounter_err);
				AM_errno = AME_GETBLOCK_COUNTER_ERROR;
				return AM_errno;
			}
			newblockindex--;
			char * newIndexData = BF_Block_GetData(newIndexBlock);
			int newCounter2;
			if(counter % 2 ==0){newCounter2 = counter/2;}
			else{newCounter2 = ((counter+1)/2)-1;}
			char c = 'i';
			memcpy(newIndexData,&c,sizeof(char));
			memcpy(newIndexData + sizeof(char),&newCounter2,sizeof(int));
			memcpy(newIndexData + sizeof(char) + sizeof(int) , helpnewblock +sizeof(char)+2*sizeof(int)+ middleOffset + size1 ,  (newCounter2*(size1+sizeof(int)))+sizeof(int));

			BF_Block_SetDirty(newIndexBlock);					//shmeiwnw to block dirty
			BF_ErrorCode unpin_err34;
			if( (unpin_err34 = BF_UnpinBlock(newIndexBlock)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err34);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
			}
			
			if(OpenTable[stacki].stack.top == -1){
				
				BF_Block* newRootBlock;
				
				BF_Block_Init(&newRootBlock);
				
				BF_ErrorCode alloc_err2;
				if( (alloc_err2 = BF_AllocateBlock(fileDesc, newRootBlock)) != BF_OK ){
					BF_PrintError(alloc_err2);
					AM_errno = AM_ALLOC_ERROR;
					return AM_errno;
				}
				
				char * newRootData = BF_Block_GetData(newRootBlock);
				int inforoot;
				BF_ErrorCode getBlockCounter_err89;
				if((getBlockCounter_err89=BF_GetBlockCounter(fileDesc,&inforoot))!=BF_OK){
					BF_PrintError(getBlockCounter_err89);
					AM_errno = AME_GETBLOCK_COUNTER_ERROR;
					return AM_errno;
				}
				inforoot--;
				char c = 'i';
				memcpy(newRootData,&c,sizeof(char));
				int newRootCounter=1;			
				memcpy(newRootData+sizeof(char),&newRootCounter,sizeof(int));
				memcpy(newRootData+sizeof(char)+sizeof(int),&indexnum,sizeof(int));
				memcpy(newRootData+sizeof(char)+2*sizeof(int),helpnewblock+sizeof(char)+2*sizeof(int)+middleOffset,size1);	
				memcpy(newRootData+sizeof(char)+2*sizeof(int)+size1,&newblockindex,sizeof(int));

				
				
				
				BF_Block_SetDirty(newRootBlock);					//shmeiwnw to block dirty
				BF_ErrorCode unpin_err12;
				if( (unpin_err12 = BF_UnpinBlock(newRootBlock)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err12);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
				}

				BF_Block* newB; //info root
				BF_Block_Init(&newB);
				BF_ErrorCode getBlock_err88;
				if( (getBlock_err88 = BF_GetBlock(fileDesc,0,newB)) != BF_OK ){
					BF_PrintError(getBlock_err88);
					AM_errno = AME_GETBLOCK_ERROR;
					return AM_errno;
				}
				char * infoData = BF_Block_GetData(newB);
				memcpy(infoData+sizeof(char)+sizeof(int)+sizeof(char)+sizeof(int)+sizeof(int),&inforoot,sizeof(int));
				BF_Block_SetDirty(newB);					//shmeiwnw to block dirty
				BF_ErrorCode unpin_err123;
				if( (unpin_err123 = BF_UnpinBlock(newB)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err123);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
				}
			}
			else{

				memcpy(value1,helpnewblock+sizeof(char)+2*sizeof(int)+middleOffset,size1);

			}
				
		
		}

		BF_Block_SetDirty(Block);					//shmeiwnw to block dirty
		BF_ErrorCode unpin_err;
		if( (unpin_err = BF_UnpinBlock(Block)) != BF_OK ){		//kanw unpin to block
			BF_PrintError(unpin_err);
			AM_errno = AM_UNPIN_ERROR;
			return AM_errno;
		}
	}

	return 0;

}

int AM_CreateIndex(char *fileName, char attrType1, int attrLength1, char attrType2, int attrLength2) {
	
	if(attrType1==INTEGER){			//elegxos megethous orismatwn kai twn 2 attrTypes
		if(attrLength1 != sizeof(int)){
			AM_errno = AME_TYPE_ERROR;
			return AM_errno;
		}		
	}else if(attrType1 == FLOAT){
		if(attrLength1 != sizeof(float)){
			AM_errno = AME_TYPE_ERROR;
			return AM_errno;
		}
	}else if(attrType1 == STRING){
		if(!(attrLength1>=1 && attrLength1<=255)){
			AM_errno = AME_TYPE_ERROR;
			return AM_errno;
		}
	}

	if(attrType2 == INTEGER){
		if(attrLength2 != sizeof(int)){
			AM_errno = AME_TYPE_ERROR;
			return AM_errno;
		}		
	}else if(attrType2 == FLOAT){
		if(attrLength2 != sizeof(float)){
			AM_errno = AME_TYPE_ERROR;
			return AM_errno;
		}
	}else if(attrType2 == STRING){
		if(!(attrLength2>=1 && attrLength2<=255)){
			AM_errno = AME_TYPE_ERROR;
			return AM_errno;
		}
	} 

	BF_ErrorCode create_err;
	if( (create_err = BF_CreateFile(fileName)) != BF_OK ){		//dhmiourgia arxeiou
		BF_PrintError(create_err);
		return AME_FILE_ALREADY_EXISTS_ERROR;
	}

	

	int index;
	if( ( index = AM_OpenIndex(fileName)) != AME_OPEN_FILES_LIMIT_ERROR){ //anoigw to arxeio
		
	}else{
		AM_errno = AME_OPEN_FILES_LIMIT_ERROR;
		return AM_errno;
	}
	OpenTable[index].attrType1 = attrType1;			//eisagwgh ston pinaka twn anoixtwn arxeiwn
	OpenTable[index].attrLength1 = attrLength1;
	OpenTable[index].attrType2 = attrType2;
	OpenTable[index].attrLength2 = attrLength2;


	BF_Block *InfoBlock;
	BF_Block_Init(&InfoBlock);

	BF_ErrorCode alloc_err;
	if( (alloc_err = BF_AllocateBlock(OpenTable[index].fileDesc, InfoBlock)) != BF_OK ){
		BF_PrintError(alloc_err);
		AM_errno = AM_ALLOC_ERROR;
		return AM_errno;
	}
	//eisodos plhroforias sto block
	//mono to info block
	char *blockData;					//dhmiourgia plhroforiakou kombou
	blockData = BF_Block_GetData(InfoBlock);
	memcpy(blockData, &attrType1, sizeof(char));
	memcpy(blockData+sizeof(char), &attrLength1, sizeof(int));
	memcpy(blockData+sizeof(char)+sizeof(int), &attrType2, sizeof(char));
	memcpy(blockData+sizeof(char)+sizeof(int)+sizeof(char), &attrLength2, sizeof(int));
	memcpy(blockData+sizeof(char)+sizeof(int)+sizeof(char)+sizeof(int), &OpenTable[index].fileDesc, sizeof(int));

	BF_Block_SetDirty(InfoBlock);					//shmeiwnw to block dirty
	BF_ErrorCode unpin_err;
	if( (unpin_err = BF_UnpinBlock(InfoBlock)) != BF_OK ){		//kanw unpin to block
		BF_PrintError(unpin_err);
		AM_errno = AM_UNPIN_ERROR;
		return AM_errno;
	}

	int close_err;
	if( (close_err = AM_CloseIndex(OpenTable[index].fileDesc)) != AME_OK ){		//kleinw to arxeio
		AM_errno = close_err;
		return AM_errno;
	}
 	return AME_OK;
}


int AM_DestroyIndex(char *fileName) {

	int fd;
	BF_ErrorCode open_err;
	if( (open_err = BF_OpenFile(fileName, &fd)) != BF_OK){
		BF_PrintError(open_err);
		AM_errno = AME_OPEN_FILE_ERROR;
		return AM_errno;
	}
	BF_ErrorCode close_err;
	if( (close_err = BF_CloseFile(fd)) != BF_OK){
		BF_PrintError(close_err);
		AM_errno = AME_CLOSE_FILE_ERROR;
		return AM_errno;
	}
	int i;
	for(i=0; i<opencnt; i++){
		if(OpenTable[i].fileDesc == fd){
			AM_errno = AME_OPEN_FILE_ERROR;
			return AM_errno;
		}
	}
	for(i=0; i<scanscnt; i++){
		if(ScansTable[i].fileDesc == fd){
			AM_errno = AME_OPEN_FILE_SCAN_ERROR;
			return AM_errno;
		}
	}
	int success;	
	char* file=malloc(sizeof(strlen(fileName))+3);
	sprintf(file,"./%s",fileName);
	if ( (success = remove(file)) != 0){        //katastrefw to arxeio
		AM_errno = AME_FAIL_DESTROY_INDEX_ERROR;
		return AM_errno;
	}else{
		printf("...Success destroy index\n");
	}
	free(file);
  	return AME_OK;
}

int AM_OpenIndex (char *fileName) {

	int fd;
	BF_ErrorCode open_err;
	if(opencnt!=20){
		if( (open_err = BF_OpenFile(fileName, &OpenTable[opencnt].fileDesc)) != BF_OK){ //anoigw to arxeio
		BF_PrintError(open_err);
		AM_errno = AME_OPEN_FILES_LIMIT_ERROR;
		return AM_errno;
		}
	}	
	else{
		printf("MAX FILES 20\n");
		return -100;
	}
	int blocks_num;
	BF_ErrorCode getcnt, getblock_err;
	
	if( (getcnt = BF_GetBlockCounter(OpenTable[opencnt].fileDesc, &blocks_num)) == BF_OK ) //pairnw ton arithmo twn block ou exei to arxeio
	{
		if(blocks_num == 0){			//an einai 0 epistrwfw th thesh pou tha mpei ston pinaka
			int current_openindex = opencnt;
			opencnt++;	
  			return current_openindex;
		}else{					//alliws
			BF_Block *block;
			BF_Block_Init(&block);

			if( (getblock_err = BF_GetBlock(OpenTable[opencnt].fileDesc, 0, block)) != BF_OK){
				BF_PrintError(getblock_err);			//pairnw ton plhroforiako kombo tou arxeiou
				AM_errno = AME_GETBLOCK_ERROR;
				return AM_errno;
			}
	
			char *blockData;

			blockData = BF_Block_GetData(block);		//bazw ta dedomena tu ston pinaka twn anoiktwn arxeiwn
			memcpy(&(OpenTable[opencnt].attrType1), blockData, sizeof(char));
			memcpy(&(OpenTable[opencnt].attrLength1), blockData+sizeof(char), sizeof(int));
			memcpy(&(OpenTable[opencnt].attrType2), blockData+sizeof(char)+sizeof(int), sizeof(char));
			memcpy(&(OpenTable[opencnt].attrLength2), blockData+sizeof(char)+sizeof(int)+sizeof(char), sizeof(int));
			memcpy(blockData+sizeof(char)+sizeof(int)+sizeof(char)+sizeof(int), &OpenTable[opencnt].fileDesc, sizeof(int));
			int i;
			printf("fileName :%s\n",fileName);
			opencnt++;
			BF_ErrorCode unpin_err;
			if( (unpin_err = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
			}
			return opencnt-1;
		}
	}else{
		BF_PrintError(getcnt);
		AM_errno = AME_GETBLOCK_COUNTER_ERROR;
		return AM_errno;
	}
}


int AM_CloseIndex (int fileDesc) {
	
	int fd;
		fd=OpenTable[fileDesc].fileDesc;
	
	int i;
	for(i=0;i<scanscnt;i++){
		if(ScansTable[i].fileDesc== fd){
			AM_errno= AME_OPEN_FILE_SCAN_ERROR;
			return AM_errno;
		}
	}
	BF_ErrorCode close_err;
	if( (close_err = BF_CloseFile(fd)) != BF_OK){
		BF_PrintError(close_err);
		AM_errno =  AME_CLOSE_FILE_ERROR ;
		return AM_errno;
	}
	int j;
	//an vgei apo th for den iparxei
	for(i=0;i<opencnt;i++){
		if(OpenTable[i].fileDesc==fd){
			OpenTable[i].fileDesc=-1;
		for(j=i;j<opencnt;j++){		//kleinw ena arxeio kai kanw ta epomena stoixeia tou pinaka aristero shift gia na einai gematos mexri to opencnt
					OpenTable[j].fileDesc=OpenTable[j+1].fileDesc;
					OpenTable[j].attrType1=OpenTable[j+1].attrType1;
					OpenTable[j].attrLength1=OpenTable[j+1].attrLength1;
					OpenTable[j].attrType2=OpenTable[j+1].attrType2;
					OpenTable[j].attrLength2=OpenTable[j+1].attrLength2;
					OpenTable[j].printc = OpenTable[j+1].printc;
					OpenTable[j].stack = OpenTable[j+1].stack;
			}
			opencnt--;	//flag gia na katalaboume an einai adeio
		}
	}
	printf("...Success close index\n");	
	return AME_OK;
}


int AM_InsertEntry(int fileDesc, void *value1, void *value2) {

	int i;
	char data = 'd';
	char index = 'i';
	char type11;
	char type22;
	int size11;
	int size22;
	int newi;
	for(i=0;i<20;i++){
		if(OpenTable[i].fileDesc == fileDesc){
			OpenTable[i].printc++;
			newi=i;
			stacki=i;
			BF_Block* Block1;
			BF_Block_Init(&Block1);
				
			BF_ErrorCode getBlock_err;
			if( (getBlock_err = BF_GetBlock(fileDesc,0,Block1)) != BF_OK ){
				BF_PrintError(getBlock_err);
				AM_errno = AME_GETBLOCK_ERROR;
				return AM_errno;
			}
			
			char* infoData=BF_Block_GetData(Block1);
			char type1,type2;
			int size1,size2;
			memcpy(&type1,infoData,sizeof(char));
			memcpy(&size1,infoData+sizeof(char),sizeof(int));
			memcpy(&type2,infoData+sizeof(char)+sizeof(int),sizeof(char));
			memcpy(&size2,infoData+sizeof(char)+sizeof(int)+sizeof(char),sizeof(int));	
			type11=type1;type22=type2;size11=size1;size22=size2;	
			int blockNum;
			BF_ErrorCode getBlockCounter_err;
			if((getBlockCounter_err=BF_GetBlockCounter(fileDesc,&blockNum))!=BF_OK){
				BF_PrintError(getBlockCounter_err);
				AM_errno = AME_GETBLOCK_COUNTER_ERROR;
				return AM_errno;
			}
			if(blockNum == 0){
				AM_errno=AME_UNITIALIZED_FILE_ERROR;
				return AM_errno; 
			}
			else if(blockNum==1){ //Exoume to infoblock 
				
				BF_Block* Block;
				BF_Block_Init(&Block);
				
				
				BF_ErrorCode alloc_err;
				if( (alloc_err = BF_AllocateBlock(fileDesc,Block)) != BF_OK ){ //data block
					BF_PrintError(alloc_err);
					AM_errno = AM_ALLOC_ERROR;
					return AM_errno;
				}	
				int counter;
				char* blockData=BF_Block_GetData(Block);
				counter=1;
				int dbptr = -1;
				memcpy(blockData,&data,sizeof(char));							//character 'd' h 'i'
				memcpy(blockData+sizeof(char),&counter,sizeof(int));			//counter
				memcpy(blockData+sizeof(char)+sizeof(int), &dbptr, sizeof(int));//pointer to next datablock
				memcpy(blockData+sizeof(char)+(2*sizeof(int)),value1,size1);	//value1
				memcpy(blockData+sizeof(char)+(2*sizeof(int))+size1,value2,size2); //value2




				BF_Block_SetDirty(Block);
				BF_ErrorCode unpin_err;
				if( (unpin_err = BF_UnpinBlock(Block)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}
				
				BF_Block* rootBlock;
				BF_Block_Init(&rootBlock);
				
				BF_ErrorCode alloc_err1;
				if( (alloc_err1 = BF_AllocateBlock(fileDesc,rootBlock)) != BF_OK ){ //root block
					BF_PrintError(alloc_err1);
					AM_errno = AM_ALLOC_ERROR;
					return AM_errno;
				}	
				
				char* rootData=BF_Block_GetData(rootBlock);
				
				BF_ErrorCode getBlockCounter_err1;
				if((getBlockCounter_err1=BF_GetBlockCounter(fileDesc,&blockNum))!=BF_OK){
					BF_PrintError(getBlockCounter_err1);
					AM_errno = AME_GETBLOCK_COUNTER_ERROR;
					return AM_errno;
				}
				int blcknum = blockNum -2;
				int rootCounter;
				rootCounter=1;
				int left_pointer=-1;
				memcpy(rootData,&index,sizeof(char));												//character 'd' h 'i'
				memcpy(rootData+sizeof(char),&rootCounter,sizeof(int));								//counter
				memcpy(rootData+sizeof(char)+sizeof(int), &left_pointer, sizeof(int));				//left_pointer
				memcpy(rootData+sizeof(char)+sizeof(int)+sizeof(int),value1,size1);					//value
				memcpy(rootData+sizeof(char)+sizeof(int)+sizeof(int)+size1,&blcknum,sizeof(int));	//right_pointer
				
				


				BF_Block_SetDirty(rootBlock);
				BF_ErrorCode unpin_err2;
				if( (unpin_err2 = BF_UnpinBlock(rootBlock)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err2);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}


				//bazoume sto info block to index 
				blcknum++;
				memcpy(infoData+sizeof(char)+sizeof(int)+sizeof(char)+sizeof(int)+sizeof(int),&blcknum, sizeof(int));
				BF_Block_SetDirty(Block1);
				BF_ErrorCode unpin_err3;
				if( (unpin_err3 = BF_UnpinBlock(Block1)) != BF_OK ){		//kanw unpin to block1
					BF_PrintError(unpin_err3);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}
			}

			else{ //exoume dentro
				
				BF_Block* Block2;
				BF_Block_Init(&Block2);
				BF_ErrorCode getBlock_err22;
				if( (getBlock_err22 = BF_GetBlock(fileDesc,0,Block2)) != BF_OK ){
					BF_PrintError(getBlock_err22);
					AM_errno = AME_GETBLOCK_ERROR;
					return AM_errno;
				}
				
				int firstRoot;
				char* info=BF_Block_GetData(Block2);	//pairnoume to protoroot block
				memcpy(&firstRoot,info+sizeof(char)+sizeof(int)+sizeof(char)+sizeof(int)+sizeof(int),sizeof(int));
				


				BF_ErrorCode unpin_err22;
				if( (unpin_err22 = BF_UnpinBlock(Block2)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err22);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}
				int dataBNum=searchRoot(fileDesc,firstRoot,value1,type1,size1); //exoume to datablock	
				
				if( (getBlock_err = BF_GetBlock(fileDesc,dataBNum,Block2)) != BF_OK ){
					BF_PrintError(getBlock_err);
					AM_errno = AME_GETBLOCK_ERROR;
					return AM_errno;
				}
				char* datab = BF_Block_GetData(Block2);				
								

				//insert se block 
				
				int currCounter; //pernoume to counter apo zeugaria
				memcpy(&currCounter,datab+sizeof(char),sizeof(int)); //counter tou block data !!
				//printf("CURRCOUNTER DATABLOCK %d\n", currCounter);
				
				//enexoume an einai keno 
				if(currCounter == 0){
					memcpy(datab + sizeof(char) + sizeof(int) + sizeof(int) ,value1,size1);
					memcpy(datab + sizeof(char) + sizeof(int) + sizeof(int) + size1,value2,size2);
					currCounter++; //valame zeugari
					memcpy(datab+sizeof(char),&currCounter,sizeof(int));

					
					//adeiazome to staack 
								OpenTable[stacki].stack.top=-1;
				}
				else{
					if(checkFull(currCounter,size1,size2)==0){ //xoraei eggrafi 
			
						int positionOffset=returnOffset(datab,value1,currCounter,size1,size2,type1); //exoume th thesi 
						int remainingPairs = currCounter - ( ( positionOffset - (sizeof(char)+(2*sizeof(int))) ) / ( size1 + size2 ) ) ;
						char * helpBlock = malloc(sizeof(char)*remainingPairs*(size1+size2));
						
						memcpy(helpBlock,datab+positionOffset,sizeof(char)*remainingPairs*(size1+size2)); //antigrafoume apo positionOffset mexri counter ta pair value1 value2
						
						memcpy(datab+positionOffset,value1,size1); //vazoume sto datab tis kainourgies times
						memcpy(datab+positionOffset+size1,value2,size2);
						currCounter++;
						memcpy(datab+sizeof(char),&currCounter,sizeof(int)); //auksanoume to counter

						memcpy(datab+positionOffset+size1+size2,helpBlock,sizeof(char)*remainingPairs*(size1+size2)); //kai vazoume piso +1 thesi ta epomena value1 value2
					
								//adeiazome to staack 
								OpenTable[stacki].stack.top=-1;
					}
					
					else{ //split block 
						mycounter++;
						mycflag=1;
						//////////////////////////////////////////////////////////////////
						//////////////////// A R T I O S //////////////////////////////
						/////////////////////////////////////////////////
						if(currCounter % 2 == 0){ //artios
							BF_Block *newDataBlock;
							BF_Block_Init(&newDataBlock);
							BF_ErrorCode alloc_err111;
							if( (alloc_err111 = BF_AllocateBlock(fileDesc,newDataBlock)) != BF_OK ){ //root block
								BF_PrintError(alloc_err111);
								AM_errno = AM_ALLOC_ERROR;
								return AM_errno;
							}	
							char* newDatab = BF_Block_GetData(newDataBlock);	
							
							int positionOffset=returnOffset(datab,value1,currCounter,size1,size2,type1); //exoume th thesi 
							int remainingPairs = currCounter - ( ( positionOffset - (sizeof(char)+(2*sizeof(int))) ) / ( size1 + size2 ) ) ;
							int firstCounter = (currCounter+1)/2;
							int secondCounter = firstCounter + 1;
							int oldPtr;
							memcpy(&oldPtr,datab+sizeof(char)+sizeof(int),sizeof(int));
							memcpy(newDatab+sizeof(char)+sizeof(int),&oldPtr,sizeof(int));
							int newPtr;
							BF_ErrorCode getBlockCounter_err123;
							if((getBlockCounter_err123=BF_GetBlockCounter(fileDesc,&newPtr))!=BF_OK){
								BF_PrintError(getBlockCounter_err123);
								AM_errno = AME_GETBLOCK_COUNTER_ERROR;
								return AM_errno;
							}
							newPtr = newPtr-1;
							memcpy(datab+sizeof(char)+sizeof(int),&newPtr,sizeof(int));
							memcpy(datab+sizeof(char),&firstCounter,sizeof(int)); // to kanourgio counter sto first block	
							memcpy(newDatab+sizeof(char),&secondCounter,sizeof(int)); // to counter sto block 2 
							if(remainingPairs == 0){ //simainei pos prepei na mpei sto telos 
								char * helpBlock22 = malloc(sizeof(char)*(currCounter - firstCounter)*(size1+size2));
								memcpy(helpBlock22,datab+sizeof(char)+2*sizeof(int)+firstCounter*(size1+size2),sizeof(char)*(currCounter - firstCounter)*(size1+size2)); //antigrafoume apo firstCoutner mexri counter ta pair value1 value2
								memcpy(newDatab+sizeof(char)+sizeof(int)+sizeof(int),helpBlock22,(currCounter - firstCounter)*(size1+size2)); //valame ta dedonena sto kanourgio block
								memcpy(newDatab+sizeof(char)+sizeof(int)+sizeof(int)+(currCounter - firstCounter)*(size1+size2),value1,size1); //kolame sto telos to teleutaio stoixeio
								memcpy(newDatab+sizeof(char)+sizeof(int)+sizeof(int)+(currCounter - firstCounter)*(size1+size2)+size1,value2,size2);
								
								// INSERT STO INDEXBLOCK

								//anevazoume sto index block th left most pou einai h proti thesi tou newDATAblock
								
								int ar=insertIndexBlock( fileDesc, size1, sizeof(int), &newDatab[9], type1,newPtr);			
								
							}
							else{
								int rem = (positionOffset - (sizeof(char) + 2*sizeof(int)) ) / (size1 + size2);
								if((rem) < firstCounter){
									int offset2 = size1 +size2;
									int newRem = firstCounter-rem;;
									char * helpBlock1 = malloc(sizeof(char)*(newRem)*(size1+size2));
									memcpy(helpBlock1,datab+positionOffset,(newRem)*(size1+size2));
									memcpy(datab+positionOffset,value1,size1);
									memcpy(datab+positionOffset+size1,value2,size2);
									//prepei na kano cpy apo to help block osa xoraei akoma sto datab 
									//xoraei akoma newrem - 1 
									int hlp;
									int offsetHelp=0;
									int databOffset=offset2;
									for(hlp=0;hlp<(newRem-1);hlp++){
										memcpy(datab+positionOffset+databOffset,helpBlock1+offsetHelp,size1);
										memcpy(datab+positionOffset+databOffset+size1,helpBlock1+offsetHelp+size1,size2);
										offsetHelp += offset2;
										databOffset += offset2;
									}
									//bazo sth proti thesi apo to new block auto pou perisepse apo to proto block kai brisketai 
									//sth teleutaia thesi tou helpblock opou deixnei ekei o offsetHelp
									memcpy(newDatab+sizeof(char)+2*sizeof(int),helpBlock1+offsetHelp,size1);
									memcpy(newDatab+sizeof(char)+2*sizeof(int)+size1,helpBlock1+offsetHelp+size1,size2);
									//prepei na knaome cpy  to datab apo counter/2 eos counter  sto new block
									char * helpBlock2 = malloc(sizeof(char)*(currCounter/2)*(size1+size2));
									memcpy(helpBlock2,datab+sizeof(char)+2*sizeof(int)+(currCounter/2)*offset2,(currCounter/2)*(size1+size2));
									//tora metafeorume sto newblock apo th deuteri thesi kai pera auta pou briskontan sto datab/2
									memcpy(newDatab+sizeof(char)+2*sizeof(int)+offset2,helpBlock2,(currCounter/2)*(size1+size2)); //amesos meta th proti thesi grafoume afta pou eiani sto hlpblock2
									int a=insertIndexBlock( fileDesc, size1, sizeof(int), &newDatab[9], type1,newPtr);
											
								}
								else{ // rem >= firstcounter
										//opote to proto block menei idio
										//to knao etsi epeidi mporoume na kanoume copy ta misa stoixeia sto deutero block 
										//kai meta na pame na valoume to stoixeio kai na kanoume anataksinomisi sto deytero block mono
										
									int offset2 = size1 +size2;
									char * helpBlock3 = malloc(sizeof(char)*(currCounter/2)*(size1+size2));
									memcpy(helpBlock3,datab+sizeof(char)+2*sizeof(int)+(currCounter/2)*offset2,(currCounter/2)*(size1+size2));
									memcpy(newDatab+sizeof(char)+2*(sizeof(int)),helpBlock3,(currCounter/2)*(size1+size2));
									int positionOffset1=returnOffset(newDatab,value1,currCounter/2,size1,size2,type1); //exoume th thesi 
									int remainingPairs = currCounter/2 - ( ( positionOffset1 - (sizeof(char)+(2*sizeof(int))) ) / ( size1 + size2 ) ) ;
									char * helpBlock = malloc(sizeof(char)*remainingPairs*(size1+size2));
						
									memcpy(helpBlock,newDatab+positionOffset1,remainingPairs*(size1+size2)); //antigrafoume apo positionOffset mexri counter ta pair value1 value2
						
									memcpy(newDatab+positionOffset1,value1,size1); //vazoume sto newdatab tis kainourgies times
									memcpy(newDatab+positionOffset1+size1,value2,size2);
									memcpy(newDatab+positionOffset1+size1+size2,helpBlock,remainingPairs*(size1+size2)); 
									
									int a=insertIndexBlock( fileDesc, size1, sizeof(int), &newDatab[9], type1,newPtr);			
								}
								
							}
													
							
						
						
						}
						//////////////////////////////////////////////////////////////////
						//////////////////// P E R R I T O S  //////////////////////////////
						/////////////////////////////////////////////////
						else{ 
							BF_Block *newDataBlock;
							BF_Block_Init(&newDataBlock);
							BF_ErrorCode alloc_err111;
							if( (alloc_err111 = BF_AllocateBlock(fileDesc,newDataBlock)) != BF_OK ){ //root block
								BF_PrintError(alloc_err111);
								AM_errno = AM_ALLOC_ERROR;
								return AM_errno;
							}	
							char* newDatab = BF_Block_GetData(newDataBlock);	
							
							
							int positionOffset=returnOffset(datab,value1,currCounter,size1,size2,type1); //exoume th thesi 
							int remainingPairs = currCounter - ( ( positionOffset - (sizeof(char)+(2*sizeof(int))) ) / ( size1 + size2 ) ) ;
							int firstCounter = (currCounter+1)/2;
							int secondCounter = firstCounter ;
							int oldPtr;
							memcpy(&oldPtr,datab+sizeof(char)+sizeof(int),sizeof(int));
							memcpy(newDatab+sizeof(char)+sizeof(int),&oldPtr,sizeof(int));
							char cc = 'd';
							memcpy(newDatab,&cc,sizeof(char));
							int newPtr;
							BF_ErrorCode getBlockCounter_err123;
							if((getBlockCounter_err123=BF_GetBlockCounter(fileDesc,&newPtr))!=BF_OK){
								BF_PrintError(getBlockCounter_err123);
								AM_errno = AME_GETBLOCK_COUNTER_ERROR;
								return AM_errno;
							}
							newPtr = newPtr-1;
							memcpy(datab+sizeof(char)+sizeof(int),&newPtr,sizeof(int));
							memcpy(datab+sizeof(char),&firstCounter,sizeof(int)); // to kanourgio counter sto first block	
							memcpy(newDatab+sizeof(char),&secondCounter,sizeof(int)); // to counter sto block 2 
							if(remainingPairs == 0){ //simainei pos prepei na mpei sto telos 
								char * helpBlock33 = malloc(sizeof(char)*(firstCounter-1)*(size1+size2));
								memcpy(helpBlock33,datab+sizeof(char)+2*sizeof(int)+(firstCounter)*(size1+size2),sizeof(char)*(firstCounter-1)*(size1+size2)); //antigrafoume apo firstCoutner mexri counter ta pair value1 value2
								memcpy(newDatab+sizeof(char)+sizeof(int)+sizeof(int),helpBlock33,(firstCounter-1)*(size1+size2)); //valame ta dedonena sto kanourgio block
								memcpy(newDatab+sizeof(char)+sizeof(int)+sizeof(int)+(firstCounter-1)*(size1+size2),value1,size1); //kolame sto telos to teleutaio stoixeio
								memcpy(newDatab+sizeof(char)+sizeof(int)+sizeof(int)+(firstCounter-1)*(size1+size2)+size1,value2,size2);	

								// INSERT STO INDEXBLOCK
								int a=insertIndexBlock( fileDesc, size1, sizeof(int), &newDatab[9], type1,newPtr);
						  	} 
							else{
								int rem = (positionOffset - (sizeof(char) + 2*sizeof(int)) ) / (size1 + size2);
								if(rem < firstCounter){
									int offset2 = size1 +size2;
									int newRem = firstCounter-rem;
									char * helpBlock1 = malloc(sizeof(char)*(newRem)*(size1+size2));
									memcpy(helpBlock1,datab+positionOffset,(newRem)*(size1+size2));
									memcpy(datab+positionOffset,value1,size1);
									memcpy(datab+positionOffset+size1,value2,size2);
									//prepei na kano cpy apo to help block osa xoraei akoma sto datab 
									//xoraei akoma newrem - 1 
									int hlp;
									int offsetHelp=0;
									int databOffset=offset2;
									for(hlp=0;hlp<(newRem-1);hlp++){
										memcpy(datab+positionOffset+databOffset,helpBlock1+offsetHelp,size1);
										memcpy(datab+positionOffset+databOffset+size1,helpBlock1+offsetHelp+size1,size2);
										offsetHelp += offset2;
										databOffset += offset2;
									}
									//bazo sth proti thesi apo to new block auto pou perisepse apo to proto block kai brisketai 
									//sth teleutaia thesi tou helpblock opou deixnei ekei o offsetHelp
									memcpy(newDatab+sizeof(char)+2*sizeof(int),helpBlock1+offsetHelp,size1);
									memcpy(newDatab+sizeof(char)+2*sizeof(int)+size1,helpBlock1+offsetHelp+size1,size2);
									
									//prepei na knaome cpy  to datab apo counter/2 eos counter  sto new block
									char * helpBlock2 = malloc(sizeof(char)*(firstCounter-1)*(size1+size2));
									memcpy(helpBlock2,datab+sizeof(char)+2*sizeof(int)+(firstCounter)*offset2,(firstCounter-1)*(size1+size2));
									//tora metafeorume sto newblock apo th deuteri thesi kai pera auta pou briskontan sto datab/2
									memcpy(newDatab+sizeof(char)+2*sizeof(int)+offset2,helpBlock2,(firstCounter-1)*(size1+size2)); //amesos meta th proti thesi grafoume afta pou eiani sto hlpblock2
													int a=insertIndexBlock( fileDesc, size1, sizeof(int), &newDatab[9], type1,newPtr);	
								} 
								else{ // rem >= firstcounter
										//opote to proto block menei idio
										//to knao etsi epeidi mporoume na kanoume copy ta misa stoixeia sto deutero block 
										//kai meta na pame na valoume to stoixeio kai na kanoume anataksinomisi sto deytero block mono
										
									int offset2 = size1 +size2;
									char * helpBlock3 = malloc(sizeof(char)*(firstCounter-1)*(size1+size2));
									memcpy(helpBlock3,datab+sizeof(char)+2*sizeof(int)+(firstCounter)*offset2,(firstCounter-1)*(size1+size2));
									memcpy(newDatab+sizeof(char)+2*(sizeof(int)),helpBlock3,(firstCounter-1)*(size1+size2));
									int positionOffset1=returnOffset(newDatab,value1,firstCounter-1,size1,size2,type1); //exoume th thesi 
									int remainingPairs = (firstCounter-1) - ( ( positionOffset1 - (sizeof(char)+(2*sizeof(int))) ) / ( size1 + size2 ) ) ;
									char * helpBlock = malloc(sizeof(char)*remainingPairs*(size1+size2));
						
									memcpy(helpBlock,newDatab+positionOffset1,remainingPairs*(size1+size2)); //antigrafoume apo positionOffset mexri counter ta pair value1 value2
						
									memcpy(newDatab+positionOffset1,value1,size1); //vazoume sto newdatab tis kainourgies times
									memcpy(newDatab+positionOffset1+size1,value2,size2);
									memcpy(newDatab+positionOffset1+size1+size2,helpBlock,remainingPairs*(size1+size2)); 
									int a=insertIndexBlock( fileDesc, size1, sizeof(int), &newDatab[9], type1,newPtr);			
								}
							}	
						}
					} 
				}
			} 
		}
	}

	////////////////////////////////////////////////////////////////////////	
	//////////////////////////*PRINT THE TREE/////////////////////////////*/
	//////////////////////////////////////////////////////////////////////

	/*	if(OpenTable[newi].printc == 100){
			printf("------------------------------------------------------------------------\n");
			printf("Printing for file with filedesc : %d\n",OpenTable[newi].fileDesc);
			printf("------------------------------------------------------------------------\n");
			printAll( fileDesc, size11, size22, type11, type22);
		} */
	
	return AME_OK;
}


int AM_OpenIndexScan(int fileDesc, int op, void *value) {
/*1​ ​ EQUAL​ ​ (πεδίο-κλειδί​ ​ ==​ ​ τιμή​ ​ της​ ​ value)
2​ ​ NOT​ ​ EQUAL​ ​ (πεδίο-κλειδί​ ​ !=​ ​ τιμή​ ​ της​ ​ value)
3​ ​ LESS​ ​ THAN​ ​ (πεδίο-κλειδί​ ​ < ​ ​ τιμή​ ​ της​ ​ value)
4​ ​ GREATER​ ​ THAN​ ​ (πεδίο-κλειδί​ ​ > ​ ​ τιμή​ ​ της​ ​ value)
5​ ​ LESS​ ​ THAN​ ​ or​ ​ EQUAL​ ​ (πεδίο-κλειδί​ ​ <=​ ​ τιμή​ ​ της​ ​ value)
6​ ​ GREATER​ ​ THAN​ ​ or​ ​ EQUAL​ ​ (πεδίο-κλειδί​ ​ >=​ ​ τιμή​ ​ της​ ​ value)*/

	int fd = OpenTable[fileDesc].fileDesc;
	ScansTable[scanscnt].fileDesc = fd;

	BF_Block* Block;
	BF_Block_Init(&Block);

	BF_ErrorCode getBlock_err;
	if( (getBlock_err = BF_GetBlock(fd, 0, Block)) != BF_OK ){
		BF_PrintError(getBlock_err);
		AM_errno = AME_GETBLOCK_ERROR;
		return AM_errno;
	}
    char* infoData=BF_Block_GetData(Block);
	int root, type1, size1, type2, size2;
	memcpy(&root, infoData+sizeof(char)+sizeof(int)+sizeof(char)+sizeof(int)+sizeof(int), sizeof(int));
	memcpy(&type1, infoData, sizeof(char));
	memcpy(&size1, infoData+sizeof(char), sizeof(int));
	memcpy(&type2, infoData+sizeof(char)+sizeof(int), sizeof(char));
	memcpy(&size2, infoData+sizeof(char)+sizeof(int)+sizeof(char), sizeof(int));

	BF_ErrorCode unpin_err;
	if( (unpin_err = BF_UnpinBlock(Block)) != BF_OK ){		//kanw unpin to block
		BF_PrintError(unpin_err);
		AM_errno = AM_UNPIN_ERROR;
		return AM_errno;
	}
	int databNum;
	databNum = searchRoot(fd, root, value, type1, size1);
	BF_Block *block;
	BF_Block_Init(&block);

	BF_ErrorCode getBlock_err1;
	if( (getBlock_err1 = BF_GetBlock(fd, databNum, block)) != BF_OK ){
		BF_PrintError(getBlock_err1);
		AM_errno = AME_GETBLOCK_ERROR;
		return AM_errno;
	}
	char* data=BF_Block_GetData(block);
	int counter;
	memcpy(&counter, data+sizeof(char), sizeof(int));
	int positionOffset;
	if(op!=GREATER_THAN){
		 positionOffset=returnOffsetScan(data, value, counter, size1, size2, type1); //exoume th thesi
	}
	else{
		positionOffset=returnOffset(data, value, counter, size1, size2, type1); //exoume th thesi
		positionOffset-=(size1+size2);
	}
	int offset = size1+size2;
	
	if(op==LESS_THAN_OR_EQUAL ){
		while(compareTypesEqual(value, type1,data,positionOffset, size1)==1){			
			positionOffset+=offset;
		}
		positionOffset-=offset;
	}
						
	BF_ErrorCode unpin_err1;
	if( (unpin_err1 = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
		BF_PrintError(unpin_err1);
		AM_errno = AM_UNPIN_ERROR;
		return AM_errno;
	}
	if(compareTypesEqual(value,type1,data,positionOffset,size1)==1){
		ScansTable[scanscnt].offset = positionOffset;
	}
	else{
		ScansTable[scanscnt].offset = -2;	
	}
	ScansTable[scanscnt].blockNum = databNum;
	ScansTable[scanscnt].operation = op;
	scanscnt++;
	return scanscnt-1;			
			
		

/*
	switch(op){

		case EQUAL:{
				int databNum = searchRoot(fd, root, value, type1, size1);
				BF_Block *block;
				BF_Block_Init(&block);

				BF_ErrorCode getBlock_err1;
				if( (getBlock_err1 = BF_GetBlock(fd, databNum, block)) != BF_OK ){
						BF_PrintError(getBlock_err1);
						AM_errno = AME_GETBLOCK_ERROR;
						return AM_errno;
				}

				char* data=BF_Block_GetData(block);
				int counter;
				memcpy(&counter, data+sizeof(char), sizeof(int));
				int positionOffset=returnOffset(data, value, counter, size1, size2, type1); //exoume th thesi
			//	printf("\nSCAN EQUAL RESULTS:\n");
				int offset = size1+size2;
				positionOffset = positionOffset-offset;
				int firstoffset = positionOffset;
				while(compareTypesEqual(value, type1,data,positionOffset, size1)==1){
					print(data , type1, size1, positionOffset);
					print(data , type2, size2, positionOffset+size1);
					positionOffset+=offset;
				}
				

				BF_ErrorCode unpin_err1;
				if( (unpin_err1 = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err1);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}
				ScansTable[scanscnt].lastValue = firstoffset ;
				ScansTable[scanscnt].lastBlockNum = databNum;
				scanscnt++;
				return scanscnt-1;
				break;
		}case NOT_EQUAL:{
				BF_Block *block;
				BF_Block_Init(&block);

				BF_ErrorCode getBlock_err;
				if( (getBlock_err = BF_GetBlock(fd, root, block)) != BF_OK ){
						BF_PrintError(getBlock_err);
						AM_errno = AME_GETBLOCK_ERROR;
						return AM_errno;
				}			

				char *data = BF_Block_GetData(block);
				char c;
				int leftmostptr;
				memcpy(&c, data, sizeof(char));
				memcpy(&leftmostptr, data+sizeof(char)+sizeof(int), sizeof(int));

				BF_ErrorCode unpin_err;
				if( (unpin_err = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}
				
				while(c != 'd'){
					BF_ErrorCode getBlock_err1;
					if( (getBlock_err1 = BF_GetBlock(fd, leftmostptr, block)) != BF_OK ){
						BF_PrintError(getBlock_err1);
						AM_errno = AME_GETBLOCK_ERROR;
						return AM_errno;
					}

					char *bdata = BF_Block_GetData(block);
					memcpy(&c ,bdata, sizeof(char));
					if(c == 'i'){
						memcpy(&leftmostptr, bdata+sizeof(char)+sizeof(int), sizeof(int));
						BF_ErrorCode unpin_err1;
						if( (unpin_err1 = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
							BF_PrintError(unpin_err1);
							AM_errno = AM_UNPIN_ERROR;
							return AM_errno;
						}
					}else{
						break;
					}
				}
				//to leftmostptr twra exei to 1o datablock number
				int counter, nextdblock,lastoffset,lastblockptr;
				int blockptr = leftmostptr;
				while(blockptr != -1){
					BF_ErrorCode getBlock_err2;
					if( (getBlock_err2 = BF_GetBlock(fd, blockptr, block)) != BF_OK ){
						BF_PrintError(getBlock_err2);
						AM_errno = AME_GETBLOCK_ERROR;
						return AM_errno;
					}
					char *blockdata = BF_Block_GetData(block);
					memcpy(&counter, blockdata+sizeof(char), sizeof(int));
					memcpy(&nextdblock, blockdata+sizeof(char)+sizeof(int), sizeof(int));
					int offset = sizeof(char)+2*sizeof(int);
					int i, res;
					for(i=0; i<counter; i++){
						res = compareTypesEqual(value, type1,blockdata, offset, size1);
						if(res == 0){
							print(blockdata , type1, size1, offset);
							print(blockdata , type2, size2, offset+size1);
						}else{
							lastoffset=offset;
							lastblockptr = blockptr;				
						}
						offset+=size1+size2;
					}
					blockptr = nextdblock;
					if(blockptr == -1)
						break;
					BF_ErrorCode unpin_err2;
					if( (unpin_err2 = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
						BF_PrintError(unpin_err2);
						AM_errno = AM_UNPIN_ERROR;
						return AM_errno;
					}
				}
				//enhmerwsh scanTable
				ScansTable[scanscnt].lastValue = lastoffset ;
				ScansTable[scanscnt].lastBlockNum = blockptr;
				scanscnt++;
				return scanscnt-1;
				break;
		}case LESS_THAN:{
			BF_Block *block1;
			BF_Block_Init(&block1);

			BF_ErrorCode getBlock_err;
			if( (getBlock_err = BF_GetBlock(fd, root, block1)) != BF_OK ){
				BF_PrintError(getBlock_err);
				AM_errno = AME_GETBLOCK_ERROR;
				return AM_errno;
			}			

			char *data = BF_Block_GetData(block1);
			char c;
			int leftmostptr;
			memcpy(&c, data, sizeof(char));
			memcpy(&leftmostptr, data+sizeof(char)+sizeof(int), sizeof(int));

			BF_ErrorCode unpin_err;
			if( (unpin_err = BF_UnpinBlock(block1)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
			}
				
			while(c != 'd'){
				BF_ErrorCode getBlock_err1;
				if( (getBlock_err1 = BF_GetBlock(fd, leftmostptr, block1)) != BF_OK ){
					BF_PrintError(getBlock_err1);
					AM_errno = AME_GETBLOCK_ERROR;
					return AM_errno;
				}

				char *bdata = BF_Block_GetData(block1);
				memcpy(&c ,bdata, sizeof(char));
				if(c == 'i'){
					memcpy(&leftmostptr, bdata+sizeof(char)+sizeof(int), sizeof(int));
					BF_ErrorCode unpin_err1;
					if( (unpin_err1 = BF_UnpinBlock(block1)) != BF_OK ){		//kanw unpin to block
						BF_PrintError(unpin_err1);
						AM_errno = AM_UNPIN_ERROR;
						return AM_errno;
					}
				}else{
					break;
				}
			}	
			int flag=0;
			int blockptr=leftmostptr;
			int counter, nextdblock;
			while(flag != 1){
				BF_ErrorCode getBlock_err3;
					if( (getBlock_err3 = BF_GetBlock(fd, blockptr, block1)) != BF_OK ){
					BF_PrintError(getBlock_err3);
					AM_errno = AME_GETBLOCK_ERROR;
					return AM_errno;
				}
				char *blockdata = BF_Block_GetData(block1);
				memcpy(&counter, blockdata+sizeof(char), sizeof(int));
				memcpy(&nextdblock, blockdata+sizeof(char)+sizeof(int), sizeof(int));
				int offset = sizeof(char)+2*sizeof(int);
				int i, res;
				for(i=0; i<counter; i++){
					res = compareTypes(value, type1,blockdata, offset, size1);
					if(res == 2){
						print(blockdata , type1, size1, offset);
						print(blockdata , type2, size2, offset+size1);
					}else {
						flag=1;
						break;
					}
					offset+=size1+size2;
				}
				blockptr = nextdblock;
				if(blockptr == -1)
					break;
				BF_ErrorCode unpin_err3;
				if( (unpin_err3 = BF_UnpinBlock(block1)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err3);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}				
			}
			//ENHMERWSH SCAN TABLE
			break;
		}case GREATER_THAN:{
				int databNum = searchRoot(fd, root, value, type1, size1);
				BF_Block *block3;
				BF_Block_Init(&block3);

				BF_ErrorCode getBlock_err3;
				if( (getBlock_err3 = BF_GetBlock(fd, databNum, block3)) != BF_OK ){
						BF_PrintError(getBlock_err3);
						AM_errno = AME_GETBLOCK_ERROR;
						return AM_errno;
				}

				char* data=BF_Block_GetData(block3);
				int counter, nextdblock;
				memcpy(&counter, data+sizeof(char), sizeof(int));
				memcpy(&nextdblock, data+sizeof(char)+sizeof(int), sizeof(int));
				int positionOffset=returnOffset(data, value, counter, size1, size2, type1); //exoume th thesi
				int currCounter = (positionOffset - sizeof(char) - 2*sizeof(int) ) / (size1+size2);
				int offset = size1+size2;
				positionOffset = positionOffset-offset;
				int i, res1, res2;
				for(i=currCounter;i<counter;i++){
					res1 = compareTypes(value, type1,data,positionOffset, size1);
					res2 = compareTypesEqual(value, type1,data,positionOffset, size1);
					if(res1 == 1 && res2 == 0){
						print(data , type1, size1, positionOffset);
						print(data , type2, size2, positionOffset+size1);
					}
					positionOffset+=offset;
				}

				BF_ErrorCode unpin_err1;
				if( (unpin_err1 = BF_UnpinBlock(block3)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err1);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}
				positionOffset=sizeof(char)+2*sizeof(int);
				int	blockptr=nextdblock;
				while(blockptr!=-1){
					BF_ErrorCode getBlock_err3;
					if( (getBlock_err3 = BF_GetBlock(fd, blockptr, block3)) != BF_OK ){
						BF_PrintError(getBlock_err3);
						AM_errno = AME_GETBLOCK_ERROR;
						return AM_errno;
					}

					char* data1=BF_Block_GetData(block3);
					int counter, nextdblock;
					memcpy(&counter, data1+sizeof(char), sizeof(int));
					memcpy(&nextdblock, data1+sizeof(char)+sizeof(int), sizeof(int));

					for(i=0;i<counter;i++){
						res1 = compareTypes(value, type1,data1,positionOffset, size1);
						res2 = compareTypesEqual(value, type1,data1,positionOffset, size1);
						if(res1 == 1 && res2 == 0){
							print(data1 , type1, size1, positionOffset);
							print(data1 , type2, size2, positionOffset+size1);
						}
						positionOffset+=offset;
					}
					blockptr=nextdblock;
					
					if( (unpin_err1 = BF_UnpinBlock(block3)) != BF_OK ){		//kanw unpin to block
						BF_PrintError(unpin_err1);
						AM_errno = AM_UNPIN_ERROR;
						return AM_errno;
					}	
				
				}
//				ScansTable[scanscnt].lastValue = positionOffset ;
	//			ScansTable[scanscnt].lastBlockNum = databNum;
		//		scanscnt++;
			//	return scanscnt-1;
				break;
		}case LESS_THAN_OR_EQUAL:{
			BF_Block *block2;
			BF_Block_Init(&block2);

			BF_ErrorCode getBlock_err44;
			if( (getBlock_err44 = BF_GetBlock(fd, root, block2)) != BF_OK ){
				BF_PrintError(getBlock_err44);
				AM_errno = AME_GETBLOCK_ERROR;
				return AM_errno;
			}			

			char *data = BF_Block_GetData(block2);
			char c;
			int leftmostptr;
			memcpy(&c, data, sizeof(char));
			memcpy(&leftmostptr, data+sizeof(char)+sizeof(int), sizeof(int));

			BF_ErrorCode unpin_err44;
			if( (unpin_err44 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err44);
				AM_errno = AM_UNPIN_ERROR;
				return AM_errno;
			}
				
			while(c != 'd'){
				BF_ErrorCode getBlock_err4;
				if( (getBlock_err4 = BF_GetBlock(fd, leftmostptr, block2)) != BF_OK ){
					BF_PrintError(getBlock_err4);
					AM_errno = AME_GETBLOCK_ERROR;
					return AM_errno;
				}

				char *bdata = BF_Block_GetData(block2);
				memcpy(&c ,bdata, sizeof(char));
				if(c == 'i'){
					memcpy(&leftmostptr, bdata+sizeof(char)+sizeof(int), sizeof(int));
					BF_ErrorCode unpin_err4;
					if( (unpin_err4 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
						BF_PrintError(unpin_err4);
						AM_errno = AM_UNPIN_ERROR;
						return AM_errno;
					}
				}else{
					break;
				}
			}	
			int flag=0;
			int blockptr=leftmostptr;
			int counter, nextdblock;

			while(flag != 1){
				BF_ErrorCode getBlock_err4;
					if( (getBlock_err4 = BF_GetBlock(fd, blockptr, block2)) != BF_OK ){
					BF_PrintError(getBlock_err4);
					AM_errno = AME_GETBLOCK_ERROR;
					return AM_errno;
				}
				char *blockdata = BF_Block_GetData(block2);
				memcpy(&counter, blockdata+sizeof(char), sizeof(int));
				memcpy(&nextdblock, blockdata+sizeof(char)+sizeof(int), sizeof(int));
				//printf("blockptr%d\n",nextdblock);
				
				int offset = sizeof(char)+2*sizeof(int);
				//print(blockdata , type1, size1, offset);
				int i, res1, res2;
				for(i=0; i<counter; i++){
				
					res1 = compareTypes(value, type1,blockdata, offset, size1);
					res2 = compareTypesEqual(value, type1,blockdata, offset, size1);
												
					if(res1 == 2 || res2 == 1){
						print(blockdata , type1, size1, offset);
						print(blockdata , type2, size2, offset+size1);
					}else{
						flag=1;
						break;
					}
					offset+=(size1+size2);
				}
				blockptr = nextdblock;

			//	if(blockptr == -1)
				//	break;
				BF_ErrorCode unpin_err4;
				if( (unpin_err4 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err4);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}				
			}
			//ENHMERWSH SCAN TABLE
				break;
		}case GREATER_THAN_OR_EQUAL:{
				int databNum = searchRoot(fd, root, value, type1, size1);
				BF_Block *block3;
				BF_Block_Init(&block3);

				BF_ErrorCode getBlock_err3;
				if( (getBlock_err3 = BF_GetBlock(fd, databNum, block3)) != BF_OK ){
						BF_PrintError(getBlock_err3);
						AM_errno = AME_GETBLOCK_ERROR;
						return AM_errno;
				}

				char* data=BF_Block_GetData(block3);
				int counter, nextdblock;
				memcpy(&counter, data+sizeof(char), sizeof(int));
				memcpy(&nextdblock, data+sizeof(char)+sizeof(int), sizeof(int));
				int positionOffset=returnOffset(data, value, counter, size1, size2, type1); //exoume th thesi
				int currCounter = (positionOffset - sizeof(char) - 2*sizeof(int) ) / (size1+size2);
				int offset = size1+size2;
				positionOffset = positionOffset-offset;
				int i, res;
				for(i=currCounter;i<counter;i++){
					res = compareTypes(value, type1,data,positionOffset, size1);
					if(res == 1){
						print(data , type1, size1, positionOffset);
						print(data , type2, size2, positionOffset+size1);
					}
					positionOffset+=offset;
				}

				BF_ErrorCode unpin_err1;
				if( (unpin_err1 = BF_UnpinBlock(block3)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err1);
					AM_errno = AM_UNPIN_ERROR;
					return AM_errno;
				}
				positionOffset=sizeof(char)+2*sizeof(int);
				int blockptr=nextdblock;
				while(blockptr!=-1){
					BF_ErrorCode getBlock_err3;
					if( (getBlock_err3 = BF_GetBlock(fd, blockptr, block3)) != BF_OK ){
						BF_PrintError(getBlock_err3);
						AM_errno = AME_GETBLOCK_ERROR;
						return AM_errno;
					}

					char* data1=BF_Block_GetData(block3);
					int counter, nextdblock;
					memcpy(&counter, data1+sizeof(char), sizeof(int));
					memcpy(&nextdblock, data1+sizeof(char)+sizeof(int), sizeof(int));

					for(i=0;i<counter;i++){
						res = compareTypes(value, type1,data1,positionOffset, size1);
						if(res == 1){
							print(data1 , type1, size1, positionOffset);
							print(data1 , type2, size2, positionOffset+size1);
						}
						positionOffset+=offset;
					}
					blockptr=nextdblock;
					
					if( (unpin_err1 = BF_UnpinBlock(block3)) != BF_OK ){		//kanw unpin to block
						BF_PrintError(unpin_err1);
						AM_errno = AM_UNPIN_ERROR;
						return AM_errno;
					}	
				
				}
				break;
		}
	}
	*/
	
  	return AME_OK;
}


void *AM_FindNextEntry(int scanDesc) {
	int fd=ScansTable[scanDesc].fileDesc;
	int flagError=0;
	BF_Block* Block;
	BF_Block_Init(&Block);

	BF_ErrorCode getBlock_err;
	if( (getBlock_err = BF_GetBlock(fd, 0, Block)) != BF_OK ){
		BF_PrintError(getBlock_err);
		AM_errno = AME_GETBLOCK_ERROR;
		return NULL;
	}
    char* infoData=BF_Block_GetData(Block);
	int root, type1, size1, type2, size2;
	memcpy(&root, infoData+sizeof(char)+sizeof(int)+sizeof(char)+sizeof(int)+sizeof(int), sizeof(int));
	memcpy(&type1, infoData, sizeof(char));
	memcpy(&size1, infoData+sizeof(char), sizeof(int));
	memcpy(&type2, infoData+sizeof(char)+sizeof(int), sizeof(char));
	memcpy(&size2, infoData+sizeof(char)+sizeof(int)+sizeof(char), sizeof(int));

	BF_ErrorCode unpin_err;
	if( (unpin_err = BF_UnpinBlock(Block)) != BF_OK ){		//kanw unpin to block
		BF_PrintError(unpin_err);
		AM_errno = AM_UNPIN_ERROR;
		return NULL;
	}
	//int leftmostptr,blockptr;
	if(notEqualFlag==0 && ((ScansTable[scanDesc].operation == NOT_EQUAL)||(ScansTable[scanDesc].operation == LESS_THAN_OR_EQUAL)||(ScansTable[scanDesc].operation == LESS_THAN) ) ){
		BF_Block *block;
		BF_Block_Init(&block);
		BF_ErrorCode getBlock_err1;
		if( (getBlock_err1 = BF_GetBlock(fd,root , block)) != BF_OK ){
				BF_PrintError(getBlock_err1);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
		}
		char* data=BF_Block_GetData(block);
		char c;
		memcpy(&c, data, sizeof(char));
		memcpy(&leftmostptr, data+sizeof(char)+sizeof(int), sizeof(int));

		BF_ErrorCode unpin_err;
		if( (unpin_err = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
			BF_PrintError(unpin_err);
			AM_errno = AM_UNPIN_ERROR;
			return NULL;
		}
				
		while(c != 'd'){
			BF_ErrorCode getBlock_err1;
			if( (getBlock_err1 = BF_GetBlock(fd, leftmostptr, block)) != BF_OK ){
				BF_PrintError(getBlock_err1);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}

			char *bdata = BF_Block_GetData(block);
			memcpy(&c ,bdata, sizeof(char));
			if(c == 'i'){
				memcpy(&leftmostptr, bdata+sizeof(char)+sizeof(int), sizeof(int));			
			}
			BF_ErrorCode unpin_err1;
			if( (unpin_err1 = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err1);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
		}
		blockptr = leftmostptr;
		notEqualFlag=1;
	}
	if(greaterFlag == 0 && ((ScansTable[scanDesc].operation == GREATER_THAN_OR_EQUAL)||(ScansTable[scanDesc].operation == GREATER_THAN)) ){
		blockptr = ScansTable[scanDesc].blockNum;	
		BF_Block *block2;
		BF_Block_Init(&block2);
		BF_ErrorCode getBlock_err133;
		if( (getBlock_err133 = BF_GetBlock(fd, ScansTable[scanDesc].blockNum, block2)) != BF_OK ){
			BF_PrintError(getBlock_err133);
			AM_errno = AME_GETBLOCK_ERROR;
			return NULL;
		}
		char* data2=BF_Block_GetData(block2);
		int positionOffset = ScansTable[scanDesc].offset;
		int counter,nextdblock;
		memcpy(&counter, data2+sizeof(char), sizeof(int));
		int pos = (positionOffset - sizeof(char) -2*sizeof(int) ) / (size1+size2);
		counterNE = pos;
		BF_ErrorCode unpin_err1;
			if( (unpin_err1 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err1);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
		greaterFlag=1;
	}
	void * val2 ;
	void * value;
	if(type1 == 'f' ){value=malloc(sizeof(float));}
	else if(type1 == 'i'){value=malloc(sizeof(int));}
	else{value = malloc(size1);}
	if(type2 == 'f' ){val2=malloc(sizeof(float));}
	else if(type2 == 'i'){val2=malloc(sizeof(int));}
	else{val2 = malloc(size2);}
	switch(ScansTable[scanDesc].operation){
	case EQUAL:{
				BF_Block *block;
				BF_Block_Init(&block);
				BF_ErrorCode getBlock_err1;
				if( (getBlock_err1 = BF_GetBlock(fd, ScansTable[scanDesc].blockNum, block)) != BF_OK ){
						BF_PrintError(getBlock_err1);
						AM_errno = AME_GETBLOCK_ERROR;
						return NULL;
				}
				char* data=BF_Block_GetData(block);
				int counter;
				memcpy(&counter, data+sizeof(char), sizeof(int));
				int positionOffset= ScansTable[scanDesc].offset;
				int offset = size1+size2;
				memcpy(value,data + positionOffset ,size1); 
				if(positionOffset == -2){
					AM_errno = AME_EOF; 
					return NULL;
				}
				while(compareTypesEqual(value, type1,data,positionOffset, size1)==1){
				memcpy(val2,data + positionOffset + size1,size2); 
					positionOffset+=offset;
				}
				BF_ErrorCode unpin_err1;
				if( (unpin_err1 = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
					BF_PrintError(unpin_err1);
					AM_errno = AM_UNPIN_ERROR;
					return NULL;
				}
				break;
		}	
		case NOT_EQUAL:{
			BF_Block *block;
			BF_Block_Init(&block);
			BF_ErrorCode getBlock_err1;
			if( (getBlock_err1 = BF_GetBlock(fd, ScansTable[scanDesc].blockNum, block)) != BF_OK ){
				BF_PrintError(getBlock_err1);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char* data=BF_Block_GetData(block);
			int positionOffset= ScansTable[scanDesc].offset;
			memcpy(value,data + positionOffset ,size1); 
			BF_ErrorCode unpin_err22;
			if( (unpin_err22 = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err22);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
			int counter, nextdblock;
			BF_ErrorCode getBlock_err2;
			if( (getBlock_err2 = BF_GetBlock(fd, blockptr, block)) != BF_OK ){
				BF_PrintError(getBlock_err2);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char *blockdata = BF_Block_GetData(block);
			memcpy(&counter, blockdata+sizeof(char), sizeof(int));
			memcpy(&nextdblock, blockdata+sizeof(char)+sizeof(int), sizeof(int));
			int offset1 = sizeof(char)+2*sizeof(int);
			int off = offset1 + counterNE*(size1+size2);
			int i, res;
			res = compareTypesEqual(value, type1,blockdata, off , size1);
			if(res == 0){
				counterNE++;
				memcpy(val2,blockdata + off + size1,size2); 
			}else{
				counterNE++;
				off = off + size1+ size2+ size1;
				memcpy(val2,blockdata+off,size2); 
			}
			if(counterNE == counter  ){
				counterNE =0;
				blockptr = nextdblock;
				if(blockptr == -1 ){
					notEqualFlag = 0;
				AM_errno = AME_EOF; 
				return NULL;
				}
			} 
					
			BF_ErrorCode unpin_err2;
			if( (unpin_err2 = BF_UnpinBlock(block)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err2);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
			break;
		}case LESS_THAN_OR_EQUAL:{
			

			BF_Block *block2;
			BF_Block_Init(&block2);
			BF_ErrorCode getBlock_err1;
			if( (getBlock_err1 = BF_GetBlock(fd, ScansTable[scanDesc].blockNum, block2)) != BF_OK ){
				BF_PrintError(getBlock_err1);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char* data2=BF_Block_GetData(block2);
			int positionOffset = ScansTable[scanDesc].offset;
			memcpy(value, data2 + positionOffset ,size1); 
			BF_ErrorCode unpin_err22;
			if( (unpin_err22 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err22);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
			int counter, nextdblock;
			BF_ErrorCode getBlock_err2;
			if( (getBlock_err2 = BF_GetBlock(fd, blockptr, block2)) != BF_OK ){
				BF_PrintError(getBlock_err2);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char *blockdata = BF_Block_GetData(block2);
			memcpy(&counter, blockdata+sizeof(char), sizeof(int));
			memcpy(&nextdblock, blockdata+sizeof(char)+sizeof(int), sizeof(int));
			int offset1 = sizeof(char)+2*sizeof(int);
			int off = offset1 + counterNE*(size1+size2);
			int res1,res2;
			res1 = compareTypes(value, type1,blockdata, off, size1);
			res2 = compareTypesEqual(value, type1,blockdata, off, size1);
			if(res1 == 2  ){
				counterNE++;
				memcpy(val2,blockdata + off + size1,size2); 
			}
			else{
				if(res2 == 1){
					counterNE++;
					memcpy(val2,blockdata + off + size1,size2); 
				}
				else{
					notEqualFlag = 0;
					AM_errno = AME_EOF; 
					return NULL;
				}
			}
			if(counterNE == counter  ){
				counterNE =0;
				blockptr = nextdblock;	
			} 
					
			BF_ErrorCode unpin_err2;
			if( (unpin_err2 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err2);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
			break;
		}case LESS_THAN:{
			BF_Block *block2;
			BF_Block_Init(&block2);
			BF_ErrorCode getBlock_err1;
			if( (getBlock_err1 = BF_GetBlock(fd, ScansTable[scanDesc].blockNum, block2)) != BF_OK ){
				BF_PrintError(getBlock_err1);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char* data2=BF_Block_GetData(block2);
			int positionOffset = ScansTable[scanDesc].offset;
			memcpy(value, data2 + positionOffset ,size1); 
			BF_ErrorCode unpin_err22;
			if( (unpin_err22 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err22);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
			int counter, nextdblock;
			BF_ErrorCode getBlock_err2;
			if( (getBlock_err2 = BF_GetBlock(fd, blockptr, block2)) != BF_OK ){
				BF_PrintError(getBlock_err2);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char *blockdata = BF_Block_GetData(block2);
			memcpy(&counter, blockdata+sizeof(char), sizeof(int));
			memcpy(&nextdblock, blockdata+sizeof(char)+sizeof(int), sizeof(int));
			int offset1 = sizeof(char)+2*sizeof(int);
			int off = offset1 + counterNE*(size1+size2);
			int res1,res2;
			res1 = compareTypes(value, type1,blockdata, off, size1);
			res2 = compareTypesEqual(value, type1,blockdata, off, size1);
			if(res1 == 2  ){
				if(res2 == 1){
					notEqualFlag = 0;
					AM_errno = AME_EOF; 
					return NULL;
				}
				counterNE++;
				memcpy(val2,blockdata + off + size1,size2); 
		
			}
			else{
					notEqualFlag = 0;
					AM_errno = AME_EOF; 
					return NULL;
			}
			if(counterNE == counter  ){
				counterNE =0;
				blockptr = nextdblock;	
			} 	
			BF_ErrorCode unpin_err2;
			if( (unpin_err2 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err2);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
			break;
		}case GREATER_THAN_OR_EQUAL:{
			BF_Block *block2;
			BF_Block_Init(&block2);
			BF_ErrorCode getBlock_err1;
			if( (getBlock_err1 = BF_GetBlock(fd, ScansTable[scanDesc].blockNum, block2)) != BF_OK ){
				BF_PrintError(getBlock_err1);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char* data1=BF_Block_GetData(block2);
			int positionOffset = ScansTable[scanDesc].offset;
			memcpy(value, data1 + positionOffset ,size1); 
				
			BF_ErrorCode unpin_err2;
			if( (unpin_err2 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err2);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
				if(blockptr == -1)	{
					greaterFlag = 0;
					AM_errno = AME_EOF; 
					return NULL;
				}
			BF_Block *block4;
			BF_Block_Init(&block4);
			BF_ErrorCode getBlock_err66;
			if( (getBlock_err66 = BF_GetBlock(fd, blockptr, block4)) != BF_OK ){
				BF_PrintError(getBlock_err66);
				printf("fd%d-blocknum%d\n",fd,blockptr);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char* data2=BF_Block_GetData(block4);
			
			
			int counter,nextdblock;
			memcpy(&counter, data2+sizeof(char), sizeof(int));
			memcpy(&nextdblock, data2+sizeof(char)+sizeof(int), sizeof(int));
			int offset1 = sizeof(char)+2*sizeof(int);
			int off = offset1 + counterNE*(size1+size2);
			int res1,res2;
			res1 = compareTypes(value, type1,data2, off, size1);
			res2 = compareTypesEqual(value, type1,data2, off, size1);
			if(res1 == 1 || res2==1 ){
				counterNE++;
				memcpy(val2,data2 + off + size1,size2); 	
			}
			else{
					greaterFlag = 0;
					AM_errno = AME_EOF; 
					return NULL;
				
			}
			if(counterNE == counter  ){
				counterNE =0;
				blockptr = nextdblock;
			} 
			BF_ErrorCode unpin_err22;
			if( (unpin_err22 = BF_UnpinBlock(block4)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err22);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
		}case GREATER_THAN:{
			if(ft==0){
				counterNE++;
				ft=1;
			}
			BF_Block *block2;
			BF_Block_Init(&block2);
			BF_ErrorCode getBlock_err1;
			if( (getBlock_err1 = BF_GetBlock(fd, ScansTable[scanDesc].blockNum, block2)) != BF_OK ){
				BF_PrintError(getBlock_err1);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char* data1=BF_Block_GetData(block2);
			int positionOffset = ScansTable[scanDesc].offset;
			memcpy(value, data1 + positionOffset ,size1); 
				
			BF_ErrorCode unpin_err2;
			if( (unpin_err2 = BF_UnpinBlock(block2)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err2);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
				if(blockptr == -1)	{
					greaterFlag = 0;
					ft=0;
					AM_errno = AME_EOF; 
					return NULL;
				}
			BF_Block *block4;
			BF_Block_Init(&block4);
			BF_ErrorCode getBlock_err66;
			if( (getBlock_err66 = BF_GetBlock(fd, blockptr, block4)) != BF_OK ){
				BF_PrintError(getBlock_err66);
				printf("fd%d-blocknum%d\n",fd,blockptr);
				AM_errno = AME_GETBLOCK_ERROR;
				return NULL;
			}
			char* data2=BF_Block_GetData(block4);
			
			
			int counter,nextdblock;
			memcpy(&counter, data2+sizeof(char), sizeof(int));
			memcpy(&nextdblock, data2+sizeof(char)+sizeof(int), sizeof(int));
			int offset1 = sizeof(char)+2*sizeof(int);
			int off = offset1 + counterNE*(size1+size2);
			int res1;
			res1 = compareTypes(value, type1,data2, off, size1);
			if(res1 == 1 ){
				counterNE++;
				memcpy(val2,data2 + off + size1,size2); 	
			}
			else{
					greaterFlag = 0;
					ft=0;
					AM_errno = AME_EOF; 
					return NULL;
				
			}
			if(counterNE == counter  ){
				counterNE =0;
				blockptr = nextdblock;
			} 
			BF_ErrorCode unpin_err22;
			if( (unpin_err22 = BF_UnpinBlock(block4)) != BF_OK ){		//kanw unpin to block
				BF_PrintError(unpin_err22);
				AM_errno = AM_UNPIN_ERROR;
				return NULL;
			}
		}


	
	}
		return	val2;
}


int AM_CloseIndexScan(int scanDesc) {

	if(scanDesc<=scanscnt){		//kleinw th sarwsh apo ton pinaka anoiktwn sarwsewn
		ScansTable[scanDesc].fileDesc = -1;
		int j;
		for(j=scanDesc;j<scanscnt;j++){		//kanw shift aristera ola ta epomena stoixeia ,wste o pinakas na einai gematos sthn arxh
			ScansTable[j].fileDesc=ScansTable[j+1].fileDesc;
			ScansTable[j].operation=ScansTable[j+1].operation;
			ScansTable[j].blockNum = ScansTable[j+1].blockNum;
			ScansTable[j].offset = ScansTable[j+1].offset;
		}
		scanscnt--;
		AM_errno = AME_OK;
		return AME_OK;
	}else{
		AM_errno = AME_CLOSE_FILE_SCAN_ERROR;
		return AME_CLOSE_FILE_SCAN_ERROR;
	}

  return AME_OK;
}


void AM_PrintError(char *errString) {		//kanei print analoga to error

	printf("\n...%s", errString);
	switch(AM_errno){
		case AME_OK:
			printf("OK\n");
			break;
		case AME_EOF:
			printf("END OF FILE REACHED\n");
			break;
		case AME_OPEN_FILES_LIMIT_ERROR:
			printf("MAX OPEN FILES\n");
			break;
		case AME_INVALID_FILE_ERROR:
			printf("NO SUCH FILE\n");
			break;
		case AME_ACTIVE_ERROR:
			printf("FILE IS OPEN\n");
			break;
		case AME_FILE_ALREADY_EXISTS_ERROR:
			printf("FILE ALREADY EXISTS\n");
			break;
		case AME_FULL_MEMORY_ERROR:
			printf("MEMORY FULL\n");
			break;
		case AME_INVALID_BLOCK_NUMBER_ERROR:
			printf("INVALID BLOCK NUMBER\n");
			break;
		case AME_GETBLOCK_ERROR:
			printf("GETBLOCK ERROR\n");
			break;
		case ERROR:
			printf("UNDEFINED ERROR\n");
			break;
		case AM_ALLOC_ERROR:
			printf("ALLOCATE BLOCK ERROR\n");
			break;
		case AM_UNPIN_ERROR:
			printf("UNPIN BLOCK ERROR\n");
			break;
		case AME_GETBLOCK_COUNTER_ERROR:
			printf("GETBLOCK COUNTER ERROR\n");
			break;
		case AME_TYPE_ERROR:
			printf("TYPE ERROR\n");
			break;
		case AME_OPEN_FILE_ERROR:
			printf("ERROR OPENING FILE\n");
			break;
		case AME_OPEN_FILE_SCAN_ERROR:
			printf("FILE IS OPEN FOR SCAN\n");
			break;
		case AME_FAIL_DESTROY_INDEX_ERROR:
			printf("FAIL DESTROY INDEX\n");
			break;
		case AME_CLOSE_FILE_ERROR:
			printf("ERROR CLOSING FILE\n");
			break;
		case AME_UNITIALIZED_FILE_ERROR:
			printf("UNINITIALIZED FILE ERROR\n");
			break;
		case AME_CLOSE_FILE_SCAN_ERROR:
			printf("ERROR CLOSING SCAN FILE\n");
			break;
		default:
			printf("ERROR\n");
			break;
	}
  	//printf("ERROR: %d\n", AM_errno);

}

void AM_Close() { //diagrafw tous 2 pinakes

	free(OpenTable);
	OpenTable=NULL;
	free(ScansTable);
	ScansTable=NULL;
}
