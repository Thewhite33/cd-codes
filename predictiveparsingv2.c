#include<stdio.h>
#include<string.h>
#include<ctype.h>

#define MAX 100

char prod[MAX][MAX];
char table[MAX][MAX][MAX];

char nt[MAX];
char terms[MAX];

int ntCount = 0;
int termCount = 0;

int n;

int ntIdx(char c){
    for(int i=0;i<ntCount;i++){
        if(nt[i] == c){
            return i;
        }
    }
    return -1;
}

int termIdx(char c){
    for(int i=0;i<termCount;i++){
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

                if(termIdx(c) == -1){

                    terms[termCount++] = c;
                }
            }
        }
    }

    /* Add $ */
    if(termIdx('$') == -1){

        terms[termCount++] = '$';
    }
}

/* Input parsing table manually */
void inputTable(){

    printf("\nEnter Parsing Table Entries:\n");
    printf("Use # for epsilon and - for empty\n\n");

    for(int i=0;i<ntCount;i++){

        for(int j=0;j<termCount;j++){

            char temp[MAX];

            printf("M[%c,%c] = ",nt[i],terms[j]);

            scanf("%s",temp);

            if(strcmp(temp,"-") == 0){

                table[i][j][0] = '\0';

            } else {

                strcpy(table[i][j],temp);
            }
        }
    }
}

/* Print parsing table */
void printTable(){

    printf("\n\n--- LL(1) Parsing Table ---\n\n");

    printf("%-10s","");

    for(int i=0;i<termCount;i++){

        printf("%-15c",terms[i]);
    }

    printf("\n");

    for(int i=0;i<ntCount;i++){

        printf("%-10c",nt[i]);

        for(int j=0;j<termCount;j++){

            if(table[i][j][0] == '\0'){

                printf("%-15s","-");

            } else {

                char cell[MAX];

                sprintf(cell,"%c->%s",nt[i],table[i][j]);

                printf("%-15s",cell);
                // printf("%c->%-11s",nt[i],table[i][j]);
            }
        }

        printf("\n");
    }
}

/* Print stack */
void printStack(char stack[],int top){

    for(int i=top;i>=0;i--){

        printf("%c",stack[i]);
    }
}

/* Predictive Parsing */
void predictiveParse(){
    char input[MAX];
    printf("\nEnter input string: ");
    scanf("%s",input);
    strcat(input,"$");
    char stack[MAX];
    int top = -1;
    /* Push $ */
    stack[++top] = '$';
    /* Push start symbol */
    stack[++top] = nt[0];
    int ip = 0;
    printf("\n\n%-25s %-25s %-25s\n",
           "STACK",
           "INPUT",
           "ACTION");
    printf("--------------------------------------------------------------------------\n");
    while(top >= 0){
        char stackTop = stack[top];
        char currentInput = input[ip];
        /* Print stack */
        printStack(stack,top);
        printf("\t\t");
        /* Print remaining input */
        printf("%s",input + ip);
        printf("\t\t");
        /*
            ACCEPT
        */
        if(stackTop == '$' && currentInput == '$'){
            printf("ACCEPT\n");
            break;
        }
        /*
            TERMINAL
        */
        if(!isupper(stackTop)){
            if(stackTop == currentInput){
                printf("Match %c\n",currentInput);
                top--;
                ip++;
            } else {
                printf("ERROR\n");
                break;
            }
        }
        /*
            NON-TERMINAL
        */
        else {
            int nti = ntIdx(stackTop);
            int ti = termIdx(currentInput);
            if(nti == -1 || ti == -1){
                printf("ERROR\n");
                break;
            }
            /*
                Empty table entry
            */
            if(table[nti][ti][0] == '\0'){
                printf("ERROR\n");
                break;
            }
            char production[MAX];
            strcpy(production,table[nti][ti]);
            printf("%c->%s\n",stackTop,production);
            /* Pop NT */
            top--;
            /*
                If epsilon,
                push nothing
            */
            if(strcmp(production,"#") != 0){
                /*
                    Push RHS in reverse
                */
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

    printf("\nEnter productions (Example: E=TR)\n\n");

    for(int i=0;i<n;i++){

        scanf("%s",prod[i]);
    }

    getNonTerminals();

    getTerminals();

    inputTable();

    printTable();

    predictiveParse();

    return 0;
}