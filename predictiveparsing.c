#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 30
#define MAX_LEN 100

char prod[MAX][MAX_LEN];
char nt[MAX];
char t[MAX];
char first[MAX][MAX];
char follow[MAX][MAX];
char table[MAX][MAX][MAX_LEN];

int n, ntCount = 0, tCount = 0;

/* ---------- utility ---------- */

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

/* ---------- extract symbols ---------- */

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
            if (!isupper(c) && c != '#') {
                if (idxT(c) == -1)
                    t[tCount++] = c;
            }
        }
    }
    t[tCount++] = '$';
}

/* ---------- FIRST ---------- */

void computeFirst() {
    int changed = 1;

    while (changed) {
        changed = 0;

        for (int i = 0; i < n; i++) {
            int A = idxNT(prod[i][0]);
            char *rhs = prod[i] + 2;

            for (int j = 0; rhs[j]; j++) {
                char X = rhs[j];

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

void firstOfString(char str[], int pos, char result[]) {
    if (str[pos] == '\0') {
        add(result, '#');
        return;
    }

    if (!isupper(str[pos])) {
        add(result, str[pos]);
        return;
    }

    int id = idxNT(str[pos]);
    int hasEps = 0;

    for (int i = 0; first[id][i]; i++) {
        if (first[id][i] == '#')
            hasEps = 1;
        else
            add(result, first[id][i]);
    }

    if (hasEps)
        firstOfString(str, pos + 1, result);
}

/* ---------- FOLLOW ---------- */

void computeFollow() {
    add(follow[0], '$');

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

/* ---------- TABLE ---------- */

void buildTable() {
    for (int i = 0; i < ntCount; i++)
        for (int j = 0; j < tCount; j++)
            table[i][j][0] = '\0';

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

/* ---------- PRINT ---------- */

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
    printf("\nParsing Table:\n\n");

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

/* ---------- PARSER ---------- */

void pushReverse(char stack[], int *top, char rhs[]) {
    if (strcmp(rhs, "#") == 0) return;

    for (int i = strlen(rhs) - 1; i >= 0; i--)
        stack[++(*top)] = rhs[i];
}

void getRHS(char prod[], char rhs[]) {
    int i = 2, j = 0;
    while (prod[i]) rhs[j++] = prod[i++];
    rhs[j] = '\0';
}

void predictiveParse(char input[]) {
    char stack[MAX];
    int top = -1;

    stack[++top] = '$';
    stack[++top] = nt[0];

    int ip = 0;

    printf("\nSTACK\tINPUT\tACTION\n");

    while (top >= 0) {
        char X = stack[top];
        char a = input[ip];

        if (X == '$' && a == '$') {
            printf("$\t$\tACCEPT\n");
            return;
        }

        if (!isupper(X)) {
            if (X == a) {
                printf("%c\t%s\tMatch\n", X, input+ip);
                top--; ip++;
            } else {
                printf("ERROR\n");
                return;
            }
        } else {
            int row = idxNT(X);
            int col = idxT(a);

            if (strlen(table[row][col]) == 0) {
                printf("ERROR\n");
                return;
            }

            char production[MAX_LEN];
            strcpy(production, table[row][col]);

            printf("%c\t%s\t%s\n", X, input+ip, production);

            top--;

            char rhs[MAX_LEN];
            getRHS(production, rhs);
            pushReverse(stack, &top, rhs);
        }
    }
}

/* ---------- MAIN ---------- */

int main() {
    char input[MAX_LEN];

    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions (A=alpha):\n");
    for (int i = 0; i < n; i++)
        scanf("%s", prod[i]);

    getNT();
    getT();

    for (int i = 0; i < ntCount; i++) {
        first[i][0] = '\0';
        follow[i][0] = '\0';
    }

    computeFirst();
    computeFollow();
    buildTable();

    printFirst();
    printFollow();
    printTable();

    printf("\nEnter input string: ");
    scanf("%s", input);
    strcat(input, "$");

    predictiveParse(input);

    return 0;
}