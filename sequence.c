#include <stdio.h>
#include <stdlib.h>
#define N 20
char sequenza[N];
char word[N];
int sequenceLenght;
int wordLenght;
int startSequence;
int lenght;

int inSequence(char x)
{
    for(int i=0;i<sequenceLenght;i++){
        if(x==sequenza[i]) return 1;
    }
    return 0;
}

int isSequence(int start,int end)
{
    int* mark=calloc(sequenceLenght,sizeof(int));
    int isSeq=1;
    for(int i=start;i<end;i++){
        for(int k=0;k<sequenceLenght;k++) {
            if(sequenza[k]==word[i] && mark[k]==0){
                mark[k]=1;
                break;
            }
        }
    }
    for(int i=0;i<sequenceLenght;i++){
        if(mark[i]==0){
            isSeq=0;
            break;
        }
    }
    free(mark);
    return isSeq;
}

void findSeq()
{
    lenght=wordLenght;
    if(sequenceLenght>wordLenght) return;
    for(int i=0;i<wordLenght;i++){
        if(inSequence(word[i]))
        {
            for(int j=sequenceLenght;j<wordLenght;j++)
            {
                if(i+j>wordLenght) break;
                if(isSequence(i,i+j)){
                    if(j<lenght)
                    {
                        startSequence=i;
                        lenght=j;
                    }
                    break;
                }
            }
        }
        if(lenght==sequenceLenght) break;
    }
    printf("La minima sequenza nella parola e': ");
    for(int i=startSequence;i<(startSequence+lenght);i++) printf("%c",word[i]);
}

int main()
{
    printf("Scrivi la parola: ");
    scanf("%s",word);
    for(int i=0;i<N;i++){
        if(word[i]==NULL){
            wordLenght=i;
            break;
        }
    }
    printf("Scrivi la sequenza da individuare: ");
    scanf("%s",sequenza);
     for(int i=0;i<N;i++){
        if(sequenza[i]==NULL){
            sequenceLenght=i;
            break;
        }
    }
    findSeq();
}
