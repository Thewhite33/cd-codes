#include<stdio.h>
#include<ctype.h>
#include<string.h>

#define MAX 20

char prod[MAX][MAX];
char first[MAX][MAX];
char follow[MAX][MAX];
char nt[MAX];
int ntCount = 0;
int n;

int idx(char c){
    for(int i=0;i<ntCount;i++){
        if(nt[i] == c){
            return i;
        }
    }
    return -1;
}

void getNonTerminals(){
    for(int i=0;i<n;i++){
        char c = prod[i][0];
        if(idx(c) == -1){
            nt[ntCount++] = c;
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

int add(char set[],char ch){
    if(!contains(set,ch)){
        int len = strlen(set);
        set[len] = ch;
        set[len+1] = '\0';

        return 1;
    }
    return 0;
}

int main(){
    printf("Enter number of productions: ");
    scanf("%d",&n);

    printf("Enter productions: ");
    for(int i=0;i<n;i++){
        scanf("%s",prod[i]);
    }

    getNonTerminals();

    //Calculating first - initial pass
    for(int i=0;i<n;i++){
        char A = prod[i][0];
        char firstSymbol = prod[i][2];

        if(!isupper(firstSymbol)){
            int Aidx = idx(A);
            add(first[Aidx],firstSymbol);
        }
    }

    int changed = 1;
    while(changed){
        changed = 0;
        for(int i=0;i<n;i++){
            char A = prod[i][0];
            int Aidx = idx(A);
            int k = 2;
            int hasEpsilon = 1;

            while(prod[i][k] != '\0' && hasEpsilon){
                char X = prod[i][k];
                hasEpsilon = 0;
                if(isupper(X)){
                    int Xidx = idx(X);
                    for(int j=0;first[Xidx][j];j++){
                        if(first[Xidx][j] == '#'){
                            hasEpsilon = 1;
                        } else{
                            if(add(first[Aidx],first[Xidx][j])){
                                changed = 1;
                            }
                        }
                    }
                } else{
                    if(X == '#'){
                        hasEpsilon = 1;
                    } else{
                        if(add(first[Aidx],X)){
                            changed = 1;
                        }
                    }
                }
                k++;
            }
            if(hasEpsilon){
                if(add(first[Aidx],'#')){
                    changed = 1;
                }
            }
        }
    }

    //Follow calcultions
    add(follow[idx(prod[0][0])],'$');
    int followChanged = 1;
    while(followChanged){
        followChanged = 0;
        for(int i=0;i<n;i++){
            char A = prod[i][0];
            int Aidx = idx(A);

            for(int j=2;prod[i][j] != '\0';j++){
                char B = prod[i][j];
                if(isupper(B)){
                    int Bidx = idx(B);
                    int k = j+1;
                    int hasEpsilon = 1;
                    while(prod[i][k] != '\0' && hasEpsilon){
                        char X = prod[i][k];
                        hasEpsilon = 0;

                        if(isupper(X)){
                            int Xidx = idx(X);
                            for(int l=0;first[Xidx][l];l++){
                                if(first[Xidx][l] == '#'){
                                    hasEpsilon = 1;
                                } else{
                                    if(add(follow[Bidx],first[Xidx][l])){
                                        followChanged = 1;
                                    }
                                }
                            }
                        }else{
                            if(X != '#'){
                                if(add(follow[Bidx], X)){
                                    followChanged = 1;
                                }
                            }
                        }
                        k++;
                    }
                    if(hasEpsilon){
                        for(int l=0; follow[Aidx][l]; l++){
                            if(add(follow[Bidx], follow[Aidx][l])){
                                followChanged = 1;
                            }
                        }
                    }
                }
            }
        }
    }

    //Print first
    for(int i=0;i<ntCount;i++){
        printf("first(%c)={",nt[i]);
        for(int j=0;first[i][j];j++){
            printf(" %c ",first[i][j]);
        }
        printf("}\n");
    }

    for(int i=0;i<ntCount;i++){
        printf("follow(%c)={",nt[i]);
        for(int j=0;follow[i][j];j++){
            printf(" %c ",follow[i][j]);
        }
        printf("}\n");
    }
}