#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROD 20
#define MAX_LEN  100

/* Grammar storage */
char lhs[MAX_PROD][3];        /* Left-hand side non-terminal (e.g. "A" or "A'") */
char rhs[MAX_PROD][MAX_LEN];  /* Right-hand side alternatives (|)  */
int  count = 0;               /* Total number of grammar rules      */

void read_grammar(int n) {
    printf("Enter productions (e.g.  A=aA|b  or  E=ET|a):\n");
    for (int i = 0; i < n; i++) {
        char line[MAX_LEN];
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';

        lhs[count][0] = line[0];          /* First char is NT     */
        lhs[count][1] = '\0';
        strcpy(rhs[count], strchr(line,'=') + 1);  /* After '='   */
        count++;
    }
}

void remove_left_recursion() {
    for (int i = 0; i < count; i++) {
        char A = lhs[i][0];

        /* ---- Collect alpha and beta lists ---- */
        char alpha[MAX_PROD][MAX_LEN];  /* left-recursive tails  */
        char beta [MAX_PROD][MAX_LEN];  /* normal alternatives   */
        int  na = 0, nb = 0;

        /* Tokenise rhs by '|' */
        char tmp[MAX_LEN];
        strcpy(tmp, rhs[i]);
        char *tok = strtok(tmp, "|");
        while (tok) {
            if (tok[0] == A)                         /* starts with A? */
                strcpy(alpha[na++], tok + 1);        /* save tail      */
            else
                strcpy(beta[nb++], tok);
            tok = strtok(NULL, "|");
        }

        if (na == 0) continue;   /* No left recursion found → skip */

        /* ---- Build new A' name (e.g. A → A') ---- */
        char Ap[3] = { A, '\'', '\0' };   /* "A'" */

        /* ---- Rewrite A:  beta1 A' | beta2 A' | ... ---- */
        char newA[MAX_LEN] = "";
        for (int b = 0; b < nb; b++) {
            if (b) strcat(newA, "|");
            strcat(newA, beta[b]);
            strcat(newA, Ap);           /* append A' after each beta */
        }
        if (nb == 0) strcpy(newA, Ap); /* A had ONLY recursive alts */
        strcpy(rhs[i], newA);

        /* ---- Append new rule:  A' → alpha1 A' | alpha2 A' | # ---- */
        lhs[count][0] = Ap[0];         /* store 'A'  */
        lhs[count][1] = Ap[1];         /* store '\'' */
        lhs[count][2] = '\0';

        char newAp[MAX_LEN] = "";
        for (int a = 0; a < na; a++) {
            if (a) strcat(newAp, "|");
            strcat(newAp, alpha[a]);
            strcat(newAp, Ap);          /* alpha1 A', alpha2 A', ...  */
        }
        strcat(newAp, "|#");            /* add  ε  alternative        */

        strcpy(rhs[count], newAp);
        count++;
    }
}

void print_grammar() {
    for (int i = 0; i < count; i++)
        printf("  %s = %s\n", lhs[i], rhs[i]);
}

int main() {
    int n;
    printf("Enter number of productions: ");
    scanf("%d", &n);
    getchar();

    read_grammar(n);

    printf("\n--- Original Grammar ---\n");
    print_grammar();

    remove_left_recursion();

    printf("\n--- After Left Recursion Removal ---\n");
    print_grammar();

    return 0;
}