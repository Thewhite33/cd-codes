#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PROD 100
#define MAX_LEN  100
#define MAX_NT   26
#define EPSILON  '#'

/* ── Grammar storage (reused from your previous code) ── */
char productions[MAX_PROD][MAX_LEN];
int  prodCount = 0;
char first[MAX_NT][MAX_LEN],  follow[MAX_NT][MAX_LEN];
bool inFirst[MAX_NT][256],    inFollow[MAX_NT][256];
bool isNT[256];

/* ── Parsing table: table[NT][terminal] = production string ── */
char table[MAX_NT][128][MAX_LEN];

/* ── Same helpers from your code ── */
bool add(char set[MAX_LEN], bool lookup[256], char ch) {
    if (!ch || lookup[(unsigned char)ch]) return false;
    lookup[(unsigned char)ch] = true;
    int l = strlen(set);
    set[l] = ch; set[l+1] = '\0';
    return true;
}
bool hasEpsilon(int idx) { return inFirst[idx][(unsigned char)EPSILON]; }

void parseLine(char *line) {
    char lhs = line[0];
    isNT[(unsigned char)lhs] = true;
    char *eq = strchr(line, '=') + 1;
    char tmp[MAX_LEN]; strcpy(tmp, eq);
    for (char *tok = strtok(tmp, "|"); tok; tok = strtok(NULL, "|")) {
        productions[prodCount][0] = lhs;
        productions[prodCount][1] = '=';
        strcpy(&productions[prodCount][2], tok);
        for (int i = 0; productions[prodCount][2+i]; i++)
            if (isupper(productions[prodCount][2+i]))
                isNT[(unsigned char)productions[prodCount][2+i]] = true;
        prodCount++;
    }
}

/* ── FIRST (identical to your simplified version) ── */
void computeFirst() {
    bool changed;
    do {
        changed = false;
        for (int p = 0; p < prodCount; p++) {
            char A = productions[p][0];
            char *rhs = &productions[p][2];
            int ai = A - 'A';
            if (!rhs[0] || rhs[0] == EPSILON) {
                if (add(first[ai], inFirst[ai], EPSILON)) changed = true;
                continue;
            }
            bool allEps = true;
            for (int j = 0; rhs[j] && allEps; j++) {
                if (!isupper(rhs[j])) {
                    if (add(first[ai], inFirst[ai], rhs[j])) changed = true;
                    allEps = false;
                } else {
                    int xi = rhs[j] - 'A';
                    for (int k = 0; first[xi][k]; k++)
                        if (first[xi][k] != EPSILON)
                            if (add(first[ai], inFirst[ai], first[xi][k])) changed = true;
                    if (!hasEpsilon(xi)) allEps = false;
                }
            }
            if (allEps && add(first[ai], inFirst[ai], EPSILON)) changed = true;
        }
    } while (changed);
}

/* ── FOLLOW (identical to your simplified version) ── */
void computeFollow(char start) {
    add(follow[start-'A'], inFollow[start-'A'], '$');
    bool changed;
    do {
        changed = false;
        for (int p = 0; p < prodCount; p++) {
            char A = productions[p][0];
            char *rhs = &productions[p][2];
            int ai = A - 'A';
            for (int j = 0; rhs[j]; j++) {
                if (!isupper(rhs[j])) continue;
                int bi = rhs[j] - 'A';
                bool eps = true;
                for (int k = j+1; rhs[k] && eps; k++) {
                    if (!isupper(rhs[k])) {
                        if (add(follow[bi], inFollow[bi], rhs[k])) changed = true;
                        eps = false;
                    } else {
                        int xi = rhs[k] - 'A';
                        for (int m = 0; first[xi][m]; m++)
                            if (first[xi][m] != EPSILON)
                                if (add(follow[bi], inFollow[bi], first[xi][m])) changed = true;
                        if (!hasEpsilon(xi)) eps = false;
                    }
                }
                if (eps)
                    for (int k = 0; follow[ai][k]; k++)
                        if (add(follow[bi], inFollow[bi], follow[ai][k])) changed = true;
            }
        }
    } while (changed);
}

/* ── FIRST of a whole RHS string (needed for table building) ── */
void firstOfRHS(char *rhs, char out[MAX_LEN], bool outLookup[256]) {
    if (!rhs[0] || rhs[0] == EPSILON) { add(out, outLookup, EPSILON); return; }
    bool allEps = true;
    for (int j = 0; rhs[j] && allEps; j++) {
        if (!isupper(rhs[j])) {
            add(out, outLookup, rhs[j]); allEps = false;
        } else {
            int xi = rhs[j] - 'A';
            for (int k = 0; first[xi][k]; k++)
                if (first[xi][k] != EPSILON) add(out, outLookup, first[xi][k]);
            if (!hasEpsilon(xi)) allEps = false;
        }
    }
    if (allEps) add(out, outLookup, EPSILON);
}

/* ── Build LL(1) parsing table ── */
void buildTable() {
    for (int p = 0; p < prodCount; p++) {
        char A   = productions[p][0];
        char *rhs = &productions[p][2];
        int  ai  = A - 'A';

        /* compute FIRST(rhs) */
        char   fset[MAX_LEN] = "";
        bool   flook[256]    = {false};
        firstOfRHS(rhs, fset, flook);

        /* Rule 1: for each terminal t in FIRST(rhs), add to table[A][t] */
        for (int i = 0; fset[i]; i++) {
            char t = fset[i];
            if (t == EPSILON) continue;
            if (table[ai][(unsigned char)t][0])
                printf("  [conflict] table[%c][%c] already has: %s\n", A, t, table[ai][(unsigned char)t]);
            else
                strcpy(table[ai][(unsigned char)t], productions[p]);
        }

        /* Rule 2: if epsilon in FIRST(rhs), use FOLLOW(A) */
        if (flook[(unsigned char)EPSILON]) {
            for (int i = 0; follow[ai][i]; i++) {
                char t = follow[ai][i];
                if (table[ai][(unsigned char)t][0])
                    printf("  [conflict] table[%c][%c] already has: %s\n", A, t, table[ai][(unsigned char)t]);
                else
                    strcpy(table[ai][(unsigned char)t], productions[p]);
            }
        }
    }
}

/* ── Collect all terminals that appear in table (for neat printing) ── */
void printTable() {
    /* gather terminals used */
    char terms[64]; int tc = 0;
    bool seen[256] = {false};
    for (int i = 0; i < MAX_NT; i++)
        if (isNT['A'+i])
            for (int t = 0; t < 128; t++)
                if (table[i][t][0] && !seen[t]) { seen[t]=true; terms[tc++]=(char)t; }
    /* sort roughly: lowercase first, then $ */
    for (int a=0;a<tc-1;a++) for(int b=a+1;b<tc;b++) if(terms[a]>terms[b]){char tmp=terms[a];terms[a]=terms[b];terms[b]=tmp;}

    /* header */
    printf("\n%-6s", "");
    for (int j = 0; j < tc; j++) printf("  %-14c", terms[j]);
    printf("\n%-6s", "------");
    for (int j = 0; j < tc; j++) printf("  %-14s", "--------------");
    printf("\n");

    /* rows */
    for (int i = 0; i < MAX_NT; i++) {
        if (!isNT['A'+i]) continue;
        printf("%-6c", 'A'+i);
        for (int j = 0; j < tc; j++) {
            char *entry = table[i][(unsigned char)terms[j]];
            printf("  %-14s", entry[0] ? entry : "-");
        }
        printf("\n");
    }
}

int main() {
    int n;
    printf("Enter number of productions: "); scanf("%d",&n); getchar();
    printf("Enter productions (e.g. E=TE'  E'=+TE'|#  use # for epsilon):\n");
    for (int i = 0; i < n; i++) {
        char line[MAX_LEN];
        fgets(line, sizeof line, stdin);
        line[strcspn(line,"\n")] = '\0';
        if (strlen(line)) parseLine(line);
    }

    computeFirst();
    computeFollow(productions[0][0]);

    printf("\n=== FIRST sets ===\n");
    for (int i = 0; i < MAX_NT; i++)
        if (isNT['A'+i] && first[i][0]){ 
            printf("FIRST(%c) = { ", 'A'+i);
            for (int j=0;first[i][j];j++) printf("%c ",first[i][j]); 
            printf("}\n"); 
        }

    printf("\n=== FOLLOW sets ===\n");
    for (int i = 0; i < MAX_NT; i++)
        if (isNT['A'+i] && follow[i][0])
            { printf("FOLLOW(%c) = { ", 'A'+i);
              for (int j=0;follow[i][j];j++) printf("%c ",follow[i][j]); printf("}\n"); }

    buildTable();
    printf("\n=== LL(1) Parsing Table ===\n");
    printTable();
    return 0;
}