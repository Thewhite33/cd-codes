#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PROD 100
#define MAX_LEN  100
#define MAX_NT   26
#define EPS      '#'

char prod[MAX_PROD][MAX_LEN];
int  pCount = 0;
char first[MAX_NT][MAX_LEN], follow[MAX_NT][MAX_LEN];
bool inFirst[MAX_NT][256], inFollow[MAX_NT][256], isNT[256];

bool addSym(char *set, bool *lookup, char c) {
    if (!c || lookup[(unsigned char)c]) return false;
    lookup[(unsigned char)c] = true;
    int l = strlen(set);
    set[l] = c;
    set[l + 1] = '\0';
    return true;
}

void parseLine(char *line) {
    char lhs = line[0];
    isNT[(unsigned char)lhs] = true;
    char *rhs = strchr(line, '=') + 1;
    prod[pCount][0] = lhs;
    prod[pCount][1] = '=';
    strcpy(&prod[pCount][2], rhs);
    for (int i = 2; prod[pCount][i]; i++)
        if (isupper(prod[pCount][i]))
            isNT[(unsigned char)prod[pCount][i]] = true;
    pCount++;
}

// Adds FIRST of rhs[j..] into (set, lookup); returns true if sequence is epsilon-transparent
bool addFirstOfSeq(char *rhs, int j, char *set, bool *lookup, bool *changed) {
    for (; rhs[j]; j++) {
        char X = rhs[j];
        if (!isupper(X)) {
            if (addSym(set, lookup, X)) *changed = true;
            return false;
        }
        int xi = X - 'A';
        for (int k = 0; first[xi][k]; k++) {
            char s = first[xi][k];
            if (s != EPS && addSym(set, lookup, s)) *changed = true;
        }
        if (!inFirst[xi][(unsigned char)EPS]) return false;
    }
    return true;
}

void computeFirst() {
    bool changed;
    do {
        changed = false;
        for (int p = 0; p < pCount; p++) {
            int ai = prod[p][0] - 'A';
            char *rhs = &prod[p][2];
            bool eps = (!rhs[0] || rhs[0] == EPS) || addFirstOfSeq(rhs, 0, first[ai], inFirst[ai], &changed);
            if (eps && addSym(first[ai], inFirst[ai], EPS)) changed = true;
        }
    } while (changed);
}

void computeFollow(char start) {
    addSym(follow[start - 'A'], inFollow[start - 'A'], '$');
    bool changed;
    do {
        changed = false;
        for (int p = 0; p < pCount; p++) {
            int ai = prod[p][0] - 'A';
            char *rhs = &prod[p][2];
            for (int j = 0; rhs[j]; j++) {
                if (!isupper(rhs[j])) continue;
                int bi = rhs[j] - 'A';
                bool eps = addFirstOfSeq(rhs, j + 1, follow[bi], inFollow[bi], &changed);
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
    int n;
    printf("Enter number of productions: ");
    scanf("%d", &n);
    getchar();
    printf("Enter productions (e.g. A=aB, '#' for epsilon):\n");
    for (int i = 0; i < n; i++) {
        char line[MAX_LEN];
        fgets(line, sizeof(line), stdin);
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line)) parseLine(line);
    }

    computeFirst();
    computeFollow(prod[0][0]);

    char *labels[] = { "FIRST", "FOLLOW" };
    char  (*sets)[MAX_LEN]  = NULL;

    for (int pass = 0; pass < 2; pass++) {
        sets = pass == 0 ? first : follow;
        printf("\n=== %s Sets ===\n", labels[pass]);
        for (int i = 0; i < MAX_NT; i++)
            if (isNT['A' + i] && sets[i][0])
                printSet(labels[pass], 'A' + i, sets[i]);
    }
}