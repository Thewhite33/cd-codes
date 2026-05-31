#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX 50

int n;
char prod[MAX][MAX];

int main(){
    printf("Enter number of productions: ");
    scanf("%d",&n);

    for(int i=0;i<n;i++){
        scanf("%s",prod[i]);
    }

    char done[26] = "";
    for(int i=0;i<n;i++){
        char A = prod[i][0];
        if(strchr(done,A)){
            continue;;
        }
        int len = strlen(done);
        done[len] = A;
        done[len+1] = '\0';
        char alpha[MAX][MAX],beta[MAX][MAX];
        int ac=0,bc=0;
        for(int j=0;j<n;j++){
            if(prod[j][0] != A) continue;
            char *rhs = prod[j]+2;

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
                continue;
            }

            // A -> beta A'
            printf("%c -> ",A);
            if(bc == 0){
                printf("%c'",A);
            }
            for(int j=0;j<bc;j++){
                printf("%s%c'",beta[j],A);
                if(j != bc-1) printf(" | ");
            }
            printf("\n");
            // A' -> alpha A'
            printf("%c' -> ",A);
            for(int j=0;j<ac;j++){
                printf("%s%c'",alpha[j],A);
                if(j != ac-1) printf(" | ");
            }
            printf(" | # \n");
    }
    return 0;
}