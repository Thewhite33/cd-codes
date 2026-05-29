#include<stdio.h>
#include<ctype.h>
#include<string.h>
#define MAX 100

int n;
char prod[MAX][MAX];
char first[MAX][MAX];
char follow[MAX][MAX];
char table[MAX][MAX][MAX];
char nt[MAX];
int ntCount = 0;
char terms[MAX];
int termCount = 0;

int ntIdx(char c){
    for(int i=0;nt[i];i++){
        if(nt[i] == c){
            return i;
        }
    }
    return -1;
}

int termIdx(char c){
    for(int i=0;terms[i];i++){
        if(terms[i] == c){
            return i;
        }
    }
    return -1;
}

void getNonTerminals(){
    for(int i=0;i<n;i++){
        char c = prod[i][0];
        if(ntIdx(c) == -1){
            nt[ntCount++] = c;
        }
    }
}

void getTerminals(){
    for(int i=0;i<n;i++){
        for(int j=2;prod[i][j];j++){
            char c = prod[i][j];
            if(!isupper(c) && c != '#'){
                int found = 0;
                for(int k=0;terms[k];k++){
                    if(terms[k] == c){
                        found = 1;
                        break;
                    }
                }
                if(!found) terms[termCount++] = c;
            }
        }
    }
    int found = 0;
    for(int k=0;terms[k];k++){
        if(terms[k] == '$'){
            found = 1;
            break;
        }
    }
    if(!found){
        terms[termCount++] = '$';
    }
}

void buildTable(){
    // Empty table
    for(int i=0;i<ntCount;i++){
        for(int j=0;j<termCount;j++){
            table[i][j][0] = '\0';
        }
    }

    // Reach to every non terminal
    for(int i=0;i<ntCount;i++){
        // pick each non terminal
        char A = nt[i];
        // go to each production
        for(int j=0;j<n;j++){
            // check if production non termnal matches
            if(prod[j][0] != A) continue;

            // take complete rhs
            char *rhs = prod[j] + 2;
            // to take first of rhs

            // assume
            int rhsCanBeEps = 1;

            // check every rhs symbol
            for(int m=0;rhs[m] && rhsCanBeEps;m++){
                char X = rhs[m]; 
                rhsCanBeEps = 0;
                
                if(isupper(X)){
                    int Xi = ntIdx(X);
                    int hasEps = 0;
                    for(int p=0;first[Xi][p];p++){
                        char firstSym = first[Xi][p];
                        // If first is not eps
                        if(firstSym != '#'){
                            int ti = termIdx(firstSym);
                            if(table[i][ti][0] == '\0'){
                                strcpy(table[i][ti],rhs);
                            }else{
                                strcat(table[i][ti],"/");
                                strcat(table[i][ti],rhs);
                            }
                        }
                        if(firstSym == '#'){
                            hasEps = 1;
                        }
                    }
                    if(hasEps){
                        rhsCanBeEps = 1;
                    }
                } else{
                    if(X == '#'){
                        rhsCanBeEps = 1;
                    }else{
                        int ti = termIdx(X);
                        if(table[i][ti][0] == '\0'){
                            strcpy(table[i][ti],rhs);
                        } else{
                            strcat(table[i][ti],"/");
                            strcat(table[i][ti],rhs);
                        }
                    }
                }
            }
            
            if(rhsCanBeEps){
                for(int f=0;follow[i][f];f++){
                    char b = follow[i][f];
                    int ti = termIdx(b);

                    if(table[i][ti][0] == '\0'){
                        strcpy(table[i][ti],rhs);
                    }else{
                        strcat(table[i][ti],"/");
                        strcat(table[i][ti],rhs);
                    }
                }
            }
        }
    }
}

void printTable(){
    printf("%-8s","NT");
    for(int i=0;i<termCount;i++){
        printf("%-12c",terms[i]);
    }
    printf("\n");

    for(int i=0;i<ntCount;i++){
        printf("%-8c",nt[i]);
        for(int j=0;j<termCount;j++){
            if(table[i][j][0] == '\0'){
                printf("%-14s","-");
            }else{
                printf("%c->%-11s",nt[i],table[i][j]);
            }
        }
        printf("\n");
    }
}

int main(){
    printf("Enter number of productions: ");
    scanf("%d",&n);

    for(int i=0;i<n;i++){
        scanf("%s",prod[i]);
    }

    getNonTerminals();
    getTerminals();

    printf("Enter first sets: \n");
    for(int i=0;i<ntCount;i++){
        printf("first(%c) = ",nt[i]);
        scanf("%s",first[i]);
    }

    printf("Enter follow sets: \n");
    for(int i=0;i<ntCount;i++){
        printf("follow(%c) = ",nt[i]);
        scanf("%s",follow[i]);
    }

    buildTable();
    printTable();
    return 0;
}