#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#define MAX 100

char prod[MAX][MAX];
int n;
char first[MAX][MAX];
char follow[MAX][MAX];
char table[MAX][MAX][MAX];
char nt[MAX];
char start;
char term[MAX];
int termCount = 0;
int ntCount = 0;

int ntIdx(char c){
    for(int i=0;i<ntCount;i++){
        if(nt[i] == c){
            return i;
        }
    }
    return -1;
}

int tIdx(char c){
    for(int i=0;i<termCount;i++){
        if(term[i] == c){
            return i;
        }
    }
    return -1;
}

void getTerminals(){
    for(int i=0;i<n;i++){
        for(int j=3;prod[i][j];j++){
            if(!isupper(prod[i][j]) && prod[i][j] != '#'){
                if(tIdx(prod[i][j]) == -1){
                    term[termCount++] = prod[i][j];
                }
            }
        }
    }
    if(tIdx('$') == -1){
        term[termCount++] = '$';
    }
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

void tableInput(){
    printf("Enter table entries: \n");
    for(int i=0;i<ntCount;i++){
        for(int j=0;j<termCount;j++){
            printf("M[%c,%c] = ",nt[i],term[j]);
            char temp[MAX];
            scanf("%s",temp);
            if(strcmp(temp,"-") == 0){
                table[i][j][0] = '\0';
            }else{
                strcpy(table[i][j],temp);
            }
        }
    }
}

void buildTable(){
    for(int i=0;i<ntCount;i++){
        for(int j=0;j<termCount;j++){
            table[i][j][0] = '\0';
        }
    }

    for(int i=0;i<ntCount;i++){
        char A = nt[i];

        for(int j=0;j<n;j++){
            if(prod[j][0] != A) continue;

            char *rhs = prod[j]+3;
            int rhsCanbeEps = 1;
            for(int m=0;rhs[m] && rhsCanbeEps;m++){
                rhsCanbeEps = 0;
                char X = rhs[m];
                if(isupper(X)){
                    int Xidx = ntIdx(X);
                    int hasEps = 0;
                    for(int l=0;first[Xidx][l];l++){
                        if(first[Xidx][l] != '#'){
                            int ti = tIdx(first[Xidx][l]);
                            if(table[i][ti][0] == '\0'){
                                strcpy(table[i][ti],rhs);
                            }else{
                                strcat(table[i][ti],"/");
                                strcat(table[i][ti],rhs);
                            }
                        }
                        else{
                            hasEps = 1;
                        }
                    }
                    if(hasEps){
                        rhsCanbeEps = 1;
                    }
                }else{
                    if(X == '#'){
                        rhsCanbeEps = 1;
                    }else{
                        int ti = tIdx(X);
                        if(table[i][ti][0] == '\0'){
                            strcpy(table[i][ti],rhs);
                        }else{
                            strcat(table[i][ti],"/");
                            strcat(table[i][ti],rhs);
                        }
                    }
                }
            }
            if(rhsCanbeEps){
                for(int f=0;follow[i][f];f++){
                    char b = follow[i][f];
                    int ti = tIdx(b);
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
    printf("%-8s","");
    for(int i=0;i<termCount;i++){
        printf("%-8c",term[i]);
    }
    printf("\n");
    for(int i=0;i<ntCount;i++){
        printf("%-8c",nt[i]);
        for(int j=0;j<termCount;j++){
            if(table[i][j][0] == '\0'){
                printf("%-8c",'-');
            }else{
                printf("%c->%-8s",nt[i],table[i][j]);
            }
        }
        printf("\n");
    }
}

void takeFirst(){
    for(int i=0;i<ntCount;i++){
        printf("first(%c) = ",nt[i]);
        scanf("%s",first[i]);
    }
}

void takeFollow(){
    for(int i=0;i<ntCount;i++){
        printf("follow(%c) = ",nt[i]);
        scanf("%s",follow[i]);
    }
}

void printStack(char stack[],int top){
    for(int i=top;i>=0;i--){
        printf("%c",stack[i]);
    }
}

void predictiveParse(){
    char input[MAX];
    printf("Enter string to parse: ");
    scanf("%s",input);
    strcat(input,"$");
    int ip = 0;
    char stack[MAX];
    int top = -1;
    stack[++top] = '$';
    stack[++top] = start;

    printf("%-10s%-10s%-10s\n","STACK","INPUT","ACTION");

    while(top>=0){
        char stackTop = stack[top];
        char currentSym = input[ip];

        printStack(stack,top);
        printf("\t\t");

        printf("%s",input+ip);
        printf("\t\t");

        if(stackTop == '$' && currentSym == '$'){
            printf("ACCEPT\n");
            break;
        }
        if(!isupper(stackTop)){
            if(stackTop == currentSym){
                printf("MATCH\n");
                top--;
                ip++;
            }else{
                printf("ERROR\n");
                break;
            }
        }else{
            int nti = ntIdx(stackTop);
            int ti = tIdx(currentSym);

            if(nti == -1 || ti == -1){
                printf("ERROR\n");
                break;
            }
            if(table[nti][ti][0] == '\0'){
                printf("ERROR\n");
                break;
            }
            char temp[MAX];
            strcpy(temp,table[nti][ti]);
            printf("%c->%s\n",stackTop,temp);
            top--;
            if(strcmp(temp,"#") != 0){
                for(int i=strlen(temp)-1;i>=0;i--){
                    stack[++top] = temp[i];
                }
            }
        }
    }
}

int main(){
    printf("Enter number of productions: ");
    scanf("%d",&n);

    for(int i=0;i<n;i++){
        scanf("%s",prod[i]);
    }

    // First and follow
    // getNonTerminals();
    // computeFirst();
    // printFirst();
    // computeFollow();
    // printFollow();

    // LL(1) table
    // getNonTerminals();
    // getTerminals();
    // takeFirst();
    // takeFollow();
    // buildTable();
    // printTable();

    // Non recursive predictive parsing
    getNonTerminals();
    printf("Enter start symbol: ");
    scanf(" %c",&start);

    if(ntIdx(start) == -1){
        printf("Invalid Start Symbol!\n");
        return 0;
    }
    getTerminals();
    tableInput();   
    predictiveParse();

    return 0;
}