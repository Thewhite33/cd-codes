#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 20
#define MAX_ITEMS 100

char prod[MAX][10];
char augmented[10];
int n;

/* LR(0) Item */
typedef struct {
    char lhs;
    char rhs[10];
} Item;

Item items[MAX_ITEMS];
int itemCount = 0;

/* States */
Item states[50][50];
int stateCount = 0;
int stateSize[50];

/* FOLLOW */
char follow[MAX][MAX];

/* ---------- helper ---------- */

int isNonTerminal(char c) {
    return isupper(c);
}

/* ---------- closure ---------- */

void closure(Item state[], int *size) {
    int changed = 1;

    while (changed) {
        changed = 0;

        for (int i = 0; i < *size; i++) {
            char *rhs = state[i].rhs;

            for (int j = 0; rhs[j]; j++) {
                if (rhs[j] == '.') {
                    char next = rhs[j+1];

                    if (isNonTerminal(next)) {
                        for (int k = 0; k < n; k++) {
                            if (prod[k][0] == next) {
                                Item newItem;
                                newItem.lhs = next;

                                newItem.rhs[0] = '.';
                                strcpy(newItem.rhs+1, prod[k]+2);

                                int exists = 0;
                                for (int x = 0; x < *size; x++) {
                                    if (strcmp(state[x].rhs, newItem.rhs) == 0 &&
                                        state[x].lhs == newItem.lhs) {
                                        exists = 1;
                                        break;
                                    }
                                }

                                if (!exists) {
                                    state[(*size)++] = newItem;
                                    changed = 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/* ---------- GOTO ---------- */

void gotoState(Item src[], int srcSize, char symbol,
               Item dest[], int *destSize) {

    *destSize = 0;

    for (int i = 0; i < srcSize; i++) {
        char *rhs = src[i].rhs;

        for (int j = 0; rhs[j]; j++) {
            if (rhs[j] == '.' && rhs[j+1] == symbol) {

                Item newItem;
                newItem.lhs = src[i].lhs;

                strcpy(newItem.rhs, rhs);
                char temp = newItem.rhs[j];
                newItem.rhs[j] = newItem.rhs[j+1];
                newItem.rhs[j+1] = temp;

                dest[(*destSize)++] = newItem;
            }
        }
    }

    closure(dest, destSize);
}

/* ---------- state comparison ---------- */

int sameState(Item a[], int asz, Item b[], int bsz) {
    if (asz != bsz) return 0;

    for (int i = 0; i < asz; i++) {
        int found = 0;
        for (int j = 0; j < bsz; j++) {
            if (a[i].lhs == b[j].lhs &&
                strcmp(a[i].rhs, b[j].rhs) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) return 0;
    }
    return 1;
}

/* ---------- build canonical collection ---------- */

void buildStates() {

    Item start[50];
    int sz = 0;

    start[0].lhs = augmented[0];
    strcpy(start[0].rhs, augmented+2);
    sz = 1;

    closure(start, &sz);

    for (int i = 0; i < sz; i++)
        states[0][i] = start[i];

    stateSize[0] = sz;
    stateCount = 1;

    for (int i = 0; i < stateCount; i++) {

        for (char X = 'A'; X <= 'Z'; X++) {

            Item temp[50];
            int tempSize = 0;

            gotoState(states[i], stateSize[i], X, temp, &tempSize);

            if (tempSize == 0) continue;

            int exists = -1;

            for (int j = 0; j < stateCount; j++) {
                if (sameState(states[j], stateSize[j], temp, tempSize)) {
                    exists = j;
                    break;
                }
            }

            if (exists == -1) {
                for (int k = 0; k < tempSize; k++)
                    states[stateCount][k] = temp[k];

                stateSize[stateCount] = tempSize;
                stateCount++;
            }
        }
    }
}

/* ---------- print states ---------- */

void printStates() {
    printf("\nLR(0) States:\n");

    for (int i = 0; i < stateCount; i++) {
        printf("\nI%d:\n", i);
        for (int j = 0; j < stateSize[i]; j++) {
            printf("%c -> %s\n",
                   states[i][j].lhs,
                   states[i][j].rhs);
        }
    }
}

/* ---------- MAIN ---------- */

int main() {

    printf("Enter number of productions: ");
    scanf("%d", &n);

    printf("Enter productions (A=alpha):\n");
    for (int i = 0; i < n; i++)
        scanf("%s", prod[i]);

    /* Augmented grammar */
    augmented[0] = 'Z';   // new start
    augmented[1] = '=';
    augmented[2] = prod[0][0];
    augmented[3] = '\0';

    printf("\nAugmented: %s\n", augmented);

    buildStates();
    printStates();

    return 0;
}