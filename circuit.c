#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#define DIM 500
#define NAME 100

typedef struct{
    char left[DIM];
    char right[DIM];
    char** inc;
    int* markInc;
    int nInc;
    int startInc;
}eq;

typedef struct{
    int valid;
    char type;
    char name[DIM];
    int firstNode;
    int lastNode;
    double volt;
    double res;
    double curr;
    int voltKnown;
    int currKnown;
    int resKnown;
}element;
double resolvePar(eq* eq,element* elems,int index,int* i,int n){
    char typeOp='x',typeVar;
    double value=0;
    (*i)=(*i)+1;
    for(;(*i)<strlen(eq[index].left);(*i)=(*i)+1){
        //printf("\ni=%d car i=%c\n",*i,eq[index].left[*i]);
        if(eq[index].left[*i]=='+' || eq[index].left[*i]=='-' || eq[index].left[*i]=='*' || eq[index].left[*i]=='/') typeOp=eq[index].left[*i];
        if(eq[index].left[*i]=='V' || eq[index].left[*i]=='I' || eq[index].left[*i]=='R'){
            typeVar=eq[index].left[*i];
            char name[NAME];
            int k,j;
            for(j=(*i)+1,k=0;j<strlen(eq[index].left);j++,k++){
                if(eq[index].left[j]=='*' || eq[index].left[j]=='/' || eq[index].left[j]==' ' || eq[index].left[j]=='+' || eq[index].left[j]=='-' || eq[index].left[j]=='(' || eq[index].left[j]==')'){
                    (*i)=j-1;
                    break;
                }
                else name[k]=eq[index].left[j];;
            }
            name[k]='\0';
            double valueVar=0;
            //printf("\nVoglio cercare la variabile %s\n",name);
            for(int j=0;j<n;j++){
                //printf("\nnome var confrontata %s con %s\n",elems[j].name,name);
                if(!strcmp(name,elems[j].name)){
                    //printf("\nElemento uguale trovato %s, ha res=%f curr=%f volt=%f\n",elems[j].name,elems[j].res,elems[j].curr,elems[j].volt);
                    if(typeVar=='V') valueVar=elems[j].volt;
                    if(typeVar=='I') valueVar=elems[j].curr;
                    if(typeVar=='R') valueVar=elems[j].res;
                    break;
                }
            }
            //printf("\n\nLa VARIABILE %c%s aveva val: %f ora %f%c%f\n",typeVar,name,valueVar,value,typeOp,valueVar);
            if(typeOp=='x') value=valueVar;
            else if(typeOp=='+') value=value+valueVar;
            else if(typeOp=='-') value=value-valueVar;
            else if(typeOp=='*') value=value*valueVar;
            else if(typeOp=='/') value=value/valueVar;
            //printf("Res: %f typeop:%c\n",value,typeOp);
        }
        if(eq[index].left[*i]==')'){
            //printf("\n\nLa parentesi aveva val: %f\n",value);  
            //(*i)=(*i)+1;
            return value;
        }
        if(eq[index].left[*i]=='1' && (eq[index].left[(*i)-1]=='(' || eq[index].left[(*i)-1]==')' || eq[index].left[(*i)-1]=='*' || eq[index].left[(*i)-1]=='/' || eq[index].left[(*i)-1]=='-' || eq[index].left[(*i)-1]=='+')){
            //printf("\n\nTrovato 1 a %d typeop=%c val prima %lf\n",*i,typeOp,value);
            if(typeOp=='x') value=1;
            else if(typeOp=='+') value=value+1;
            else if(typeOp=='-') value=value-1;
            else if(typeOp=='*') value=value;
            else if(typeOp=='/') value=value;
            //printf("\nRes dopo 1 e' %lf\n",value);
        }
        if(eq[index].left[*i]=='('){
            //printf("\nChiamo parentesi a %d\n",*i);
            int parInd=(*i);
            if(typeOp=='x') value=resolvePar(eq,elems,index,i,n);
            else if(typeOp=='+') value=value+resolvePar(eq,elems,index,i,n);
            else if(typeOp=='-') value=value-resolvePar(eq,elems,index,i,n);
            else if(typeOp=='*') value=value*resolvePar(eq,elems,index,i,n);
            else if(typeOp=='/') value=value/resolvePar(eq,elems,index,i,n);
            //printf("\n\nHo chiamato la parentesi a %d mi e' ritornata a %d ed ha reso var=%lf dopo un %c\n",parInd,*i,value,typeOp);
        }
    }
    //printf("\n\nLa parentesi aveva val: %f\n",value);
    return value;
}
        
double resolveEq(eq* eq,element* elems,int index,int elemN,int n){
    //printf("\n\nEntrato in resolveEq con eq:%s e indice %d inc %s eq right %s\n",eq[index].left,index,eq[index].inc[0],eq[index].right);
    for(int i=0;i<n;i++){
        if(!elems[i].currKnown && elems[i].voltKnown && elems[i].resKnown) {
            elems[i].curr=elems[i].volt/elems[i].res; 
            elems[i].currKnown=1;
        }
        if(elems[i].currKnown && !elems[i].voltKnown && elems[i].resKnown) {
            elems[i].volt=elems[i].curr*elems[i].res;
            elems[i].voltKnown=1;
        }
        if(elems[i].currKnown && elems[i].voltKnown && !elems[i].resKnown) {
            elems[i].res=elems[i].volt/elems[i].curr;  
            elems[i].resKnown=1;
        }
    }
    char typeOp='x',typeVar;
    double value=0;
    for(int i=0;i<strlen(eq[index].left);i++){
        //printf("\ni=%d car i=%c\n",i,eq[index].left[i]);
        if(eq[index].left[i]=='+' || eq[index].left[i]=='-' || eq[index].left[i]=='*' || eq[index].left[i]=='/') typeOp=eq[index].left[i];
        if(eq[index].left[i]=='V' || eq[index].left[i]=='I' || eq[index].left[i]=='R'){
            typeVar=eq[index].left[i];
            char name[NAME];
            int k,j;
            for(j=(i)+1,k=0;j<strlen(eq[index].left);j++,k++){
                if(eq[index].left[j]=='*' || eq[index].left[j]=='/' || eq[index].left[j]==' ' || eq[index].left[j]=='+' || eq[index].left[j]=='-' || eq[index].left[j]=='(' || eq[index].left[j]==')'){
                    (i)=j-1;
                    break;
                }
                else name[k]=eq[index].left[j];
            }
            name[k]='\0';
            double valueVar=0;
            //printf("\nVoglio cercare la variabile %s\n",name);
            for(int j=0;j<n;j++){
                //printf("\nnome var confrontata %s con %s\n",elems[j].name,name);
                if(!strcmp(name,elems[j].name)){
                    //printf("\nElemento uguale trovato %s, ha res=%f curr=%f volt=%f\n",elems[j].name,elems[j].res,elems[j].curr,elems[j].volt);
                    if(typeVar=='V') valueVar=elems[j].volt;
                    if(typeVar=='I') valueVar=elems[j].curr;
                    if(typeVar=='R') valueVar=elems[j].res;
                    break;
                }
            }
            //printf("\n\nLa VARIABILE %c%s aveva val: %f ora %f%c%f\n",typeVar,name,valueVar,value,typeOp,valueVar);
            if(typeOp=='x') value=valueVar;
            else if(typeOp=='+') value=value+valueVar;
            else if(typeOp=='-') value=value-valueVar;
            else if(typeOp=='*') value=value*valueVar;
            else if(typeOp=='/') value=value/valueVar;
            //printf("Res: %f typeop:%c\n",value,typeOp);
        }
        if(eq[index].left[i]=='('){
            //printf("\nChiamo parentesi a %d\n",i);
            if(typeOp=='x') value=resolvePar(eq,elems,index,&i,n);
            else if(typeOp=='+') value=value+resolvePar(eq,elems,index,&i,n);
            else if(typeOp=='-') value=value-resolvePar(eq,elems,index,&i,n);
            else if(typeOp=='*') value=value*resolvePar(eq,elems,index,&i,n);
            else if(typeOp=='/') value=value/resolvePar(eq,elems,index,&i,n);
            //printf("\n\nLa parentesi dopo la %c ha reso var=%lf\n",typeOp,value);
        }
        if(eq[index].left[i]=='1' && (eq[index].left[(i)-1]=='(' || eq[index].left[(i)-1]==')' || eq[index].left[(i)-1]=='*' || eq[index].left[(i)-1]=='/' || eq[index].left[(i)-1]=='-' || eq[index].left[(i)-1]=='+')){
            //printf("\n\nTrovato 1 a %d typeOp=%c val prima %lf\n",i,typeOp,value);
            if(typeOp=='x') value=1;
            else if(typeOp=='+') value=value+1;
            else if(typeOp=='-') value=value-1;
            else if(typeOp=='*') value=value;
            else if(typeOp=='/') value=value;
            //printf("\nRes dopo 1 e' %lf\n",value);
        }
        //if(eq[index].left[i]==')'){
         //   (i)=(i)+1
           // return value;
        //}
    }
    char typeInc=eq[index].inc[0][0];
    //if(value<0) value=(-value);
    if(typeInc=='V'){
        //printf("\n\nLa Tensione aveva val: %f\n",value);
        elems[elemN].volt=value;
        elems[elemN].voltKnown=1;
    }
    if(typeInc=='I'){
        //printf("\n\nLa Corrente aveva val: %f\n",value);
        elems[elemN].curr=value;
        elems[elemN].currKnown=1;
    }
    if(typeInc=='R'){
        //printf("\n\nLa Resistenza aveva val: %f\n",value);
        elems[elemN].res=value;
        elems[elemN].resKnown=1;
    }
    for(int i=0;i<n;i++){
        if(!elems[i].currKnown && elems[i].voltKnown && elems[i].resKnown) {
            elems[i].curr=elems[i].volt/elems[i].res; 
            elems[i].currKnown=1;
        }
        if(elems[i].currKnown && !elems[i].voltKnown && elems[i].resKnown) {
            elems[i].volt=elems[i].curr*elems[i].res;
            elems[i].voltKnown=1;
        }
        if(elems[i].currKnown && elems[i].voltKnown && !elems[i].resKnown) {
            elems[i].res=elems[i].volt/elems[i].curr;  
            elems[i].resKnown=1;
        }
    }
    return value;
}

int nInc(char* equaz,element* elems,int n, eq* equ,int nEqu){
    //printf("Entrato nella NINC con %s n=%d\n",equaz,nEqu);
    int* markV=calloc(n,sizeof(int));
    int* markI=calloc(n,sizeof(int));
    int* markR=calloc(n,sizeof(int));
    int nInc=0;
    for(int i=0;i<strlen(equaz);i++){
        //printf("INIZIO CICLO %d NEQU %d CAR %c\n",i,nEqu,equaz[i]);
        if(equaz[i]=='V' || equaz[i]=='R' || equaz[i]=='I'){
            char type=equaz[i];
            char name[NAME];
            int k,j;
            for(j=i+1,k=0;j<strlen(equaz);j++,k++){
                if(equaz[j]=='*' || equaz[j]=='/' || equaz[j]==' ' || equaz[j]=='+' || equaz[j]=='-' || equaz[j]=='(' || equaz[j]==')')  break;
                else name[k]=equaz[j];
            }
            i=j;
            //printf("NUOVO I DOPO AVER TROVATO INCOGNITA %d STRLEN EQUAZ %d\n",i,strlen(equaz));
            if(i>=strlen(equaz)) i=strlen(equaz)-1;
            name[k]='\0';
            //printf("CONTINUO CICLO %d NEQU %d NAME: %s\n",i,nEqu,name);
            for(j=0;j<n;j++){
                if(!strcmp(name,elems[j].name)){
                    if(type=='V'){
                        if(!markV[j]){
                            if(!elems[j].voltKnown && (!elems[j].resKnown || !elems[j].currKnown)){
                                if(equ[nEqu].startInc>0){
                                    //strcpy(equ[nEqu].inc[nInc],"");
                                    //strcpy(equ[nEqu].inc[nInc],"V");
                                    //strcat(equ[nEqu].inc[nInc],name);
                                    if(equ[nEqu].inc[nInc]!=NULL){
                                        equ[nEqu].inc[nInc][0]='V';
                                        int s;
                                        for(s=0;s<strlen(name);s++) equ[nEqu].inc[nInc][s+1]=name[s];
                                        equ[nEqu].inc[nInc][s+1]='\0';
                                    }
                                }
                                nInc++;
                                //printf("Letto incognita %c %s in %d\n",type,name,nEqu);
                            }
                            markV[j]=1;
                        }
                    }
                    if(type=='I'){
                        if(!markI[j]){
                            if(!elems[j].currKnown && (!elems[j].voltKnown || !elems[j].resKnown)){
                                if(equ[nEqu].startInc>0){
                                    //strcpy(equ[nEqu].inc[nInc],"");
                                    //strcpy(equ[nEqu].inc[nInc],"I");
                                    //strcat(equ[nEqu].inc[nInc],name);
                                    if(equ[nEqu].inc[nInc]!=NULL){
                                        equ[nEqu].inc[nInc][0]='I';
                                        int s;
                                        for(s=0;s<strlen(name);s++) equ[nEqu].inc[nInc][s+1]=name[s];
                                        equ[nEqu].inc[nInc][s+1]='\0';
                                    }
                                }
                                nInc++;
                                //printf("Letto incognita %c %s in %d\n",type,name,nEqu);
                            }
                            markI[j]=1;
                        }
                    }
                    if(type=='R'){
                        if(!markR[j]){
                            if(!elems[j].resKnown && (!elems[j].voltKnown || !elems[j].currKnown)) {
                                if(equ[nEqu].startInc>0){
                                    //strcpy(equ[nEqu].inc[nInc],"");
                                    //strcpy(equ[nEqu].inc[nInc],"R");
                                    //strcat(equ[nEqu].inc[nInc],name);
                                    if(equ[nEqu].inc[nInc]!=NULL){
                                        equ[nEqu].inc[nInc][0]='R';
                                        int s;
                                        for(s=0;s<strlen(name);s++) equ[nEqu].inc[nInc][s+1]=name[s];
                                        equ[nEqu].inc[nInc][s+1]='\0';
                                    }
                                }
                                nInc++;
                                //printf("Letto incognita %c %s in %d\n",type,name,nEqu);
                            }
                            markR[j]=1;
                        }
                    }
                    break;
                }
            }
        }
    }
    //printf("FINE NINC\n");
    free(markV);
    free(markI);
    free(markR);
    //printf("FINE FREE NINC\n");
    return nInc;
}

void readCircuit(element* elems,int n,char* fileName){
    FILE* fp;
    fp=fopen(fileName,"r");
    char car;
    for(int i=0;i<n;i++){
        fscanf(fp,"%c %s %d %d ",&(elems[i].type),elems[i].name,&(elems[i].firstNode),&(elems[i].lastNode));
        fscanf(fp,"%c ",&car);
        if(car!='x'){
            fscanf(fp,"%lf ",&(elems[i].curr));
            elems[i].currKnown=1;
        }
        else elems[i].currKnown=0;
        fscanf(fp,"%c ",&car);
        if(car!='x'){
            fscanf(fp,"%lf ",&(elems[i].volt));
            elems[i].voltKnown=1;
        }
        else elems[i].voltKnown=0;
        fscanf(fp,"%c ",&car);
        if(car!='x'){
            fscanf(fp,"%lf ",&(elems[i].res));
            elems[i].resKnown=1;
        }
        else elems[i].resKnown=0;
        elems[i].valid=1;
    }
    fclose(fp);
}

void printCircuit(element* elems,int n){
    for(int i=0;i<n;i++){
        if(elems[i].valid==1){
            printf("Type:%c Name:%s firstNode:%d lastNode:%d ",elems[i].type,elems[i].name,elems[i].firstNode,elems[i].lastNode);
            if(!elems[i].currKnown) printf("Curr: - ");
            else printf("Curr: %f ",elems[i].curr);
            if(!elems[i].voltKnown) printf("Volt: - ");
            else printf("Volt: %f ",elems[i].volt);
            if(!elems[i].resKnown) printf("Res: - \n");
            else printf("Res: %f \n",elems[i].res);
        }
    }
}

int raccogliEMoltiplicaASinistra(char* toRac,char* inc,char* left,int change){
    int match=0,startSeq,cont=0;
    char buf[DIM];
    int nPar=0,firsPar=-1;
    for(int i=0;i<strlen(toRac);i++){
        buf[cont]=toRac[i];
        if(toRac[i]==inc[match]){
            if(match==0) startSeq=i;
            match++;
            if(match==strlen(inc)){
                cont=cont-strlen(inc)+1;
                buf[cont]='X';
                match=0;
            }
        }
        else    match=0;
        cont++;
    }
    buf[cont]='\0';
    int nonRacc=0;
    for(int i=0;i<cont && nonRacc==0;i++){
        if(buf[i]=='('){
            if(!nPar)   firsPar=cont;
            nPar++;
        }
        if(buf[i]==')'){
            nPar--;
            if(!nPar)   firsPar=-1;
        }
        if(buf[i]=='X' && nPar!=0){
            /*int nParX=nPar,nDiv=0,nDec=0,nMolt=0;
            for(int j=i-1;nParX>0;j--){
                if(buf[j]=='('){
                    nDec++;                    
                    nParX--;
                }
                if(buf[j]==')') nParX++;
                if(buf[j]=='/' && (nParX<nPar || (nParX==nPar && nDec==0))) nDiv++;
            }
            if(nDiv%2) nonRacc=1; */
            nonRacc=1;
        }
    }
    for(int i=0;i<strlen(buf);i++) if(buf[i]=='X')  buf[i]='1';
    if(nonRacc==1){
        //printf("ERRORE !!! NON RACCOGLIBILE\t NON RACCOGLIBILE!!!!!\n");
        return 1;
    }
    else if(change==0 && nonRacc==0) return 0;
    if(!strcmp(buf,"+1")){
        strcpy(toRac,"+");
        strcat(toRac,inc);
    }
    else if(!strcmp(buf,"-1")){
        strcpy(toRac,"+");
        strcat(toRac,inc);
        int par=0;
        for(int i=0;i<strlen(left);i++){
            //printf("\nCar letto in raccol sx:%c par=%d a %d\n",left[i],par,i);
            if(left[i]=='(')    par++;
            if(left[i]==')')  par--;
            if(!par && left[i]=='+')left[i]='-';
            else if(!par && left[i]=='-') left[i]='+';
        }
        //printf("\nRaccol sx:%s\n",left);
    }
    else{
        strcpy(toRac,"+");
        strcat(toRac,inc);
        char changeBuf[DIM];
        strcpy(changeBuf,"+(");
        strcat(changeBuf,left);
        strcat(changeBuf,")/(");
        strcat(changeBuf,buf);
        strcat(changeBuf,")");
        strcpy(left,changeBuf);
    }
    //printf("RACcc:Left: %s --Right: %s --buf: %s\n",left,toRac,buf);
    return 0;
}

void sost(char* eq,char* toSost,char* sostEq,element element){
    //printf("\nENTRATO IN SOST CON EQ %s TOSOST %sNEW VAL %s\n",eq,toSost,sostEq);
    int match=0,startSeq,cont=0,hasPar=0;
    for(int i=0;i<strlen(sostEq);i++){
        if(sostEq[i]=='('){
            hasPar=1;
            break;
        }
    }
    char buf[DIM];
    for(int i=0;i<strlen(eq);i++){
        //printf("INIZIO CICLO SOST %d CAR %c TOSOST[0] %c ELEMENT NAME %s\n",i,eq[i],toSost[0],element.name);
        buf[cont]=eq[i];
        if(eq[i]==toSost[match] || (match==0 && ((toSost[0]=='V' && eq[i]=='I' && element.resKnown) || (toSost[0]=='I' && eq[i]=='V' && element.resKnown) || (toSost[0]=='V' && eq[i]=='R' && element.currKnown) || (toSost[0]=='I' && eq[i]=='R' && element.voltKnown) || (toSost[0]=='R' && eq[i]=='V' && element.currKnown) || (toSost[0]=='R' && eq[i]=='I' && element.voltKnown)))){
            //printf("START MATCH STRING IN SOST INC %s NEW VAL %s\n",toSost,sostEq);
            if(match==0) startSeq=i;
            match++;
            if(match==strlen(toSost)){
                //printf("MATCHED STRING IN SOST INC %s NEW VAL %s\n",toSost,sostEq);
                cont=cont-strlen(toSost)+1;
                if(eq[startSeq-1]=='*' || eq[startSeq-1]=='/' || eq[startSeq+strlen(toSost)]=='*' || eq[startSeq+strlen(toSost)]=='/'){
                    buf[cont]='(';
                    cont++;
                }
                if(eq[startSeq]!=toSost[0]){
                    buf[cont]='(';
                    cont++;
                    if(toSost[0]=='V'){
                        buf[cont]='(';
                        cont++;
                    }      
                }
                if(eq[startSeq]!=toSost[0] && toSost[0]!='V'){
                    for(int k=0;k<strlen(toSost);k++){
                        if(k==0 && eq[startSeq]=='V' && toSost[0]=='R')  buf[cont]='I';
                        else if(k==0 && eq[startSeq]=='V' && toSost[0]=='I')  buf[cont]='R';
                        else if (k==0 && eq[startSeq]=='I') buf[cont]='V';
                        else if (k==0 && eq[startSeq]=='R') buf[cont]='V';
                        else buf[cont]=toSost[k];
                        cont++;
                    }
                    if((eq[startSeq]=='I' && toSost[0]=='R') || (eq[startSeq]=='R' && toSost[0]=='I'))  buf[cont]='/';
                    else   buf[cont]='*';
                    cont++;
                    buf[cont]='(';
                    cont++;
                }
                if(hasPar)  for(int k=1;k<strlen(sostEq);k++,cont++)    buf[cont]=sostEq[k];
                else{
                    char signNoPar=buf[cont-1];
                    //printf("\nCHAR SIGNNOPAR %c buf prima di noPar %s\n",buf[cont-1],buf);
                    int changeSigns=0;
                    if(signNoPar=='-'){
                        cont=cont-1;
                        for(int k=0;k<strlen(sostEq);k++,cont++){
                            if(sostEq[k]=='+') buf[cont]='-';
                            else if(sostEq[k]=='-')  buf[cont]='+';
                            else buf[cont]=sostEq[k];
                        }
                    }
                    else if(signNoPar=='+'){
                        cont=cont-1;
                        for(int k=0;k<strlen(sostEq);k++,cont++)    buf[cont]=sostEq[k];
                    }
                    else {for(int k=0;k<strlen(sostEq);k++,cont++)    buf[cont]=sostEq[k];}
                    //printf("\nBuf dopo noPar: %s --cont=%d\n",buf,cont);
                }
                if(eq[startSeq]!=toSost[0] && toSost[0]=='V'){
                    //printf("\nBuf appena entrato in V: %s --cont=%d\n",buf,cont);
                    buf[cont]=')';
                    cont++;
                    buf[cont]='/';
                    cont++;
                    //printf("\nBuf prima del for di V: %s --cont=%d\n",buf,cont);
                    for(int k=0;k<strlen(toSost);k++){
                        if(k>0) buf[cont]=toSost[k];
                        else if(!k && eq[startSeq]=='R') buf[cont+1]='I';
                        else if(!k && eq[startSeq]=='I'){
                      //      printf("\nV: inizio var scrivo %c in %d\n",'R',cont);
                            buf[cont]='R';}
                        cont++;
                    }
                    //printf("\nBuf dopo V: %s --cont=%d\n",buf,cont);
                }
                cont--;
                if(eq[startSeq]!=toSost[0]){
                    cont++;
                    buf[cont]=')';
                    if(toSost[0]!='V'){
                        cont++;
                        buf[cont]=')';
                    }                
                }
                if(eq[startSeq-1]=='*' || eq[startSeq-1]=='/' || eq[startSeq+strlen(toSost)]=='*' || eq[startSeq+strlen(toSost)]=='/'){
                    cont++;
                    buf[cont]=')';
                }
                match=0;
            }
        }
        else    match=0;
        cont++;
    }
    buf[cont]='\0';
    //printf("\nSOST:Eq di partenza: %s\nEq finale: %s\n",eq,buf);
    int i;
    for(i=0;i<strlen(buf);i++) eq[i]=buf[i];
    eq[i]='\0';
}



void changeVar(char* eq,char* old,char* new,element element){
    //printf("\nENTRATO IN change var INC %s NEW VAL %s\n",old,new);
    int match=0,startSeq,cont=0;
    char buf[DIM];
    for(int i=0;i<strlen(eq);i++){
        //printf("INIZIO CICLO SOST %d CAR %c OLD %s NEW %s\n",i,eq[i],old,new);
        buf[cont]=eq[i];
        if(eq[i]==old[match]){
            //printf("START MATCH STRING IN SOST INC %s NEW VAL %s\n",toSost,sostEq);
            if(match==0) startSeq=i;
            match++;
            if(match==strlen(old)){
                //printf("MATCHED STRING IN SOST INC %s NEW VAL %s\n",old,new);
                cont=startSeq;
                buf[cont]='(';
                cont++;
                if(new[0]!='V'){
                    for(int k=0;k<strlen(old);k++){
                        if(k==0 && old[0]=='V' && new[0]=='R')  buf[cont]='I';
                        else if(k==0 && old[0]=='V' && new[0]=='I')  buf[cont]='R';
                        else if (k==0 && old[0]=='I') buf[cont]='V';
                        else if (k==0 && old[0]=='R') buf[cont]='V';
                        else buf[cont]=new[k];
                        cont++;
                    }
                    if(old[i]=='I' || old[0]=='R')  buf[cont]='/';
                    else   buf[cont]='*';
                    cont++;
                    for(int k=0;k<strlen(new);k++){
                        buf[cont]=new[k];
                        cont++;
                    }
                }
                else{
                    for(int k=0;k<strlen(new);k++){
                        buf[cont]=new[k];
                        cont++;
                    }
                    buf[cont]='/';
                    cont++;
                    for(int k=0;k<strlen(old);k++){
                        if(k==0 && old[0]=='R')  buf[cont]='I';
                        else if(k==0 && old[0]=='I')  buf[cont]='R';
                        else buf[cont]=new[k];
                        cont++;
                    }
                }
                buf[cont]=')';
                match=0;
            }
        }
        else    match=0;
        cont++;
    }
    buf[cont]='\0';
    //printf("Change var:Eq di partenza: %s\nEq finale: %s\n",eq,buf);
    int i;
    for(i=0;i<strlen(buf);i++) eq[i]=buf[i];
    eq[i]='\0';
}



void portaIncADestra(eq* eq,int incIndex){
    char buf[DIM];
    //printf("INCOGNITA DA PORTARE A DESTRA: %s eq: %s = %s\n",eq[index].inc[incIndex],eq[index].left,eq[index].right);
    //if(eq[index].right[0]!='+' || eq[index].right[0]!='-' || eq[index].right[0]!='0'){
      //  eq[index].right[0]='0';
        //eq[index].right[1]='\0';
    //}
    int match=0,par=0,firstPar;
    strcpy(buf,(*eq).left);
    for(int i=0;i<strlen((*eq).left);i++){
      //  printf("\nI=%d letto %c\n",i,eq[index].left[i]);
        if((*eq).left[i]=='('){
            par++;
            if(par==1) firstPar=i;
        }
        if((*eq).left[i]==')'){
            par--;
            if(par==0) firstPar=(-1);
        }
        if((*eq).left[i]==(*eq).inc[incIndex][match]){
            match++;
            if(match==strlen((*eq).inc[incIndex])){
                if(par>0){
                    char sign=(*eq).left[firstPar-1];
                    buf[firstPar-1]='X';
                    if(!strcmp((*eq).right,"0")){
                        //printf("\nAVEVO 0 A DESTRA!!\n");
                        if(sign=='+') (*eq).right[0]='-';
                        else if(sign=='-')  (*eq).right[0]='+';
                        int insPar=1,l=1;
                        for(int k=firstPar,s=0;insPar>0;k++,l++,s++){
                            if((*eq).left[k]=='(' && s>0)   insPar++;
                            if((*eq).left[k]==')'){
                                insPar--;
                                if(!insPar){
                                    i=k;
                                    par=0;
                                    firstPar=(-1);
                                }
                            }
                            (*eq).right[l]=(*eq).left[k];
                            buf[k]='X';
                        }
                        (*eq).right[l]='\0';
                    }
                    else{
                        //printf("\nNON AVEVO 0 A DESTRA!! FIRST PAR=%d\n",firstPar);
                        if(sign=='+') strcat((*eq).right,"-");
                        else if(sign=='-')  strcat((*eq).right,"+");
                        char bufPar[DIM];
                        int l=0,insPar=0;
                        for(int k=firstPar;insPar>0 || (k==firstPar && insPar==0);k++,l++){
                            //printf("\nCiclo di lettura parentesi k=%d letto %c inspar %d l %d\n",k,eq[index].left[k],insPar,l);
                            if((*eq).left[k]=='(')   insPar++;
                            if((*eq).left[k]==')'){
                                insPar--;
                                if(!insPar){
                                    i=k;
                                    par=0;
                                    firstPar=(-1);
                                }
                            }
                            bufPar[l]=(*eq).left[k];
                            buf[k]='X';
                        }
                        bufPar[l]='\0';
                        strcat((*eq).right,bufPar);
                    }
                }
                else{
                    int signIndex=0;
                    char sign;
                    for(int s=i;s>=0;s--){
                        if((*eq).left[s]=='+'){
                            signIndex=s;
                            sign='+';
                            break;
                        }
                        if((*eq).left[s]=='-'){
                            signIndex=s;
                            sign='-';
                            break;
                        }
                    }
                    buf[signIndex]='X';
                    if(!strcmp((*eq).right,"0")){
                        //printf("\nAVEVO 0 A DESTRA!!\n");
                        if(sign=='+') (*eq).right[0]='-';
                        else if(sign=='-')  (*eq).right[0]='+';
                        int l,k;
                        for(l=1,k=0;k<strlen((*eq).inc[incIndex]);k++,l++){
                            (*eq).right[l]=(*eq).inc[incIndex][k];
                            buf[signIndex+l]='X';
                        }
                        (*eq).right[l]='\0';
                    }
                    else{
                       // printf("\nNON AVEVO 0 A DESTRA!!\n");
                        if(sign=='+') strcat((*eq).right,"-");
                        else if(sign=='-')  strcat((*eq).right,"+");
                        strcat((*eq).right,(*eq).inc[incIndex]);
                        int l,k;
                        for(l=1,k=0;k<strlen((*eq).inc[incIndex]);k++,l++)  buf[signIndex+l]='X';
                    }
                }
                  
            }
        }
        else    match=0;
    }
    //printf("\nArrivato qua 2\n");
    strcpy((*eq).left,"");
    int z=0;
    for(int k=0;k<strlen(buf);k++){
        if(buf[k]!='X'){
            (*eq).left[z]=buf[k];
            z++;
        }
    }
    (*eq).left[z]='\0';
    //printf("Porta inc a d: Left eq: %s\nRight eq: %s\n",(*eq).left,(*eq).right);
}
void resolveOneInc(eq* eq,element* element){
    //printf("\n\nEntrato in RESOLVEONEINC con equazione %s = %s\n",(*eq).left,(*eq).right);
    int match=0,startSeq,cont=0;
    char buf[DIM];
    int par=0,firstPar=-1;
    for(int i=0;i<strlen((*eq).right);i++){
        buf[cont]=(*eq).right[i];
        if((*eq).right[i]==(*eq).inc[0][match]){
            if(match==0) startSeq=i;
            match++;
            if(match==strlen((*eq).inc[0])){
                cont=cont-strlen((*eq).inc[0])+1;
                buf[cont]='X';
                match=0;
            }
        }
        else    match=0;
        cont++;
    }
    buf[cont]='\0';
    char raccol[DIM];
    strcpy(raccol,"");
    //printf("\n\nRESOLVEONEINC BUF %s n.%d e inc %s raccol %s\n",buf,index,inc,raccol);
    for(int i=0;i<strlen(buf);i++){
        if(buf[i]=='('){
            par++;
            if(par==1) firstPar=i;
        }
        if(buf[i]==')'){
            par--;
            if(par==0) firstPar=(-1);
        }
        if(buf[i]=='X'){
            if(par>0){
                int nParMax=0,nParX=0;
                for(int j=firstPar;nParX>0 && j!=firstPar;j++){
                    if(buf[j]=='(') nParX++;
                    if(buf[j]==')')  nParX--;
                    if(buf[j]=='X' && (nParX>nParMax)) nParMax=nParX;
                }
                char bufferInc[DIM];
                int incIndex=0;
                nParX=0;
                char raccSign=buf[firstPar-1];
                for(int j=firstPar;nParX>0 || (nParX==0 && j==firstPar);j++){
                    //if(j==firstPar)printf("Entrato nella ricerca dentro le par car %c\n",buf[j]);
                    if(buf[j]=='(') nParX++;
                    if(buf[j]==')'){
                        nParX--;
                        if(nParX==0){
                            i=j;
                            par=0;
                            firstPar=(-1);
                        }
                    }
                    if((buf[j]=='+' || buf[j]=='-') && (buf[j+1]!='(' && buf[j+1]!='X') && (nParX>nParMax)){
                        //printf("Entrato con j=%d car=%c car dopo=%c\n",j,buf[j],buf[j+1]);
                        for(int k=j+1;k<strlen(buf);k++){
                            if(buf[k]=='+' || buf[k]=='-' || buf[k]==')' || buf[k]=='('){
                                j=k-1;
                                break;
                            }
                        }
                        //printf("Uscito con j=%d\n",j);
                    }
                    else{
                        if(buf[j]=='X') bufferInc[incIndex]='1';
                        else bufferInc[incIndex]=buf[j];
                        incIndex++;
                    }
                }
                bufferInc[incIndex]='\0';
                //printf("\nONEINC: Raccol prima:\n%s\nRaccolgo: %s\n",raccol,bufferInc);
                if(raccSign=='-')   strcat(raccol,"-");
                else   strcat(raccol,"+");
                strcat(raccol,bufferInc);
                //printf("\nONEINC: Raccol dopo:\n%s\n",raccol);
            }
            else{
                if(buf[i-1]=='+')   strcat(raccol,"+");
                if(buf[i-1]=='-')   strcat(raccol,"-");
                strcat(raccol,"1");
            }
        }
    }   
        par=0;
        for(int i=0;i<strlen(buf);i++){
            if(buf[i]=='(') par++;
            if(buf[i]==')')  par--;
            if(buf[i]=='+' && !par) buf[i]='-';
            if(buf[i]=='-' && !par) buf[i]='+';
            if(buf[i]=='+' || buf[i]=='-' || buf[i]=='*' || buf[i]=='/' || buf[i]=='X'){
                if(buf[i]!='X' && buf[i+1]=='X'){
                    //buf[i]='X';
                    if(buf[i]=='*' || buf[i]=='/'){
                        //printf("A %d ho *// e dopo X car %c car prima %c\n",i,buf[i],buf[i-1]);
                        for(int k=i-1;k>=0;k--){
                            if(buf[k]=='('){
                                //printf("Parentesi a %d quindi metto 'x' anche a %d car %c\n",k,k-1,buf[k]);
                                buf[k-1]='X';
                                buf[k]='X';
                                break;
                            }
                            else buf[k]='X';
                        }
                        for(int k=i+1;k>=0;k++){
                            if(buf[k]==')'){
                                buf[k]='X';
                                break;
                            }
                            else buf[k]='X';
                        }
                    }
                    buf[i]='X';
                }
                else if(buf[i]=='X' && (buf[i+1]=='*' || buf[i+1]=='/')){
                    //printf("A %d ho X e dopo *//\n",i);
                    for(int k=i-1;k>=0;k--){
                        if(buf[k]=='('){
                            buf[k-1]='X';
                            buf[k]='X';
                            break;
                        }
                        else buf[k]='X';
                    }
                    for(int k=i+1;k>=0;k++){
                        if(buf[k]==')'){
                            buf[k]='X';
                            break;
                        }
                        else buf[k]='X';
                    }
                }
            }
        }
        char bufLeft[DIM];
        strcpy(bufLeft,"");
        int indexLeft=0;
        for(int i=0;i<strlen(buf);i++){    
            if(buf[i]!='X'){
                bufLeft[indexLeft]=buf[i];
                indexLeft++;
            }
        }
        bufLeft[indexLeft]='\0';
        par=0;
        for(int i=0;i<indexLeft;i++){
            if(bufLeft[i]=='(') par++;
            if(bufLeft[i]==')') par--;
            if(bufLeft[i]=='+' && !par) bufLeft[i]='-';
            else if(bufLeft[i]=='+' && !par) bufLeft[i]='+';
        }
        int l=0;
        (*eq).right[l]='+';
        l++;
        for(int k=0;k<strlen((*eq).inc[0]);k++,l++) (*eq).right[l]=(*eq).inc[0][k];
        (*eq).right[l]='\0';
        //printf("\n\nBufLeft:%s\nRaccol:%s\n",bufLeft,raccol);
        if(indexLeft>0) strcat((*eq).left,bufLeft);
        if(!strcmp(raccol,"+1")){
        }
        else if(!strcmp(raccol,"-1")){
            par=0;
            for(int i=0;i<strlen((*eq).left);i++){
                if((*eq).left[i]=='(')    par++;
                if((*eq).left[i]==')')  par--;
                if(!par && (*eq).left[i]=='+') (*eq).left[i]='-';
                if(!par && (*eq).left[i]=='-') (*eq).left[i]='+';
            }
        }
        else{
            strcpy(bufLeft,"+(");
            strcat(bufLeft,(*eq).left);
            strcat(bufLeft,")/(");
            strcat(bufLeft,raccol);
            strcat(bufLeft,")");
            strcpy((*eq).left,bufLeft);
        }
        //printf("\nEQ FINALE:\n%s = %s\n",(*eq).left,(*eq).right);
    }
            
int noInc(eq* eq,int n){
    for(int i=0;i<n;i++)    if(eq[i].nInc>0)    return 0;
    return 1;
}
void resolve(element* elems,eq* eq,int n){
    int minIndex=-1,min=-1,zz=0;
    while(!noInc(eq,n)){
        //printf("Nuovo ciclo resolve\n");
        minIndex=-1,min=-1;
        for(int i=0;i<n;i++){
            if(min==-1 && strcmp(eq[i].right,"0")==0){
                min=eq[i].nInc;
                minIndex=i;
            }
            else if(eq[i].nInc<min && strcmp(eq[i].right,"0")==0){
                min=eq[i].nInc;
                minIndex=i;
            }
        }
        if(minIndex>=0){
            for(int i=0;i<eq[minIndex].nInc;i++){
                if(!eq[minIndex].markInc[i] && !raccogliEMoltiplicaASinistra(eq[minIndex].left,eq[minIndex].inc[i],eq[minIndex].left,0)){
                    //printf("\n\nScelta incognita %s nell'equazione n.%d\n",eq[minIndex].inc[i],minIndex);
                    portaIncADestra(&eq[minIndex],i);
                    raccogliEMoltiplicaASinistra(eq[minIndex].right,eq[minIndex].inc[i],eq[minIndex].left,1);
                    element element;
                    for(int j=0;j<n;j++){
                        int uguali=1;
                        for(int s=1;s<strlen(eq[minIndex].inc[i]) && s<(strlen(elems[j].name)+1);s++){
                            if(eq[minIndex].inc[i][s]!=elems[j].name[s-1]){
                                uguali=0;
                                break;
                            }
                        }
                        if(uguali==1){
                            element=elems[j];
                            break;
                        }
                    }
                    for(int j=0;j<n;j++){
                        if(j!=minIndex){
                            char bufSost[DIM];
                            strcpy(bufSost,eq[j].left);
                            int startInc=eq[j].nInc;
                            if(strcmp(eq[j].right,"0")){
                                strcat(bufSost," = ");
                                strcat(bufSost,eq[j].right);
                            }
                            sost(bufSost,eq[minIndex].inc[i],eq[minIndex].left,element);
                            int newInc=nInc(bufSost,elems,n,eq,j);
                            //printf("\tSTART INC: %d START EQ: %s=%s\n\tNEW INC: %d NEW EQ: %s\n",startInc,eq[j].left,eq[j].right,newInc,bufSost);
                            if(newInc<=startInc) {
                                sost(eq[j].left,eq[minIndex].inc[i],eq[minIndex].left,element);
                                if(strcmp(eq[j].right,"0")) sost(eq[j].right,eq[minIndex].inc[i],eq[minIndex].left,element);
                                eq[j].nInc=newInc;
                            }
                            else{ 
                                strcpy(bufSost,"");
                                if(strcmp(eq[j].right,"0")){
                                    strcat(bufSost," = ");
                                    strcat(bufSost,eq[j].right);
                                }
                                eq[j].nInc==nInc(bufSost,elems,n,eq,j);
                            }
                        }
                    }
                    break;
                }
            }   
        }
        for(int i=0;i<n;i++){
            for(int j=0;j<eq[i].nInc;j++){
                for(int k=j+1;k<eq[i].nInc;k++){
                    int uguali=1;
                    for(int s=0;s<strlen(eq[i].inc[j]) && s<strlen(eq[i].inc[k]);s++){
                        if(s>0 && eq[i].inc[j][s]!=eq[i].inc[k][s]){
                            uguali=0;
                            break;
                        }
                    }
                    if(uguali==1){
                        element element;
                        for(int s=0;s<n;s++){
                            int same=1;
                            for(int d=1;d<strlen(eq[i].inc[k]) && d<(strlen(elems[s].name)+1);d++){
                                if(eq[i].inc[k][d]!=elems[s].name[d-1]){
                                    same=0;
                                    break;
                                }
                            }
                            if(same==1){
                                element=elems[s];
                                break;
                            }
                        }
                        changeVar(eq[i].left,eq[i].inc[j],eq[i].inc[k],element);
                        if(strcmp(eq[i].right,"0")) changeVar(eq[i].right,eq[i].inc[j],eq[i].inc[k],element);
                        break;
                    }
                }
            }
            char bufSost[DIM];
            strcpy(bufSost,eq[i].left);
            if(strcmp(eq[i].right,"0")){
                strcat(bufSost," = ");
                strcat(bufSost,eq[i].right);
            }
            eq[i].nInc=nInc(bufSost,elems,n,eq,i);
        }
        for(int i=0;i<n;i++){
            if(eq[i].nInc==1){
                for(int s=0;s<n;s++){
                    int same=1;
                    for(int d=1;d<strlen(eq[i].inc[0]) && d<(strlen(elems[s].name)+1);d++){
                        if(eq[i].inc[0][d]!=elems[s].name[d-1]){
                            same=0;
                            break;
                        }
                    }
                    if(same==1){
                        portaIncADestra(&eq[i],0);
                        resolveOneInc(&eq[i],&elems[s]);
                        resolveEq(eq,elems,i,s,n);
                        elems[s].resKnown=1;
                        elems[s].voltKnown=1;
                        elems[s].currKnown=1;
                        for(int l=0;l<n;l++){
                            char bufSost[DIM];
                            strcpy(bufSost,eq[l].left);
                            if(strcmp(eq[l].right,"0")){
                                strcat(bufSost," = ");
                                strcat(bufSost,eq[l].right);
                            }
                            eq[l].nInc=nInc(bufSost,elems,n,eq,l);
                        }
                        break;
                    }
                }
            }
        }
        //printf("\nLe equazioni generate sono le seguenti:\n");
        //for(int i=0;i<n;i++){
            //if(eq[i].left!=NULL && eq[i].right!=NULL){
                //printf("eq n. %d ---- %s = %s ---- N inc: %d\n",i,eq[i].left,eq[i].right,eq[i].nInc);
                //printf("Inc:\t");
                //for(int j=0;j<eq[i].nInc;j++)   if(eq[i].inc[j]!=NULL)  printf("%s ",eq[i].inc[j]);
                //printf("\n");
            //}
        //}
        //zz++;
    }
    for(int i=0;i<n;i++){
        if(!elems[i].currKnown && elems[i].voltKnown && elems[i].resKnown) {
            elems[i].curr=elems[i].volt/elems[i].res; 
            elems[i].currKnown=1;
        }
        if(elems[i].currKnown && !elems[i].voltKnown && elems[i].resKnown) {
            elems[i].volt=elems[i].curr*elems[i].res;
            elems[i].voltKnown=1;
        }
        if(elems[i].currKnown && elems[i].voltKnown && !elems[i].resKnown) {
            elems[i].res=elems[i].volt/elems[i].curr;  
            elems[i].resKnown=1;
        }
    }
}     
        
    

int main(int argc,char** argv){
    if(argc!=3) exit(0);
    element* elems;
    int n=atoi(argv[1]);
    eq* eq;
    eq=malloc(n*sizeof(eq));
    elems=malloc(n*sizeof(element));
    readCircuit(elems,n,argv[2]);
    for(int i=0;i<n;i++){
        strcpy(eq[i].left,"");
        eq[i].right[0]='0';eq[i].right[1]='\0';
    }
    strcpy(eq[0].left,"-IR0+IR1+IR2");
    strcpy(eq[1].left,"-IR2+IR3");
    //strcpy(eq[2].left,"+VG0-VG0+(RR1*IR1)-(RR2*(+(((RR3*(((+VG0-(RR1*IR1))/RR0)+IR1)))/RR3)))-(((RR3*(((+VG0-(RR1*IR1))/RR0)+IR1))))");
    //strcpy(eq[3].left,"+VG0-(VR0/(RR1*RR2))-(VR2+(VR0*(VG0+RR3)))-VR3");
    strcpy(eq[2].left,"+VG0-VR0-VR1");
    strcpy(eq[3].left,"+VG0-VR0-VR2-VR3");
    strcpy(eq[4].left,"+VR1-VR2-VR3");
    printf("\nLe equazioni generate sono le seguenti:\n");
    for(int i=0;i<n;i++){
        if(eq[i].left!=NULL && eq[i].right!=NULL){
            printf("eq n. %d ---- %s = %s ---- N inc: %d\n",i,eq[i].left,eq[i].right,eq[i].nInc);
            printf("Inc:\t");
            for(int j=0;j<eq[i].nInc;j++)   if(eq[i].inc[j]!=NULL)  printf("%s ",eq[i].inc[j]);
            printf("\n");
        }
    }
    //printf("Finito la scrittura delle equazioni\n");
    for(int i=0;i<n;i++){
        //printf("Ciclo n.%d\n",i);
        eq[i].startInc=0;
        eq[i].nInc=nInc(eq[i].left,elems,n,eq,i);
        eq[i].startInc=eq[i].nInc;
        eq[i].markInc=calloc(eq[i].startInc,sizeof(int));
        eq[i].inc=malloc(eq[i].startInc*sizeof(char*));
        for(int j=0;j<eq[i].startInc;j++){
            eq[i].inc[j]=malloc(NAME);
            strcpy(eq[i].inc[j],"");
        }
    }
    //printf("Fine cicli\n");
    strcpy(eq[0].inc[0],"IR0");
    strcpy(eq[0].inc[1],"IR1");
    strcpy(eq[0].inc[2],"IR2");
    strcpy(eq[1].inc[0],"IR2");
    strcpy(eq[1].inc[1],"IR3");
    strcpy(eq[2].inc[0],"VR0");
    strcpy(eq[2].inc[1],"VR1");
    strcpy(eq[3].inc[0],"VR0");
    strcpy(eq[3].inc[1],"VR2");
    strcpy(eq[3].inc[2],"VR3");
    strcpy(eq[4].inc[0],"VR1");
    strcpy(eq[4].inc[1],"VR2");
    strcpy(eq[4].inc[2],"VR3");
    printCircuit(elems,n);
    //printf("Finito stampa circuito\n");
    resolve(elems,eq,n);
    //resolveEq(eq,elems,0,1,n);
    //portaIncADestra(eq,2,0);
    //resolveOneInc(eq,2,"IR1",elems,3);
    //resolveEq(eq,elems,2,3,n);
    //sost(eq[2].left,"IR1","-IR3+(IR2*(RR3/RR2))-(RR2/(RR3*RR1))",elems[2]);
    printf("\n\nRISULTATO:\n");
    printCircuit(elems,n);
    for(int i=0;i<n;i++){
        free(eq[i].markInc);
        for(int j=0;j<eq[i].startInc;j++) if(eq[i].inc[j]!=NULL)    free(eq[i].inc[j]);
        free(eq[i].inc);
    }
    free(eq);
    free(elems);
    exit(0);
}
    
