#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#define MAX 100

char prod[MAX][MAX];
char first[MAX][MAX];
char follow[MAX][MAX];

int ntCount = 0;
int n;

char nt[MAX];

int ntIdx(char c){
    for(int i=0;i<ntCount;i++){
        if(nt[i] == c){
            return i;
        }
    }
    return -1;
}

void getTerminals(){
    for(int i=0;i<n;i++){
        char c = prod[i][0];
        if(ntIdx(c) == -1){
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

int add(char set[],char c){
    if(!contains(set,c)){
        int l = strlen(set);
        set[l] = c;
        set[l+1] = '\0';
        return 1;
    }
    return 0;
}

void calculateFirst(){
    for(int i=0;i<n;i++){
        char A = prod[i][0];
        char X = prod[i][2];

        if(!isupper(X)){
            int Aidx = ntIdx(A);
            add(first[Aidx],X);
        }
    }
    int changed = 1;
    while(changed){
        changed = 0;
        for(int i=0;i<n;i++){
            char A = prod[i][0];
            int Aidx = ntIdx(A);
            int k = 2;
            int hasEps = 1;

            while(prod[i][k] != '\0' && hasEps){
                char X = prod[i][k];
                hasEps = 0;
                if(isupper(X)){
                    int Xidx = ntIdx(X);
                    for(int j=0;first[Xidx][j];j++){
                        if(first[Xidx][j] == '#'){
                            hasEps = 1;
                        }else{
                            if(add(first[Aidx],first[Xidx][j])){
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
                k++;
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

void calculateFollow(){
    add(follow[ntIdx(prod[0][0])],'$');
    int followChanged = 1;
    while(followChanged){
        followChanged = 0;
        for(int i=0;i<n;i++){
            char A = prod[i][0];
            int Aidx = ntIdx(A);

            for(int j=2;prod[i][j]!='\0';j++){
                char B = prod[i][j];
                int Bidx = ntIdx(B);
                if(isupper(B)){
                    int k = j+1;
                    int hasEps = 1;
                    while(prod[i][k] != '\0' && hasEps){
                        char X = prod[i][k];
                        hasEps = 0;
                        if(isupper(X)){
                            int Xidx = ntIdx(X);
                            for(int l=0;first[Xidx][l];l++){
                                if(first[Xidx][l] == '#'){
                                    hasEps = 1;
                                }else{
                                    if(add(follow[Bidx],first[Xidx][l])){
                                        followChanged = 1;
                                    }
                                }
                            }
                        }else{
                            if(X == '#'){
                                hasEps = 1;
                            }else{
                                if(add(follow[Bidx],X)){
                                    followChanged = 1;
                                }
                            }
                        }
                        k++;
                    }
                    if(hasEps){
                        for(int l=0;follow[Aidx][l];l++){
                            if(add(follow[Bidx],follow[Aidx][l])){
                                followChanged = 1;
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

    getTerminals();

    calculateFirst();
    printFirst();

    calculateFollow();
    printFollow();

    return 0;
}