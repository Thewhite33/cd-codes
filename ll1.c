#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20

char prod[MAX][MAX];
char nt[MAX];              // non-terminals
char t[50];                // terminals
char first[MAX][MAX];
char follow[MAX][MAX];
char table[MAX][50][MAX];

int n, ntCount = 0, tCount = 0;

/* ---------------- UTIL ---------------- */

int contains(char set[], char ch) {
    for (int i = 0; set[i]; i++)
        if (set[i] == ch) return 1;
    return 0;
}

int add(char set[], char ch) {
    if (!contains(set, ch)) {
        int l = strlen(set);
        set[l] = ch;
        set[l+1] = '\0';
        return 1;
    }
    return 0;
}

int idxNT(char c) {
    for (int i = 0; i < ntCount; i++)
        if (nt[i] == c) return i;
    return -1;
}

int idxT(char c) {
    for (int i = 0; i < tCount; i++)
        if (t[i] == c) return i;
    return -1;
}

/* ---------------- EXTRACT SYMBOLS ---------------- */

void getNT() {
    for (int i = 0; i < n; i++) {
        if (idxNT(prod[i][0]) == -1)
            nt[ntCount++] = prod[i][0];
    }
}

void getT() {
    for (int i = 0; i < n; i++) {
        for (int j = 2; prod[i][j]; j++) {
            char c = prod[i][j];
            if (!isupper(c) && c != '#' && c != '|') {
                if (idxT(c) == -1)
                    t[tCount++] = c;
            }
        }
    }
    t[tCount++] = '$';
}

/* ---------------- FIRST ---------------- */

void computeFirst() {
    int changed = 1;

    while (changed) {
        changed = 0;

        for (int i = 0; i < n; i++) {
            int A = idxNT(prod[i][0]);
            char *rhs = prod[i] + 2;

            for (int j = 0; rhs[j]; j++) {
                char X = rhs[j];

                if (X == '|') continue;

                if (!isupper(X)) {
                    if (add(first[A], X)) changed = 1;
                    break;
                }

                int B = idxNT(X);
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
}

/* FIRST of string */
void firstOfString(char *rhs, int pos, char result[]) {
    if (rhs[pos] == '\0') {
        add(result, '#');
        return;
    }

    if (!isupper(rhs[pos])) {
        add(result, rhs[pos]);
        return;
    }

    int id = idxNT(rhs[pos]);
    int hasEps = 0;

    for (int i = 0; first[id][i]; i++) {
        if (first[id][i] == '#')
            hasEps = 1;
        else
            add(result, first[id][i]);
    }

    if (hasEps)
        firstOfString(rhs, pos + 1, result);
}

/* ---------------- FOLLOW ---------------- */

void computeFollow() {
    add(follow[0], '$');   // start symbol

    int changed = 1;

    while (changed) {
        changed = 0;

        for (int i = 0; i < n; i++) {
            char A = prod[i][0];
            int Aidx = idxNT(A);

            char *rhs = prod[i] + 2;

            for (int j = 0; rhs[j]; j++) {
                char B = rhs[j];

                if (!isupper(B)) continue;

                int Bidx = idxNT(B);
                int k = j + 1;
                int hasEps = 1;

                while (rhs[k]) {
                    if (rhs[k] == '|') {
                        hasEps = 1;
                        k++;
                        continue;
                    }

                    hasEps = 0;

                    if (!isupper(rhs[k])) {
                        if (add(follow[Bidx], rhs[k])) changed = 1;
                        break;
                    }

                    int Cidx = idxNT(rhs[k]);
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
}

/* ---------------- TABLE ---------------- */

void buildTable() {
    for (int i = 0; i < ntCount; i++)
        for (int j = 0; j < tCount; j++)
            strcpy(table[i][j], "");

    for (int i = 0; i < n; i++) {
        char A = prod[i][0];
        int Aidx = idxNT(A);

        char *rhs = prod[i] + 2;

        char temp[MAX] = "";
        firstOfString(rhs, 0, temp);

        for (int j = 0; temp[j]; j++) {
            if (temp[j] == '#') continue;

            int tidx = idxT(temp[j]);

            if (strlen(table[Aidx][tidx]) != 0)
                printf("Conflict at [%c, %c]\n", A, temp[j]);

            strcpy(table[Aidx][tidx], prod[i]);
        }

        if (contains(temp, '#')) {
            for (int j = 0; follow[Aidx][j]; j++) {
                int tidx = idxT(follow[Aidx][j]);

                if (strlen(table[Aidx][tidx]) != 0)
                    printf("Conflict at [%c, %c]\n", A, follow[Aidx][j]);

                strcpy(table[Aidx][tidx], prod[i]);
            }
        }
    }
}

/* ---------------- PRINT ---------------- */

void printFirst() {
    printf("\nFIRST:\n");
    for (int i = 0; i < ntCount; i++) {
        printf("FIRST(%c) = { ", nt[i]);
        for (int j = 0; first[i][j]; j++)
            printf("%c ", first[i][j]);
        printf("}\n");
    }
}

void printFollow() {
    printf("\nFOLLOW:\n");
    for (int i = 0; i < ntCount; i++) {
        printf("FOLLOW(%c) = { ", nt[i]);
        for (int j = 0; follow[i][j]; j++)
            printf("%c ", follow[i][j]);
        printf("}\n");
    }
}

void printTable() {
    printf("\nLL(1) Parsing Table:\n\n");

    printf("     ");
    for (int i = 0; i < tCount; i++)
        printf("%5c", t[i]);
    printf("\n");

    for (int i = 0; i < ntCount; i++) {
        printf("%5c", nt[i]);
        for (int j = 0; j < tCount; j++) {
            if (strlen(table[i][j]) == 0)
                printf("%5s", "-");
            else
                printf("%5s", table[i][j]);
        }
        printf("\n");
    }
}

/* ---------------- MAIN ---------------- */

int main() {
    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions (A=alpha):\n");
    for (int i = 0; i < n; i++)
        scanf("%s", prod[i]);

    getNT();
    getT();

    computeFirst();
    computeFollow();
    buildTable();

    printFirst();
    printFollow();
    printTable();

    return 0;
}