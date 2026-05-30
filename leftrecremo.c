#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 100

char nt[MAX];
char rhs[MAX][MAX][MAX];
int counts[MAX];

void parse_input(char *input,int idx){
    int i=0,j=0,k=0;
    while(input[i] != '\0'){
        if(input[i] == '|'){
            rhs[idx][j][k] = '\0';
            j++;
            k = 0;
        }else{
            rhs[idx][j][k++] = input[i];
        }
        i++;
    }
    rhs[idx][j][k] = '\0';
    counts[idx] = j+1;
}

void substitue(int i,int j){
    char newrhs[MAX][MAX];
    int newcount = 0;

    for(int k=0;k<counts[i];k++){
        if(rhs[i][k][0] == nt[j]){
            for(int l=0;l<counts[j];l++){
                strcpy(newrhs[newcount],rhs[j][l]);
                strcat(newrhs[newcount],rhs[i][k]+1);
                newcount++;
            }
        }else{
            strcpy(newrhs[newcount++],rhs[i][k]);
        }
    }

    counts[i] = newcount;
    for(int k=0;k<newcount;k++){
        strcpy(rhs[i][k],newrhs[k]);
    }
}

void remove_direct(int i){
    char alpha[MAX][MAX], beta[MAX][MAX];
    int a_cnt = 0, b_cnt = 0;
    char non_terminal = nt[i];

    for(int j=0;j<counts[i];j++){
        if(rhs[i][j][0] == non_terminal){
            strcpy(alpha[a_cnt++],rhs[i][j]+1);
        }else{
            strcpy(beta[b_cnt++],rhs[i][j]);
        }
    }

    if(a_cnt == 0) return;

    printf("%c -> ",non_terminal);
    if(b_cnt == 0){
        printf("%c'",non_terminal);
    }else{
        for(int j=0;j<b_cnt;j++){
            printf("%s%c' ",beta[j],non_terminal);
            if(j<b_cnt-1) printf(" | ");
        }
    }

    printf("\n%c' -> ",non_terminal);
    for(int j=0;j<a_cnt;j++){
        printf("%s%c' | ",alpha[j],non_terminal);
    }
    printf("#\n");
}

int main(){
    char input[MAX];
    int n;
    printf("Enter number of productions: ");
    scanf("%d",&n);

    for(int i=0;i<n;i++){
        printf("Enter prod: ");
        scanf("%s",input);
        nt[i] = input[0];
        parse_input(input+3,i);
    }

    for(int i=0;i<n;i++){
        for(int j=0;j<i;j++){
            substitue(i,j);
        }

        // printf("%c -> ",nt[i]);
        // for(int j=0;j<counts[i];j++){
        //     printf("%s",rhs[i][j]);
        //     if(j < counts[i]-1)
        //         printf(" | ");
        // }
        // printf("\n");

        remove_direct(i);
    }
    return 0;
}