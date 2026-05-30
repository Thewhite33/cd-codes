#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#define MAX 100

char prod[MAX][MAX];
char first[MAX][MAX];
char follow[MAX][MAX];
char table[MAX][MAX][MAX];
char nt[MAX];
char terms[MAX];

int ntCount = 0;
int termCount = 0;
int n;

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

int ntIdx(char c){
    for(int i=0;nt[i];i++){
        if(nt[i] == c){
            return i;
        }
    }
    return -1;
}

int tIdx(char c){
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

void computeFirst(){
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
            char *rhs = prod[i]+2;
            int hasEps = 1;
            for(int m=0;rhs[m] && hasEps;m++){
                hasEps = 0;
                char X = rhs[m];
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

            for(int j=2;prod[i][j]!='\0';j++){
                char X = prod[i][j];
                if(isupper(X)){
                    int Xidx = ntIdx(X);
                    int k = j+1;
                    int hasEps = 1;
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
                        for(int l=0;follow[Aidx][l];l++){
                            if(add(follow[Xidx],follow[Aidx][l])){
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

// LL(1) table and stack trace functions 

void getTerminals(){
    for(int i=0;i<n;i++){
        for(int j=2;prod[i][j] != '\0';j++){
            if(!isupper(prod[i][j]) && prod[i][j] != '#'){
                if(tIdx(prod[i][j]) == -1){
                    terms[termCount++] = prod[i][j];
                }
            }
        }
    }
    if(tIdx('$') == -1){
        terms[termCount++] = '$';
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

            char *rhs = prod[j]+2;
            int rhsCanbeEps = 1;

            for(int m=0;rhs[m] && rhsCanbeEps;m++){
                char X = rhs[m];
                rhsCanbeEps = 0;

                if(isupper(X)){
                    int hasEps = 0;
                    int Xidx = ntIdx(X);
                    for(int k=0;first[Xidx][k];k++){
                        char firstSym = first[Xidx][k];
                        if(firstSym != '#'){
                            int ti = tIdx(firstSym);
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
                    int ti = tIdx(follow[i][f]);
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

void inputTable(){
    printf("Enter predictive parsing table entries: ");
    for(int i=0;i<ntCount;i++){
        for(int j=0;j<termCount;j++){
            char temp[50];
            printf("M[%c,%c] = ",nt[i],terms[j]);
            scanf("%s",temp);
            if(strcmp(temp,"-") == 0){
                table[i][j][0] = '\0';
            }else{
                strcpy(table[i][j],temp);
            }
        }
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

    char stack[MAX];
    int top = -1;
    stack[++top] = '$';
    stack[++top] = nt[0];
    int ip = 0;
    printf("\n\n%-15s%-15s%-15s\n","STACK","INPUT","ACTION");

    while(top>=0){
        char stackTop = stack[top];
        char currentSymbol = input[ip];
        printStack(stack,top);
        printf("\t\t");
        printf("%s",input+ip);
        printf("\t\t");
        if(stackTop == '$' && currentSymbol == '$'){
            printf("ACCEPT\n");
            break;
        }
        if(!isupper(stackTop)){
            if(stackTop == currentSymbol){
                printf("MATCH %c\n",currentSymbol);
                top--;
                ip++;
            }else{
                printf("ERROR\n");
                break;
            }
        }else{
            int nti = ntIdx(stackTop);
            int ti = tIdx(currentSymbol);
            if(ti == -1 || ti == -1){
                printf("ERROR\n");
                break;
            }
            if(table[nti][ti][0] == '\0'){
                printf("ERROR\n");
                break;
            }
            char production[MAX];
            strcpy(production,table[nti][ti]);
            printf("%c->%s\n",stackTop,production);
            top--;
            if(strcmp(production,"#") != 0){
                for(int i=strlen(production)-1;i>=0;i--){
                    stack[++top] = production[i];
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

    // Calculate first and follow
    // getNonTerminals();

    // computeFirst();
    // printFirst();

    // computeFollow();
    // printFollow();

    // Build LL(1) Table
    // getNonTerminals();
    // getTerminals();

    // takeFirst();
    // printFirst();
    // takeFollow();

    // buildTable();
    // printTable();

    // Non recursive predictive parsing
    getTerminals();
    getNonTerminals();
    inputTable();
    // printTable();
    predictiveParse();

    return 0;
}