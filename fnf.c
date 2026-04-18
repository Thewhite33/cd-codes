#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20

char prod[MAX][MAX];
char nt[MAX];          // unique non-terminals
char first[MAX][MAX];
char follow[MAX][MAX];

int n, ntCount = 0;

/* check existence */
int contains(char set[], char ch) {
    for (int i = 0; set[i]; i++)
        if (set[i] == ch) return 1;
    return 0;
}

/* add symbol */
int add(char set[], char ch) {
    if (!contains(set, ch)) {
        int l = strlen(set);
        set[l] = ch;
        set[l+1] = '\0';
        return 1;
    }
    return 0;
}

/* get index */
int idx(char c) {
    for (int i = 0; i < ntCount; i++)
        if (nt[i] == c) return i;
    return -1;
}

/* extract unique non-terminals */
void getNonTerminals() {
    for (int i = 0; i < n; i++) {
        char c = prod[i][0];
        if (idx(c) == -1) {
            nt[ntCount++] = c;
        }
    }
}

int main() {
    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions (A=alpha):\n");
    for (int i = 0; i < n; i++) {
        scanf("%s", prod[i]);
    }

    getNonTerminals();

    for (int i = 0; i < ntCount; i++) {
        first[i][0] = '\0';
        follow[i][0] = '\0';
    }

    /* ================= FIRST ================= */
    int changed = 1;

    while (changed) {
        changed = 0;

        for (int i = 0; i < n; i++) {
            int A = idx(prod[i][0]);
            char *rhs = prod[i] + 2;

            for (int j = 0; rhs[j]; j++) {
                char X = rhs[j];

                if (X == '|') continue;

                if (!isupper(X)) {
                    if (add(first[A], X)) changed = 1;
                    break;
                }

                int B = idx(X);
                int hasEps = 0;

                for (int k = 0; first[B][k]; k++) {
                    if (first[B][k] == '#')
                        hasEps = 1;
                    else if (add(first[A], first[B][k]))
                        changed = 1;
                }

                if (!hasEps)
                    break;

                if (rhs[j+1] == '\0')
                    if (add(first[A], '#')) changed = 1;
            }
        }
    }

    /* ================= FOLLOW ================= */

    // start symbol
    add(follow[0], '$');

    changed = 1;

    while (changed) {
        changed = 0;

        for (int i = 0; i < n; i++) {
            char A = prod[i][0];
            int Aidx = idx(A);

            char *rhs = prod[i] + 2;

            for (int j = 0; rhs[j]; j++) {
                char B = rhs[j];

                if (!isupper(B)) continue;

                int Bidx = idx(B);

                int k = j + 1;
                int hasEps = 1;

                while (rhs[k]) {
                    // if (rhs[k] == '|') {
                    //     hasEps = 1;
                    //     k++;
                    //     continue;
                    // }

                    hasEps = 0;

                    if (!isupper(rhs[k])) {
                        if (add(follow[Bidx], rhs[k])) changed = 1;
                        break;
                    }

                    int Cidx = idx(rhs[k]);
                    int epsInC = 0;

                    for (int x = 0; first[Cidx][x]; x++) {
                        if (first[Cidx][x] == '#')
                            epsInC = 1;
                        else if (add(follow[Bidx], first[Cidx][x]))
                            changed = 1;
                    }

                    if (epsInC) {
                        hasEps = 1;
                        k++;
                    } else {
                        hasEps = 0;
                        break;
                    }
                }

                if (hasEps) {
                    for (int x = 0; follow[Aidx][x]; x++) {
                        if (add(follow[Bidx], follow[Aidx][x]))
                            changed = 1;
                    }
                }
            }
        }
    }

    /* ================= OUTPUT ================= */

    printf("\nFIRST sets:\n");
    for (int i = 0; i < ntCount; i++) {
        printf("FIRST(%c) = { ", nt[i]);
        for (int j = 0; first[i][j]; j++)
            printf("%c ", first[i][j]);
        printf("}\n");
    }

    printf("\nFOLLOW sets:\n");
    for (int i = 0; i < ntCount; i++) {
        printf("FOLLOW(%c) = { ", nt[i]);
        for (int j = 0; follow[i][j]; j++)
            printf("%c ", follow[i][j]);
        printf("}\n");
    }

    return 0;
}