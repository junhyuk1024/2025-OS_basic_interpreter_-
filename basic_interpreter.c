/* Basic Interpreter by H?eyin Uslu raistlinthewiz@hotmail.com */
/* Code licenced under GPL */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#define CLEAR() system("cls")
#else
#define CLEAR() system("clear")
#endif

struct node {
    int type; /* 1 var, 2 function, 3 function call, 4 begin, 5 end */
    char exp_data;
    int val;
    int line;
    struct node* next;
};/*구조체 node 선언*/
typedef struct node Node;

struct stack { Node* top; };/*stack 구조선언*/
typedef struct stack Stack;

struct opnode { char op; struct opnode* next; };/*opnode 구조선언언*/
typedef struct opnode opNode;

struct opstack { opNode* top; };/*opNode 구조 선언언*/
typedef struct opstack OpStack;

struct postfixnode { int val; struct postfixnode* next; };/*postfixnode 구조 선언언*/
typedef struct postfixnode Postfixnode;

struct postfixstack { Postfixnode* top; };/*postfixstack 구조 선언언*/
typedef struct postfixstack PostfixStack;

static int GetVal(char, int*, Stack*);
static int GetLastFunctionCall(Stack*);
static Stack* FreeAll(Stack*);
static int my_stricmp(const char* a, const char* b);
static void rstrip(char* s);
/*코드에 필요한 함수를 구조체로 선언언*/
static Stack* Push(Node sNode, Stack* stck)/*stack push 연산산*/
{
    Node* newnode = (Node*)malloc(sizeof(Node));/*newnode 동적 할당당*/
    if (!newnode) { printf("ERROR, Couldn't allocate memory..."); return NULL; }/*에러처리리*/
    newnode->type = sNode.type;
    newnode->val = sNode.val;
    newnode->exp_data = sNode.exp_data;
    newnode->line = sNode.line;/*newnode구조체 안 변수 매핑핑*/
    newnode->next = stck->top;./*삽입된노드가 top을 가리킴킴*/
    stck->top = newnode;/*stck의 top을 newnode로 업데이트트*/
    return stck;/*stack 포인터터반환*/
}

static OpStack* PushOp(char op, OpStack* opstck)/*OpStack push 연산산*/
{
    opNode* newnode = (opNode*)malloc(sizeof(opNode));/*newnode 동적 할당당*/
    if (!newnode) { printf("ERROR, Couldn't allocate memory..."); return NULL; }/*에러 처리리*/
    newnode->op = op;/*newnode구조체 안 변수 매핑핑*/
    newnode->next = opstck->top;/*삽입된 노드가 top을 가리킴킴*/
    opstck->top = newnode;/*top을 newnode로 업데이트트*/
    return opstck;/*opstck 포인터터반환*/
}

static char PopOp(OpStack* opstck)/*PopOp 삭제 연산*/
{
    opNode* temp;
    char op;
    if (opstck->top == NULL)
    {
        return 0;
    }/*stack에 값이 없으면 0반환환*/
    op = opstck->top->op;/*삭제할 값 op에 저장*/
    temp = opstck->top;/*top을 temp에 저장장*/
    opstck->top = opstck->top->next;/*top 내리기기*/
    free(temp);/*메모리 해제제*/
    return op;/*삭제된값 반환환*/
}

static PostfixStack* PushPostfix(int val, PostfixStack* poststck)/*PushPostfix 후위표기식식*/
{
    Postfixnode* newnode = (Postfixnode*)malloc(sizeof(Postfixnode));/*newnode 동적할당당*/
    if (!newnode) { printf("ERROR, Couldn't allocate memory..."); return NULL; }/*에러 처리리*/
    newnode->val = val;/*newnode구조체 안 변수 매핑핑*/
    newnode->next = poststck->top;/*삽입된 노드가 top을 가리킴킴*/
    poststck->top = newnode;/*top을 newnode로 업데이트트*/
    return poststck;/*poststck 포인터터반환*/
}

static int PopPostfix(PostfixStack* poststck)/*PopPostfix 후위표기식*/
{
    Postfixnode* temp;
    int val;
    if (poststck->top == NULL)
    {
        return 0;
    }/*stack에 값이 없으면 0반환환*/
    val = poststck->top->val;/*삭제할 값 val에 저장*/
    temp = poststck->top;/*top을 temp에 저장장*/
    poststck->top = poststck->top->next;/*top 내리기*/
    free(temp);/*메모리 해제*/
    return val;/*삭제된 값 반환환*/
}

static void Pop(Node* sNode, Stack* stck)/*Pop연산산*/
{
    Node* temp;
    if (stck->top == NULL) return;/*stack이 비었으면 반환환*/
    sNode->exp_data = stck->top->exp_data;
    sNode->type = stck->top->type;
    sNode->line = stck->top->line;
    sNode->val = stck->top->val;/*제거될 값(top)을 sNode에 복사*/
    temp = stck->top;/*top을 temp에 저장*/
    stck->top = stck->top->next;/*top 내리기*/
    free(temp);/*메모리 해제제*/
}

static int isStackEmpty(OpStack* stck)/*Stack이 비었는지 검사  true=0*/
{
    return stck->top == 0;
}

static int Priotry(char operator)/*연산자 우선순위 결정정*/
{
    if ((operator=='+') || (operator=='-')) return 1;
    else if ((operator=='/') || (operator=='*')) return 2;
    return 0;
}

int main(int argc, char** argv)
{
    char line[4096];
    char dummy[4096];
    char lineyedek[4096];
    char postfix[4096];
    char* firstword;

    int val1;
    int val2;

    int LastExpReturn = 0;/*가장 최근에 평가된 표현식식*/
    int LastFunctionReturn = -999;
    int CalingFunctionArgVal = 0;/*호출하는 함수 값*/

    Node tempNode;

    OpStack* MathStack = (OpStack*)malloc(sizeof(OpStack));
    FILE* filePtr;
    PostfixStack* CalcStack = (PostfixStack*)malloc(sizeof(PostfixStack));
    int resultVal = 0;
    Stack* STACK = (Stack*)malloc(sizeof(Stack));

    int curLine = 0;
    int foundMain = 0;
    int WillBreak = 0;
    /*각 변수들 값 설정, Stack 동적 메모리 할당당*/
    if (!MathStack || !CalcStack || !STACK) {
        printf("Memory alloc failed\n");
        return 1;
    }/*메모리 할당 실패 시 에러문문 출력*/
    MathStack->top = NULL;
    CalcStack->top = NULL;
    STACK->top = NULL;/*Stack들의 top 초기화화*/

    CLEAR();/*커널 클리어어*/

    if (argc != 2)
    {
        printf("Incorrect arguments!\n");
        printf("Usage: %s <inputfile.spl>", argv[0]);
        return 1;
    }/*argc !=2이면 에러문 출력력*/

    filePtr = fopen(argv[1], "r");/*파일 읽기 모드드*/
    if (filePtr == NULL)
    {
        printf("Can't open %s. Check the file please", argv[1]);
        return 2;
    }/*파일이 비었으면 에러문 출력력*/

    while (fgets(line, 4096, filePtr))/*파일 처리리*/
    {
        int k = 0;

        while (line[k] != '\0')
        {
            if (line[k] == '\t') line[k] = ' ';
            k++;
        }/*파일 읽기기*/

        rstrip(line);/*파일 내용 줄이기기*/
        strcpy(lineyedek, line);/*파일 내용 lineyedek에 복사사*/

        curLine++;/*현재 코드라인 ++*/
        tempNode.val = -999;
        tempNode.exp_data = ' ';
        tempNode.line = -999;
        tempNode.type = -999;/*tempNode 값 할당당*/

        if (my_stricmp("begin", line) == 0)/*line 문자열이 'begin'이면면*/
        {
            if (foundMain)/*0*/
            {
                tempNode.type = 4;/*begin=4*/
                STACK = Push(tempNode, STACK);/*STACK에 tempNode삽입입*/
            }
        }
        else if (my_stricmp("end", line) == 0)/*line 문자열이 'end'면면*/
        {
            if (foundMain)/*0*/
            {
                int sline;
                tempNode.type = 5;/*end=5*/
                STACK = Push(tempNode, STACK);/*STACK에 tempNode 삽입입*/

                sline = GetLastFunctionCall(STACK);/*sline에 STACK의 줄번호 삽입입*/
                if (sline == 0)/*STACK의 줄번호가 0이면면*/
                {
                    printf("Output=%d", LastExpReturn);/*가장 최근에 평가된 표현식 반환환*/
                }
                else
                {
                    int j;
                    int foundCall = 0;
                    LastFunctionReturn = LastExpReturn;

                    fclose(filePtr);/*파일 닫기기*/
                    filePtr = fopen(argv[1], "r");/*파일 읽기 모드*/
                    curLine = 0;
                    for (j = 1; j < sline; j++)
                    {
                        fgets(dummy, 4096, filePtr);/*STACK 줄번호만큼 파일 읽기*/
                        curLine++;
                    }

                    while (foundCall == 0)/*함수 호출을 발견 못할 때까지지*/
                    {
                        Pop(&tempNode, STACK);/*STACK값 삭제제*/
                        if (tempNode.type == 3) foundCall = 1;/*functioncall을 발견하면 foundCall 값=1*/
                    }
                }
            }
        }
        else/*line 문자열이 'begin' , 'end'도 아니면*/
        {
            firstword = strtok(line, " ");/*line을 "  "단위로 분리리 >> firstword에 첫번째 단어 저장장*/
            if (!firstword) continue;/*line이 공백이면 넘어감감*/

            if (my_stricmp("int", firstword) == 0)/*firstword가 'int'이면*/
            {
                if (foundMain)/*0*/
                {
                    tempNode.type = 1;/*var=1*/
                    firstword = strtok(NULL, " ");/*line분리 작업 재개개*/
                    if (!firstword) continue;
                    tempNode.exp_data = firstword[0];/*분리된 값 exp_data에 저장장*/

                    firstword = strtok(NULL, " ");/*line분리 작업 재개*/
                    if (!firstword) continue;

                    if (my_stricmp("=", firstword) == 0)/*firstword가 '='이면면*/
                    {
                        firstword = strtok(NULL, " ");/*line 분리 작업 재개*/
                        if (!firstword) continue;
                    }

                    tempNode.val = atoi(firstword);/*fistword를 정수값으로 변환 후 val에 저장*/
                    tempNode.line = 0;
                    STACK = Push(tempNode, STACK);/*STACK에 tempNode 삽입입*/
                }
            }
            else if (my_stricmp("function", firstword) == 0)/*firstword가 'function'이면면*/
            {
                firstword = strtok(NULL, " ");/*line분리 작업 재개*/
                if (!firstword) continue;

                tempNode.type = 2;/*function=2*/
                tempNode.exp_data = firstword[0];/*firstword를 exp_data에 저장장*/
                tempNode.line = curLine;/*현재줄번호를 line에 저장장*/
                tempNode.val = 0;
                STACK = Push(tempNode, STACK);/*STACK에 tempNode 삽입입*/

                if (firstword[0] == 'm' && firstword[1] == 'a' && firstword[2] == 'i' && firstword[3] == 'n')
                {
                    foundMain = 1;
                }/*firstword가 'main'이면 foundMain=1*/
                else
                {
                    if (foundMain)/*0*/
                    {
                        firstword = strtok(NULL, " ");/*line 분리 작업 재개개*/
                        if (!firstword) continue;
                        tempNode.type = 1;/*var=1*/
                        tempNode.exp_data = firstword[0];/*firstword값 exp_data에 삽입*/
                        tempNode.val = CalingFunctionArgVal;/*호출하는 함수 값을 val에 저장장*/
                        tempNode.line = 0;
                        STACK = Push(tempNode, STACK);/*STACK에 tempNode값 저장장*/
                    }
                }
            }
            else if (firstword[0] == '(')/*firstword가 '("이면"*/
            {
                if (foundMain)/*0*/
                {
                    int i = 0;
                    int y = 0;

                    MathStack->top = NULL;/*MathStack의 top 초기화화*/

                    while (lineyedek[i] != '\0')/*lineyedek은 line 복사본, lineyedek이 공백문자가 아니라면면*/
                    {
                        if (isdigit((unsigned char)lineyedek[i]))/*10진수가 맞으면면*/
                        {
                            postfix[y] = lineyedek[i];/*postfix에 lineyedek값 삽입*/
                            y++;
                        }
                        else if (lineyedek[i] == ')')/*lineyedek가 '('이면면*/
                        {
                            if (!isStackEmpty(MathStack))/*MathStack이 안비어있으면면*/
                            {
                                postfix[y] = PopOp(MathStack);/*postfix에 MathStack 삭제 값 삽입입*/
                                y++;
                            }
                        }
                        else if (lineyedek[i] == '+' || lineyedek[i] == '-' || lineyedek[i] == '*' || lineyedek[i] == '/')/*lineyedek이 사칙연산이면면*/
                        {
                            if (isStackEmpty(MathStack))/*MathStack이 비어있으면면*/
                            {
                                MathStack = PushOp(lineyedek[i], MathStack);/*MathStack에 lineyedek값 삽입 후 MatchStack 업데이트트*/
                            }
                            else/*MathStack이 차있으면면*/
                            {
                                if (Priotry(lineyedek[i]) <= Priotry(MathStack->top->op))/*MathStack의 top의 우선순위가 lineyedek 값보다 크거나 같으면면*/
                                {
                                    postfix[y] = PopOp(MathStack);/*MathStack삭제 값을 postfix에 삽입*/
                                    y++;
                                    MathStack = PushOp(lineyedek[i], MathStack);/*MatchStack에 lineyedek값 삽입 후 MathStack 업데이트트*/
                                }
                                else/*MathStack의 top의 우선순위가 lineyedek 값보다 작으면면*/
                                {
                                    MathStack = PushOp(lineyedek[i], MathStack);/*MathStack에 lineyedek값 삽입 후 MathStack 업데이트트*/
                                }
                            }
                        }
                        else if (isalpha((unsigned char)lineyedek[i]) > 0)/*lineyedek의 문자가 a to z이라면*/
                        {
                            int codeline = 0;
                            int dummyint = 0;
                            int retVal = GetVal(lineyedek[i], &codeline, STACK);/*STACK에서 lineyedek[i]찾기,찾은 codeline 저장,retVal에 저장*/

                            if ((retVal != -1) && (retVal != -999))/*retVal값이 -999 or -1 둘다 아니면면*/
                            {
                                postfix[y] = (char)(retVal + 48);/*retVal+48 을 postfix에 삽입입*/
                                y++;
                            }
                            else/*retVal=-999 && -1이면면*/
                            {
                                if (LastFunctionReturn == -999)/*-999일시시*/
                                {
                                    int j;
                                    tempNode.type = 3;/*function call = 3*/
                                    tempNode.line = curLine;
                                    STACK = Push(tempNode, STACK);/*STACK에 tempNode 삽입, STACK 업데이트트*/

                                    CalingFunctionArgVal = GetVal(lineyedek[i + 2], &dummyint, STACK);
                                    /*호출된 함수 값은 STACK에서 lineyedek[i+2]를 찾았냐에 달림 var or -1 or -999*/
                                    fclose(filePtr);/*파일 닫기기*/
                                    filePtr = fopen(argv[1], "r");/*파일 읽기 모드드*/
                                    curLine = 0;

                                    for (j = 1; j < codeline; j++)
                                    {
                                        fgets(dummy, 4096, filePtr);
                                        curLine++;
                                    }/*codeline까지 파일 읽기기*/

                                    WillBreak = 1;
                                    break;/*완료시 탈출출*/
                                }
                                else/*retVal == -1이면*/
                                {
                                    postfix[y] = (char)(LastFunctionReturn + 48);/*LastFunction+48값을 postfix에 삽입입*/
                                    y++;
                                    i = i + 3;
                                    LastFunctionReturn = -999;
                                }
                            }
                        }
                        i++;/*lineyedek ++ 계속 검사사*/
                    }

                    if (WillBreak == 0)
                    {
                        while (!isStackEmpty(MathStack))/*MatchStack이 비어있지않으면면*/
                        {
                            postfix[y] = PopOp(MathStack);/*MathStack 삭제 값 postfix에 삽입입*/
                            y++;
                        }

                        postfix[y] = '\0';/*공백문자 삽입입*/

                        i = 0;
                        CalcStack->top = NULL;/*CalcStack 의 top 초기화화*/
                        while (postfix[i] != '\0')/*postfix에서 공백문자 발견되지 않았으면면*/
                        {
                            if (isdigit((unsigned char)postfix[i]))/*10진수이라면면*/
                            {
                                CalcStack = PushPostfix(postfix[i] - '0', CalcStack);/*CalcStack에 postfix값 삽입 후 업데이트트*/
                            }
                            else if (postfix[i] == '+' || postfix[i] == '-' || postfix[i] == '*' || postfix[i] == '/')/*postfix가 사칙연산이면면*/
                            {
                                val1 = PopPostfix(CalcStack);
                                val2 = PopPostfix(CalcStack);/*CalcStack 삭제 값을 val1,val2에 저장장*/

                                switch (postfix[i])
                                {
                                case '+': resultVal = val2 + val1; break;
                                case '-': resultVal = val2 - val1; break;
                                case '/': resultVal = val2 / val1; break;
                                case '*': resultVal = val2 * val1; break;
                                }/*사칙연산 계산산*/
                                CalcStack = PushPostfix(resultVal, CalcStack);/*CalcStack값에 resultVal(사칙연산값)삽입 후 업데이트트*/
                            }
                            i++;/*postfix 검사 재개개 */
                        }

                        LastExpReturn = CalcStack->top->val;/*가장 최근에 평가된 값을 CalcStack의 top의 val값으로 업데이트트*/
                    }
                    WillBreak = 0;
                }
            }
        }
    }

    fclose(filePtr);/*파일 닫기기*/
    STACK = FreeAll(STACK);/*STACK 비우기기*/

    printf("\nPress a key to exit...");
    getch();
    return 0;
}

static Stack* FreeAll(Stack* stck)/*FreeAll 연산(Stack 비우기기)*/
{
    Node* head = stck->top;/*top을 head에 삽입*/
    while (head) {/*head가 존재할때까지*/
        Node* temp = head;
        head = head->next;/*head값을 위로 올림*/
        free(temp);/*전 head값 삭제*/
    }
    stck->top = NULL;
    return NULL;
}

static int GetLastFunctionCall(Stack* stck)/*GetLastFunctionCall 연산(stack의 코드가 몇번쨰 line인지 반환환)*/
{
    Node* head = stck->top;/*top을 head에 삽입*/
    while (head) {/*head가 존재할때까지지*/
        if (head->type == 3) return head->line;/*stck의 head가 3(functioncall)을 가리키면 stck의 line(코드의 몇번째 줄?)을 반환*/
        head = head->next;/*head값을 위로 올림림*/
    }
    return 0;
}

static int GetVal(char exp_name, int* line, Stack* stck)/*GetVal 연산산(exp_name 찾기,반환환)*/
{
    Node* head;
    *line = 0;
    if (stck->top == NULL) return -999;/*stack이 비었으면 -999반환환*/
    head = stck->top;/*top을 head에 삽입입*/
    while (head) {/*head가 존재할때까지지*/
        if (head->exp_data == exp_name)
        {
            if (head->type == 1) return head->val;/*exp_name이 var면 val반환환*/
            else if (head->type == 2) { *line = head->line; return -1; }/*function이면 -1반환 *line은 head의 line을 가리킴*/
        }
        head = head->next;/*head 값 올리기기*/
    }
    return -999;
}

static int my_stricmp(const char* a, const char* b)/*문자가 같은지 검사 true =0*/
{
    unsigned char ca, cb;
    while (*a || *b) {
        ca = (unsigned char)tolower((unsigned char)*a);/* a를 소문자로 변환환*/
        cb = (unsigned char)tolower((unsigned char)*b);/* b를 소문자로 변환환*/
        if (ca != cb) return (int)ca - (int)cb;/*변환값값이 다르면 값 반환*/
        if (*a) a++;
        if (*b) b++;/*포인터를 더하며 검사사*/
    }
    return 0;
}

static void rstrip(char* s)/*문자열 단축축*/
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ')) s[--n] = '\0';
}
