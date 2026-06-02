#include<stdio.h>
#include<string.h>
#define MAX 100

char prod[MAX][MAX];
int n;
int ntCount = 0;

void printGrammer(){
    for(int i=0;i<n;i++){
        printf("%s\n",prod[i]);
    }
}

int isNonTerminal(char c){
    return c>='A' && c<='Z';
}

void substitute(char Ai,char Aj){
    char newProd[MAX][MAX];
    int newCount = 0;

    for(int i=0;i<n;i++){
        if(prod[i][0] != Ai){
            strcpy(newProd[newCount++],prod[i]);
            continue;
        }
        char *rhs = prod[i]+2;
        if(rhs[0] == Aj){
            char rem[MAX];
            strcpy(rem,rhs+1);
            for(int j=0;j<n;j++){
                if(prod[j][0] == Aj){
                    char temp[MAX];
                    strcpy(temp,prod[j]+2);
                    strcat(temp,rem);
                    sprintf(newProd[newCount++],"%c=%s",Ai,temp);
                }
            }
        }else{ 
            strcpy(newProd[newCount++],prod[i]);
        }
    }
    n = newCount;
    for(int i=0;i<n;i++){
        strcpy(prod[i],newProd[i]);
    }
}

void removeDirect(char A){
    char alpha[MAX][MAX];
    char beta[MAX][MAX];
    int ac=0,bc=0;

    for(int i=0;i<n;i++){
        if(prod[i][0] != A){
            continue;
        }
        char *rhs = prod[i]+2;
        if(rhs[0] == A){
            strcpy(alpha[ac++],rhs+1);
        }else{
            strcpy(beta[bc++],rhs);
        }
    }
    if(ac == 0){
        printf("%c -> ",A);
        for(int j=0;j<bc;j++){
            printf("%s",beta[j]);
            if(j!=bc-1) printf(" | ");
        }
        printf("\n");
    }else{
    // printf("Removing Left Recursion for %c\n",A);
        printf("%c -> ",A);
        if(bc == 0){
            printf("%c'",A);
        }else{
            for(int i=0;i<bc;i++){
                printf("%s%c'",beta[i],A);
                if(i != bc-1){
                    printf(" | ");
                }
            }
            printf("\n");
        }
        printf("%c' -> ",A);
        for(int i=0;i<ac;i++){
            printf("%s%c'",alpha[i],A);
            if(i != bc-1){
                printf(" | ");
            }
        }
        printf(" # \n");
    }
}

int main(){
    printf("Enter number of productions: ");
    scanf("%d",&n);

    for(int i=0;i<n;i++){
        scanf("%s",prod[i]);
    }

    char nt[26];
    for(int i=0;i<n;i++){
        int found = 0;
        for(int j=0;j<ntCount;j++){
            if(nt[j] == prod[i][0]){
                found = 1;
                break;
            }
        }
        if(!found){
            nt[ntCount++] = prod[i][0];
        }
    }

    for(int i=0;i<ntCount;i++){
        char Ai = nt[i];
        for(int j=0;j<i;j++){
            char Aj = nt[j];
            substitute(Ai,Aj);
        }
        removeDirect(Ai);
    }
    // printGrammer();
    return 0;
}