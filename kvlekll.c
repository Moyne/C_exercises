

/*void chooseChange(eq* eq,int n){
    int min=-1,minPos;
    for(int i=0;i<n;i++){
        if(eq[i].nInc>min || min==-1) minPos=i;
    }

void createKvl(int* markEl,element* elems,char* eq,int n){
    strcpy(eq,"");
    int taken=0;
    for(int i=0;i<n;i++){
        if(markEl[i]){
            taken++;
            if(taken==1){
                strcat(eq,"+ V");
                strcat(eq,elems[i].name);
                strcat(eq," ");
            }
            else{
                strcat(eq,"- V");
                strcat(eq,elems[i].name);
                strcat(eq," ");
            }
        }
    }
    printf("CREATEKVL: %s\n",eq);
}

void kvlR(element* element,int pos,int n,eq* eq,int* j,int* markEl){
    if(pos>=n){
        int startNode,node,startFirstNode,startLastNode,firstNode,lastNode,taken=0;
        for(int j=0;j<n;j++){
            if(markEl[j]){
                taken++;
                if(taken==1){
                    startNode=element[j].firstNode;
                    node=element[j].lastNode;
                    startFirstNode=element[j].firstNode;
                    startLastNode=element[j].lastNode;
                    firstNode=element[j].firstNode;
                    lastNode=element[j].lastNode;
                }
                else{
                    if(element[j].firstNode==node || (element[j].firstNode==firstNode && element[j].lastNode==lastNode))    node=element[j].lastNode;
                    else if(element[j].firstNode==firstNode && startFirstNode==firstNode && startLastNode==lastNode){
                        node=element[j].lastNode;
                        startNode=startFirstNode;
                        startFirstNode=startLastNode;
                    }
                    else{
                        node=0;
                        startNode=1;
                        break;
                    }
                    firstNode=element[j].firstNode;
                    lastNode=element[j].lastNode;
                }
            }
        }
        printf("\nEcco il vettore generato:\n");
        for(int j=0;j<n;j++) printf("%d ",markEl[j]);
        printf("\n");
        if(taken>0 && node==startNode){
            createKvl(markEl,element,eq[*j].left,n);
            eq[*j].nInc=nInc(eq[*j].left,element,n,eq,*j);
            printf("Vecchio j %d\n",*j);
            (*j)++;
            printf("Nuovo j %d\n",*j);
        }
        return;
    }
    markEl[pos]=1;
    kvlR(element,pos+1,n,eq,j,markEl);
    markEl[pos]=0;
    kvlR(element,pos+1,n,eq,j,markEl);
    return;
}
void kvlGenerator(element* element,int n,eq* eq,int* j){
    int* markEl=calloc(n,sizeof(int));
    kvlR(element,0,n,eq,j,markEl);
    free(markEl);
}
void kll(element* element,int node,int i,int n,char* eq){
    strcpy(eq,"");
    for(int j=i;j<n;j++){
        if(element[j].lastNode==node && element[j].type!='G'){
            strcat(eq,"- I");
            strcat(eq,element[j].name);
            strcat(eq," ");
        }
        if(element[j].firstNode==node && element[j].type!='G'){
            strcat(eq,"+ I");
            strcat(eq,element[j].name);
            strcat(eq," ");
        }
    }
}
       
int kllGenerator(element* element,int n,eq* eq){
    int* mark=calloc(n,sizeof(int));
    int j=0;
    for(int i=0;i<n;i++){
        if(!mark[element[i].lastNode] && element[i].lastNode!=0 && element[i].type!='G'){
            kll(element,element[i].lastNode,i,n,eq[j].left);
            eq[j].nInc=nInc(eq[j].left,element,n,eq,j);
            j++;
            mark[element[i].lastNode]=1;
        }
    }
    free(mark);
    return j;
}
*/
