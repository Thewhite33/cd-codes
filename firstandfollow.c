#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PROD 100
#define MAX_LEN 100
#define MAX_NT 26
#define EPS '#'

char prod[MAX_PROD][MAX_LEN];
int pCount = 0;
char first[MAX_NT][MAX_LEN], follow[MAX_NT][MAX_LEN];
bool inFirst[MAX_NT][256], inFollow[MAX_NT][256], isNT[256];

bool addSym(char *set, bool *lookup, char c) {
    if (!c || lookup[(unsigned char)c]) return false;
    lookup[(unsigned char)c] = true;
    int l = strlen(set);
    set[l] = c; set[l+1] = '\0';
    return true;
}

void parseLine(char *line) {
    char lhs = line[0];
    isNT[(unsigned char)lhs] = true;
    char *rhs = strchr(line, '=') + 1;
    for (char *tok = strtok(rhs, "|"); tok; tok = strtok(NULL, "|")) {
        prod[pCount][0] = lhs; prod[pCount][1] = '=';
        strcpy(&prod[pCount][2], tok);
        for (int i = 0; prod[pCount][2+i]; i++)
            if (isupper(prod[pCount][2+i])) isNT[(unsigned char)prod[pCount][2+i]] = true;
        pCount++;
    }
}

// Add FIRST of symbol sequence rhs[j..] into set[idx], return if epsilon-transparent
bool addFirstOfSeq(char *rhs, int j, int idx, bool *changed, bool isFirst) {
    bool allEps = true;
    for (; rhs[j] && allEps; j++) {
        char X = rhs[j];
        int xi = X - 'A';
        if (!isupper(X)) {
            if (isFirst ? addSym(first[idx], inFirst[idx], X)
                        : addSym(follow[idx], inFollow[idx], X)) *changed = true;
            allEps = false;
        } else {
            for (int k = 0; first[xi][k]; k++) {
                char s = first[xi][k];
                if (s != EPS && (isFirst ? addSym(first[idx], inFirst[idx], s)
                                         : addSym(follow[idx], inFollow[idx], s))) *changed = true;
            }
            if (!inFirst[xi][(unsigned char)EPS]) allEps = false;
        }
    }
    return allEps;
}

void computeFirst() {
    bool changed;
    do {
        changed = false;
        for (int p = 0; p < pCount; p++) {
            char A = prod[p][0]; char *rhs = &prod[p][2];
            int ai = A - 'A';
            if (!rhs[0] || rhs[0] == EPS) { addSym(first[ai], inFirst[ai], EPS); continue; }
            if (addFirstOfSeq(rhs, 0, ai, &changed, true))
                if (addSym(first[ai], inFirst[ai], EPS)) changed = true;
        }
    } while (changed);
}

void computeFollow(char start) {
    addSym(follow[start-'A'], inFollow[start-'A'], '$');
    bool changed;
    do {
        changed = false;
        for (int p = 0; p < pCount; p++) {
            char A = prod[p][0]; char *rhs = &prod[p][2];
            int ai = A - 'A';
            for (int j = 0; rhs[j]; j++) {
                if (!isupper(rhs[j])) continue;
                int bi = rhs[j] - 'A';
                bool eps = rhs[j+1] ? addFirstOfSeq(rhs, j+1, bi, &changed, false) : true;
                if (eps)
                    for (int k = 0; follow[ai][k]; k++)
                        if (addSym(follow[bi], inFollow[bi], follow[ai][k])) changed = true;
            }
        }
    } while (changed);
}

void printSet(char *label, char sym, char *set) {
    printf("%s(%c) = { ", label, sym);
    for (int i = 0; set[i]; i++) printf("%c ", set[i]);
    printf("}\n");
}

int main() {
    int n; printf("Enter number of productions: "); scanf("%d", &n); getchar();
    printf("Enter productions (e.g. A=aB|c, '#' for epsilon):\n");
    for (int i = 0; i < n; i++) {
        char line[MAX_LEN]; fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line)) parseLine(line);
    }

    computeFirst();
    computeFollow(prod[0][0]);

    printf("\n=== FIRST Sets ===\n");
    for (int i = 0; i < MAX_NT; i++)
        if (isNT['A'+i] && first[i][0]) printSet("FIRST", 'A'+i, first[i]);

    printf("\n=== FOLLOW Sets ===\n");
    for (int i = 0; i < MAX_NT; i++)
        if (isNT['A'+i] && follow[i][0]) printSet("FOLLOW", 'A'+i, follow[i]);
}