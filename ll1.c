#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX 20

char prod[MAX][MAX];
char first[MAX][MAX];
char follow[MAX][MAX];
char table[MAX][MAX][MAX]; /* table[NT][terminal] = production string */
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
    /* Collect all terminal symbols (lowercase + #), then add $ */
    for (int i = 0; i < n; i++) {
        for (int j = 2; prod[i][j]; j++) {
            char c = prod[i][j];
            if (!isupper(c) && c != '#' && termIdx(c) == -1)
                terms[termCount++] = c;
        }
    }
    if (termIdx('$') == -1)
        terms[termCount++] = '$';
}

int contains(char set[], char c) {
    for (int i = 0; set[i]; i++)
        if (set[i] == c) return 1;
    return 0;
}

int addToSet(char set[], char ch) {
    if (!contains(set, ch)) {
        int len = strlen(set);
        set[len] = ch;
        set[len + 1] = '\0';
        return 1;
    }
    return 0;
}

void computeFirst() {
    /* Initial pass: productions starting with a terminal */
    for (int i = 0; i < n; i++) {
        char A = prod[i][0];
        char fs = prod[i][2];
        if (!isupper(fs))
            addToSet(first[ntIdx(A)], fs);
    }

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < n; i++) {
            char A = prod[i][0];
            int Ai = ntIdx(A);
            int k = 2;
            int hasEps = 1;

            while (prod[i][k] != '\0' && hasEps) {
                char X = prod[i][k];
                hasEps = 0;
                if (isupper(X)) {
                    int Xi = ntIdx(X);
                    for (int j = 0; first[Xi][j]; j++) {
                        if (first[Xi][j] == '#') hasEps = 1;
                        else if (addToSet(first[Ai], first[Xi][j])) changed = 1;
                    }
                } else {
                    if (X == '#') hasEps = 1;
                    else if (addToSet(first[Ai], X)) changed = 1;
                }
                k++;
            }
            if (hasEps)
                if (addToSet(first[Ai], '#')) changed = 1;
        }
    }
}

void computeFollow() {
    /* Start symbol gets $ */
    addToSet(follow[ntIdx(prod[0][0])], '$');

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < n; i++) {
            char A = prod[i][0];
            int Ai = ntIdx(A);

            for (int j = 2; prod[i][j] != '\0'; j++) {
                char B = prod[i][j];
                if (!isupper(B)) continue;
                int Bi = ntIdx(B);

                int k = j + 1;
                int hasEps = 1;

                while (prod[i][k] != '\0' && hasEps) {
                    char X = prod[i][k];
                    hasEps = 0;
                    if (isupper(X)) {
                        int Xi = ntIdx(X);
                        for (int l = 0; first[Xi][l]; l++) {
                            if (first[Xi][l] == '#') hasEps = 1;
                            else if (addToSet(follow[Bi], first[Xi][l])) changed = 1;
                        }
                    } else {
                        if (X != '#')
                            if (addToSet(follow[Bi], X)) changed = 1;
                    }
                    k++;
                }

                if (hasEps) {
                    for (int l = 0; follow[Ai][l]; l++)
                        if (addToSet(follow[Bi], follow[Ai][l])) changed = 1;
                }
            }
        }
    }
}

/*
 * firstOfString: compute FIRST of a production RHS string.
 * Writes result into 'out'. Returns 1 if epsilon is in the set.
 */
int firstOfString(char *rhs, char *out) {
    out[0] = '\0';
    int allEps = 1;

    for (int i = 0; rhs[i] && allEps; i++) {
        char X = rhs[i];
        allEps = 0;
        if (isupper(X)) {
            int Xi = ntIdx(X);
            for (int j = 0; first[Xi][j]; j++) {
                if (first[Xi][j] == '#') allEps = 1;
                else addToSet(out, first[Xi][j]);
            }
        } else {
            if (X == '#') allEps = 1;
            else addToSet(out, X);
        }
    }
    if (allEps) addToSet(out, '#');
    return allEps;
}

void buildTable() {
    /* Initialize table cells to empty strings */
    for (int i = 0; i < ntCount; i++)
        for (int j = 0; j < termCount; j++)
            table[i][j][0] = '\0';

    for (int i = 0; i < n; i++) {
        char A = prod[i][0];
        int Ai = ntIdx(A);
        char *rhs = prod[i] + 2;

        char fset[MAX];
        firstOfString(rhs, fset);

        for (int j = 0; fset[j]; j++) {
            char a = fset[j];
            if (a == '#') {
                /* Add production to M[A, b] for each b in FOLLOW(A) */
                for (int k = 0; follow[Ai][k]; k++) {
                    int ti = termIdx(follow[Ai][k]);
                    if (ti != -1) {
                        if (table[Ai][ti][0] == '\0')
                            strcpy(table[Ai][ti], rhs);
                        else {
                            /* Conflict: append with '/' separator */
                            strcat(table[Ai][ti], "/");
                            strcat(table[Ai][ti], rhs);
                        }
                    }
                }
            } else {
                int ti = termIdx(a);
                if (ti != -1) {
                    if (table[Ai][ti][0] == '\0')
                        strcpy(table[Ai][ti], rhs);
                    else {
                        strcat(table[Ai][ti], "/");
                        strcat(table[Ai][ti], rhs);
                    }
                }
            }
        }
    }
}

void printFirst() {
    printf("\n--- FIRST Sets ---\n");
    for (int i = 0; i < ntCount; i++) {
        printf("FIRST(%c) = { ", nt[i]);
        for (int j = 0; first[i][j]; j++) {
            char c = first[i][j];
            printf("%c ", (c == '#') ? 'e' : c);
        }
        printf("}\n");
    }
}

void printFollow() {
    printf("\n--- FOLLOW Sets ---\n");
    for (int i = 0; i < ntCount; i++) {
        printf("FOLLOW(%c) = { ", nt[i]);
        for (int j = 0; follow[i][j]; j++)
            printf("%c ", follow[i][j]);
        printf("}\n");
    }
}

void printTable() {
    printf("\n--- LL(1) Parsing Table ---\n");

    /* Header */
    printf("%-6s", "");
    for (int j = 0; j < termCount; j++)
        printf("| %-12c", terms[j]);
    printf("|\n");

    /* Divider */
    int width = 6 + termCount * 14 + 1;
    for (int k = 0; k < width; k++) printf("-");
    printf("\n");

    /* Rows */
    for (int i = 0; i < ntCount; i++) {
        printf("%-6c", nt[i]);
        for (int j = 0; j < termCount; j++) {
            if (table[i][j][0] == '\0')
                printf("| %-12s", " ");
            else {
                char cell[MAX + 4];
                snprintf(cell, sizeof(cell), "%c->%s", nt[i], table[i][j]);
                printf("| %-12s", cell);
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
    computeFirst();
    computeFollow();
    buildTable();

    printFirst();
    printFollow();
    printTable();

    /* Check for conflicts */
    int conflict = 0;
    for (int i = 0; i < ntCount; i++)
        for (int j = 0; j < termCount; j++)
            if (strchr(table[i][j], '/')) {
                if (!conflict) printf("\n[!] Conflicts detected (not LL(1)):\n");
                printf("    M[%c, %c] = %c->%s\n", nt[i], terms[j], nt[i], table[i][j]);
                conflict = 1;
            }
    if (!conflict)
        printf("\nGrammar is LL(1) no conflicts.\n");

    return 0;
}