#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_PROD   20
#define MAX_STATES 100
#define MAX_ITEMS  100
#define MAX_SYM    50
#define MAX_LEN    20

/* ═══════════════════════════════════════
   GRAMMAR
═══════════════════════════════════════ */
typedef struct { char lhs; char rhs[MAX_LEN]; } Prod;
Prod gram[MAX_PROD];
int  gCnt = 0;
char startSym;

/* ═══════════════════════════════════════
   FIRST / FOLLOW
═══════════════════════════════════════ */
char First[128][MAX_SYM];  int fCnt[128];
char Follow[128][MAX_SYM]; int foCnt[128];

int  inSet(char *s,int c,char x){ for(int i=0;i<c;i++) if(s[i]==x) return 1; return 0; }
void addSet(char *s,int *c,char x){ if(!inSet(s,*c,x)) s[(*c)++]=x; }

void computeFirst(){
    int ch;
    do{
        ch=0;
        for(int p=0;p<gCnt;p++){
            char A=gram[p].lhs; char *r=gram[p].rhs;
            if(!r[0]||r[0]=='#'){if(!inSet(First[A],fCnt[A],'#')){addSet(First[A],&fCnt[A],'#');ch=1;}continue;}
            int ae=1;
            for(int j=0;r[j]&&ae;j++){
                if(!isupper(r[j])){if(!inSet(First[A],fCnt[A],r[j])){addSet(First[A],&fCnt[A],r[j]);ch=1;}ae=0;}
                else{
                    for(int k=0;k<fCnt[(int)r[j]];k++)
                        if(First[(int)r[j]][k]!='#'&&!inSet(First[A],fCnt[A],First[(int)r[j]][k])){addSet(First[A],&fCnt[A],First[(int)r[j]][k]);ch=1;}
                    if(!inSet(First[(int)r[j]],fCnt[(int)r[j]],'#')) ae=0;
                }
            }
            if(ae&&!inSet(First[A],fCnt[A],'#')){addSet(First[A],&fCnt[A],'#');ch=1;}
        }
    }while(ch);
}

void computeFollow(){
    addSet(Follow[(int)startSym],&foCnt[(int)startSym],'$');
    int ch;
    do{
        ch=0;
        for(int p=0;p<gCnt;p++){
            char A=gram[p].lhs; char *r=gram[p].rhs;
            for(int j=0;r[j];j++){
                if(!isupper(r[j])) continue;
                char B=r[j]; int eps=1;
                for(int k=j+1;r[k]&&eps;k++){
                    if(!isupper(r[k])){if(!inSet(Follow[B],foCnt[B],r[k])){addSet(Follow[B],&foCnt[B],r[k]);ch=1;}eps=0;}
                    else{
                        for(int m=0;m<fCnt[(int)r[k]];m++)
                            if(First[(int)r[k]][m]!='#'&&!inSet(Follow[B],foCnt[B],First[(int)r[k]][m])){addSet(Follow[B],&foCnt[B],First[(int)r[k]][m]);ch=1;}
                        if(!inSet(First[(int)r[k]],fCnt[(int)r[k]],'#')) eps=0;
                    }
                }
                if(eps) for(int k=0;k<foCnt[(int)A];k++)
                    if(!inSet(Follow[B],foCnt[B],Follow[A][k])){addSet(Follow[B],&foCnt[B],Follow[A][k]);ch=1;}
            }
        }
    }while(ch);
}

/* ═══════════════════════════════════════
   LR(0) ITEMS + STATES
═══════════════════════════════════════ */
typedef struct { int prod; int dot; } Item;
typedef struct { Item items[MAX_ITEMS]; int cnt; } State;

State states[MAX_STATES];
int   sCnt = 0;

int  itemEq(Item a,Item b){ return a.prod==b.prod&&a.dot==b.dot; }
int  inState(State *s,Item it){ for(int i=0;i<s->cnt;i++) if(itemEq(s->items[i],it)) return 1; return 0; }
void addItem(State *s,Item it){ if(!inState(s,it)) s->items[s->cnt++]=it; }
char afterDot(Item it){ char *r=gram[it.prod].rhs; return r[it.dot]?r[it.dot]:0; }

void closure(State *s){
    int ch;
    do{
        ch=0;
        for(int i=0;i<s->cnt;i++){
            char X=afterDot(s->items[i]);
            if(!X||!isupper(X)) continue;
            for(int p=0;p<gCnt;p++)
                if(gram[p].lhs==X){Item ni={p,0};if(!inState(s,ni)){addItem(s,ni);ch=1;}}
        }
    }while(ch);
}

State goTo(State *s,char X){
    State ns; ns.cnt=0;
    for(int i=0;i<s->cnt;i++)
        if(afterDot(s->items[i])==X){Item ni={s->items[i].prod,s->items[i].dot+1};addItem(&ns,ni);}
    closure(&ns);
    return ns;
}

int stateEq(State *a,State *b){
    if(a->cnt!=b->cnt) return 0;
    for(int i=0;i<a->cnt;i++) if(!inState(b,a->items[i])) return 0;
    return 1;
}
int findState(State *ns){ for(int i=0;i<sCnt;i++) if(stateEq(&states[i],ns)) return i; return -1; }

void buildStates(){
    State s0; s0.cnt=0;
    addItem(&s0,(Item){0,0});
    closure(&s0);
    states[sCnt++]=s0;
    for(int i=0;i<sCnt;i++){
        char syms[MAX_SYM]; int sc=0;
        for(int j=0;j<states[i].cnt;j++){
            char X=afterDot(states[i].items[j]); if(!X) continue;
            int f=0; for(int k=0;k<sc;k++) if(syms[k]==X){f=1;break;}
            if(!f) syms[sc++]=X;
        }
        for(int j=0;j<sc;j++){
            State ns=goTo(&states[i],syms[j]);
            if(ns.cnt==0) continue;
            if(findState(&ns)<0) states[sCnt++]=ns;
        }
    }
}

/* ═══════════════════════════════════════
   SLR TABLE
═══════════════════════════════════════ */
char action[MAX_STATES][128][8];
int  gotot[MAX_STATES][128];

void buildTable(){
    for(int i=0;i<MAX_STATES;i++)
        for(int j=0;j<128;j++){action[i][j][0]=0;gotot[i][j]=-1;}

    for(int i=0;i<sCnt;i++)
        for(int j=0;j<states[i].cnt;j++){
            Item it=states[i].items[j];
            char X=afterDot(it);
            if(X){
                State ns=goTo(&states[i],X); int nsi=findState(&ns);
                if(nsi<0) continue;
                if(!isupper(X)){
                    char buf[8]; sprintf(buf,"s%d",nsi);
                    if(!action[i][(int)X][0]) strcpy(action[i][(int)X],buf);
                } else {
                    gotot[i][(int)X]=nsi;
                }
            } else {
                /* dot at end */
                if(it.prod==0){   /* augmented production → accept */
                    if(!action[i]['$'][0]) strcpy(action[i]['$'],"acc");
                } else {
                    char A=gram[it.prod].lhs;
                    char buf[8]; sprintf(buf,"r%d",it.prod);
                    for(int k=0;k<foCnt[(int)A];k++){
                        char t=Follow[A][k];
                        if(!action[i][(int)t][0]) strcpy(action[i][(int)t],buf);
                        else if(strcmp(action[i][(int)t],buf)!=0)
                            printf("  [conflict] state %d on '%c': %s vs %s\n",i,t,action[i][(int)t],buf);
                    }
                }
            }
        }
}

/* ═══════════════════════════════════════
   PRINT TABLE
═══════════════════════════════════════ */
void printTable(){
    /* collect terminals and non-terminals actually used */
    char terms[MAX_SYM]; int tc=0;
    char nts[MAX_SYM];   int nc=0;
    for(int i=0;i<sCnt;i++)
        for(int j=0;j<128;j++){
            if(action[i][j][0]&&!inSet(terms,tc,(char)j)) terms[tc++]=(char)j;
            if(gotot[i][j]>=0 &&!inSet(nts,nc,(char)j))   nts[nc++]=(char)j;
        }
    /* simple sort */
    for(int a=0;a<tc-1;a++) for(int b=a+1;b<tc;b++) if(terms[a]>terms[b]){char t=terms[a];terms[a]=terms[b];terms[b]=t;}
    for(int a=0;a<nc-1;a++) for(int b=a+1;b<nc;b++) if(nts[a]>nts[b])  {char t=nts[a];  nts[a]=nts[b];    nts[b]=t;}

    printf("\n=== SLR Parsing Table ===\n");
    printf("%-5s |","St");
    for(int i=0;i<tc;i++) printf(" %-6c",terms[i]);
    printf(" |");
    for(int i=0;i<nc;i++) printf(" %-4c",nts[i]);
    printf("\n");
    for(int i=0;i<6+tc*7+nc*5+4;i++) printf("-"); printf("\n");

    for(int i=0;i<sCnt;i++){
        printf("%-5d |",i);
        for(int j=0;j<tc;j++) printf(" %-6s",action[i][(int)terms[j]][0]?action[i][(int)terms[j]]:"-");
        printf(" |");
        for(int j=0;j<nc;j++){int g=gotot[i][(int)nts[j]];if(g>=0){char b[8];sprintf(b,"%d",g);printf(" %-4s",b);}else printf(" %-4s","-");}
        printf("\n");
    }
}

/* ═══════════════════════════════════════
   PARSE
═══════════════════════════════════════ */
void parse(char *inp){
    char buf[100]; sprintf(buf,"%s$",inp);
    int  stk[200]; int sp=0; stk[0]=0;
    char sym[200]; int syp=-1;

    printf("\n=== Parsing \"%s\" ===\n",inp);
    printf("%-25s %-20s %-15s\n","Stack","Input","Action");
    printf("%-25s %-20s %-15s\n","-----","-----","------");

    int pos=0;
    while(1){
        char sstr[100]="0";
        for(int i=0;i<sp;i++){char t[10];sprintf(t,"%c%d",sym[i],stk[i+1]);strcat(sstr,t);}
        printf("%-25s %-20s ",sstr,buf+pos);

        int s=stk[sp]; char a=buf[pos];
        char *act=action[s][(int)a];

        if(!act[0]){printf("Error: no action\n");return;}
        if(act[0]=='s'){
            int ns=atoi(act+1);
            printf("shift s%d\n",ns);
            sym[++syp]=a; stk[++sp]=ns; pos++;
        } else if(act[0]=='r'){
            int pn=atoi(act+1);
            printf("reduce %c -> %s\n",gram[pn].lhs,gram[pn].rhs);
            int len=(gram[pn].rhs[0]=='#')?0:strlen(gram[pn].rhs);
            sp-=len; syp-=len;
            char A=gram[pn].lhs; sym[++syp]=A;
            int gs=gotot[stk[sp]][(int)A];
            if(gs<0){printf("Error: no goto\n");return;}
            stk[++sp]=gs;
        } else {
            printf("Accept!\n"); return;
        }
    }
}

/* ═══════════════════════════════════════
   MAIN — fully user input
═══════════════════════════════════════ */
int main(){
    int n;
    printf("Enter number of productions (excluding augmented): ");
    scanf("%d",&n); getchar();
    printf("Enter productions as  A=aB|c  (uppercase=NT, # =epsilon):\n");

    char userNT[MAX_PROD];   /* to track user's non-terminals in order */
    int  uCnt=0;
    char lines[MAX_PROD][MAX_LEN*2];

    for(int i=0;i<n;i++){
        printf("  P%d: ",i+1);
        fgets(lines[i],sizeof(lines[i]),stdin);
        lines[i][strcspn(lines[i],"\n")]=0;
        char lhs=lines[i][0];
        if(!inSet(userNT,uCnt,lhs)) userNT[uCnt++]=lhs;
    }

    /* augmented start:  S' → original start */
    char origStart = userNT[0];
    char augStart  = (origStart=='S') ? 'Z' : 'S';  /* avoid clash */
    gram[gCnt].lhs=augStart;
    gram[gCnt].rhs[0]=origStart; gram[gCnt].rhs[1]=0;
    gCnt++;
    startSym=augStart;

    /* parse user lines into gram[] */
    for(int i=0;i<n;i++){
        char lhs=lines[i][0];
        char *rhs=strchr(lines[i],'=')+1;
        char tmp[MAX_LEN*2]; strcpy(tmp,rhs);
        for(char *tok=strtok(tmp,"|");tok;tok=strtok(NULL,"|")){
            gram[gCnt].lhs=lhs;
            strcpy(gram[gCnt].rhs,tok);
            gCnt++;
        }
    }

    computeFirst();
    computeFollow();

    /* print FIRST / FOLLOW for user's NTs only */
    printf("\n=== FIRST sets ===\n");
    for(int i=0;i<uCnt;i++){
        printf("FIRST(%c) = { ",userNT[i]);
        for(int j=0;j<fCnt[(int)userNT[i]];j++) printf("%c ",First[(int)userNT[i]][j]);
        printf("}\n");
    }
    printf("\n=== FOLLOW sets ===\n");
    for(int i=0;i<uCnt;i++){
        printf("FOLLOW(%c) = { ",userNT[i]);
        for(int j=0;j<foCnt[(int)userNT[i]];j++) printf("%c ",Follow[(int)userNT[i]][j]);
        printf("}\n");
    }

    /* print productions with numbers */
    printf("\n=== Productions ===\n");
    for(int i=0;i<gCnt;i++)
        printf("  P%d: %c -> %s\n",i,gram[i].lhs,gram[i].rhs);

    buildStates();

    printf("\n=== LR(0) States ===\n");
    for(int i=0;i<sCnt;i++){
        printf("I%d:\n",i);
        for(int j=0;j<states[i].cnt;j++){
            Item it=states[i].items[j]; char *r=gram[it.prod].rhs;
            printf("  %c ->",gram[it.prod].lhs);
            for(int k=0;r[k];k++){if(k==it.dot) printf(" ."); printf(" %c",r[k]);}
            if(it.dot==(int)strlen(r)) printf(" .");
            printf("\n");
        }
    }

    buildTable();
    printTable();

    char inp[50];
    printf("\nEnter input string to parse: "); scanf("%s",inp);
    parse(inp);
    return 0;
}