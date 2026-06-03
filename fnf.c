#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#define MAX 100

char prod[MAX][MAX];
int n;
char first[MAX][MAX];
char follow[MAX][MAX];
char nt[MAX];
int ntCount = 0;

int ntIdx(char c){
    for(int i=0;i<ntCount;i++){
        if(nt[i] == c){
            return i;
        }
    }
    return -1;
}

void getNonTerminals(){
    for(int i=0;i<n;i++){
        if(ntIdx(prod[i][0]) == -1){
            nt[ntCount++] = prod[i][0];
        }
    }
}

int contains(char set[],char c){
    for(int i=0;set[i];i++){
        if(set[i] == c){
            return 1;
        }
    }
    return 0;
}

int add(char set[],char c){
    if(!contains(set,c)){
        int len = strlen(set);
        set[len] = c;
        set[len+1] = '\0';
        return 1;
    }
    return 0;
}

void computeFirst(){
    for(int i=0;i<n;i++){
        char A = prod[i][0];
        char B = prod[i][3];

        if(!isupper(B)){
            int Aidx = ntIdx(A);
            add(first[Aidx],B);
        }
    }
    int changed = 1;
    while(changed){
        changed = 0;
        for(int i=0;i<n;i++){
            char A = prod[i][0];
            int Aidx = ntIdx(A);

            char *rhs = prod[i]+3;
            int hasEps = 1;
            for(int m=0;rhs[m] && hasEps;m++){
                hasEps = 0;
                char X = rhs[m];
                int Xidx = ntIdx(X);
                if(isupper(X)){
                    for(int l=0;first[Xidx][l];l++){
                        if(first[Xidx][l] == '#'){
                            hasEps = 1;
                        }else{
                            if(add(first[Aidx],first[Xidx][l])){
                                changed = 1;
                            }
                        }
                    }
                }else{
                    if(X == '#'){
                        hasEps = 1;
                    }else{
                        if(add(first[Aidx],X)){
                            changed = 1;
                        }
                    }
                }
            }
            if(hasEps){
                if(add(first[Aidx],'#')){
                    changed = 1;
                }
            }
        }
    }
}

void printFirst(){
    for(int i=0;i<ntCount;i++){
        printf("first(%c) = { ",nt[i]);
        for(int j=0;first[i][j];j++){
            printf("%c ",first[i][j]);
        }
        printf("}\n");
    }
}

void computeFollow(){
    add(follow[ntIdx(prod[0][0])],'$');
    int changed = 1;
    while(changed){
        changed = 0;
        for(int i=0;i<n;i++){
            char A = prod[i][0];
            int Aidx = ntIdx(A);

            for(int j=3;prod[i][j];j++){
                char X = prod[i][j];
                if(isupper(X)){
                    int Xidx = ntIdx(X);
                    int hasEps = 1;
                    int k = j+1;
                    while(prod[i][k] != '\0' && hasEps){
                        hasEps = 0;
                        char B = prod[i][k];
                        if(isupper(B)){
                            int Bidx = ntIdx(B);
                            for(int l=0;first[Bidx][l];l++){
                                if(first[Bidx][l] == '#'){
                                    hasEps = 1;
                                }else{
                                    if(add(follow[Xidx],first[Bidx][l])){
                                        changed = 1;
                                    }
                                }
                            }
                        }else{
                            if(B == '#'){
                                hasEps = 1;
                            }else{
                                if(add(follow[Xidx],B)){
                                    changed = 1;
                                }
                            }
                        }
                        k++;
                    }
                    if(hasEps){
                        for(int f=0;follow[Aidx][f];f++){
                            if(add(follow[Xidx],follow[Aidx][f])){
                                changed = 1;
                            }
                        }
                    }
                }
            }
        }
    }
}

void printFollow(){
    for(int i=0;i<ntCount;i++){
        printf("follow(%c) = { ",nt[i]);
        for(int j=0;follow[i][j];j++){
            printf("%c ",follow[i][j]);
        }
        printf("}\n");
    }
}

int main(){
    printf("Enter number of productions: ");
    scanf("%d",&n);

    for(int i=0;i<n;i++){
        scanf("%s",prod[i]);
    }

    getNonTerminals();

    computeFirst();
    printFirst();

    computeFollow();
    printFollow();
    return 0;
}