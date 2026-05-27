#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20

char prod[MAX][MAX];
char first[MAX][MAX];
char follow[MAX][MAX];
char table[MAX][MAX][MAX];
char nt[MAX];
char terms[MAX];
int ntCount = 0;
int termCount = 0;
int n;

int ntIdx(char c) {
    for (int i = 0; i < ntCount; i++)
        if (nt[i] == c) return i;
    return -1;
}

int termIdx(char c) {
    for (int i = 0; i < termCount; i++)
        if (terms[i] == c) return i;
    return -1;
}

void getNonTerminals() {
    for (int i = 0; i < n; i++) {
        char c = prod[i][0];
        if (ntIdx(c) == -1)
            nt[ntCount++] = c;
    }
}

void getTerminals() {
    for (int i = 0; i < n; i++) {
        for (int j = 2; prod[i][j]; j++) {
            char c = prod[i][j];
            if (!isupper(c) && c != '#') {
                int found = 0;
                for (int k = 0; k < termCount; k++)
                    if (terms[k] == c) { found = 1; break; }
                if (!found) terms[termCount++] = c;
            }
        }
    }
    /* Add $ at the end */
    int found = 0;
    for (int k = 0; k < termCount; k++)
        if (terms[k] == '$') { found = 1; break; }
    if (!found) terms[termCount++] = '$';
}

void buildTable() {
    for (int i = 0; i < ntCount; i++)
        for (int j = 0; j < termCount; j++)
            table[i][j][0] = '\0';

    for (int i = 0; i < ntCount; i++) {
        char *rhs_first = first[i];

        /* For each terminal in FIRST(A) (excluding #), fill M[A, a] */
        for (int j = 0; rhs_first[j]; j++) {
            char a = rhs_first[j];
            if (a == '#') continue;
            int ti = termIdx(a);
            if (ti == -1) continue;

            /* Find the production(s) for this NT that derive 'a' in FIRST */
            for (int k = 0; k < n; k++) {
                if (prod[k][0] != nt[i]) continue;
                char *rhs = prod[k] + 2;

                /* Check if 'a' is in FIRST of this specific production's RHS */
                int inFirst = 0;
                int allEps = 1;
                for (int m = 0; rhs[m] && allEps; m++) {
                    char X = rhs[m];
                    allEps = 0;
                    if (isupper(X)) {
                        int Xi = ntIdx(X);
                        if (Xi == -1) { allEps = 0; break; }
                        int hasEps = 0;
                        for (int p = 0; first[Xi][p]; p++) {
                            if (first[Xi][p] == a) inFirst = 1;
                            if (first[Xi][p] == '#') hasEps = 1;
                        }
                        if (hasEps) allEps = 1;
                    } else {
                        if (X == '#') { allEps = 1; }
                        else { if (X == a) inFirst = 1; }
                    }
                }

                if (inFirst) {
                    if (table[i][ti][0] == '\0')
                        strcpy(table[i][ti], rhs);
                    else {
                        strcat(table[i][ti], "/");
                        strcat(table[i][ti], rhs);
                    }
                }
            }
        }

        /* If # is in FIRST(A), for each b in FOLLOW(A), fill M[A, b] with epsilon production */
        int hasEps = 0;
        for (int j = 0; rhs_first[j]; j++)
            if (rhs_first[j] == '#') { hasEps = 1; break; }

        if (hasEps) {
            for (int j = 0; follow[i][j]; j++) {
                char b = follow[i][j];
                int ti = termIdx(b);
                if (ti == -1) continue;

                /* Find the epsilon production for this NT */
                for (int k = 0; k < n; k++) {
                    if (prod[k][0] != nt[i]) continue;
                    if (strcmp(prod[k] + 2, "#") == 0) {
                        if (table[i][ti][0] == '\0')
                            strcpy(table[i][ti], "#");
                        else {
                            strcat(table[i][ti], "/#");
                        }
                    }
                }
            }
        }
    }
}

void printTable() {
    printf("\n--- LL(1) Parsing Table ---\n\n");

    /* Column width */
    int cw = 14;

    /* Header row */
    printf("%-8s", "");
    for (int j = 0; j < termCount; j++)
        printf("| %-*c", cw, terms[j]);
    printf("|\n");

    /* Divider */
    int total = 8 + termCount * (cw + 2) + 1;
    for (int k = 0; k < total; k++) printf("-");
    printf("\n");

    /* Table rows */
    for (int i = 0; i < ntCount; i++) {
        printf("%-8c", nt[i]);
        for (int j = 0; j < termCount; j++) {
            if (table[i][j][0] == '\0') {
                printf("| %-*s", cw, "");
            } else {
                char cell[MAX + 6];
                snprintf(cell, sizeof(cell), "%c->%s", nt[i], table[i][j]);
                printf("| %-*s", cw, cell);
            }
        }
        printf("|\n");
    }
}

int main() {
    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions (format A=rhs, use # for epsilon):\n");
    for (int i = 0; i < n; i++)
        scanf("%s", prod[i]);

    getNonTerminals();
    getTerminals();

    printf("\nNon-terminals found: ");
    for (int i = 0; i < ntCount; i++) printf("%c ", nt[i]);
    printf("\n");

    printf("Terminals found: ");
    for (int i = 0; i < termCount; i++) printf("%c ", terms[i]);
    printf("\n");

    /* User enters FIRST sets */
    printf("\nEnter FIRST sets (use # for epsilon, no spaces):\n");
    for (int i = 0; i < ntCount; i++) {
        printf("FIRST(%c) = ", nt[i]);
        scanf("%s", first[i]);
    }

    /* User enters FOLLOW sets */
    printf("\nEnter FOLLOW sets (include $, no spaces):\n");
    for (int i = 0; i < ntCount; i++) {
        printf("FOLLOW(%c) = ", nt[i]);
        scanf("%s", follow[i]);
    }

    buildTable();
    printTable();

    return 0;
}