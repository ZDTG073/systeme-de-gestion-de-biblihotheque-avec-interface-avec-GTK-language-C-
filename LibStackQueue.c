//#include <LibStackQueue.h>
#include <stdio.h>
#include <stdbool.h>

    typedef struct book
    {
        int ID;
        char Title[30];
        char Author[30];
        bool Status;
    }Book;

    typedef struct user
    {
        int ID;
        char Name[30];
        int Book_ID;
        
    }User;

    typedef struct plist
    {
        Book Data;
        int Qte;
        struct plist *Next;
        struct plist *Prev;
    }Plist;

///declaration of TypeElement
//typedef char TypeElm;

///declaration of a Stack
typedef struct Estack* Stack;
typedef struct Estack { Book Data; Stack Next;}SNode;

///declaration of a Queue
typedef struct Element* EQueue;
typedef struct Element { User  Data; EQueue Next;}QNode;
typedef struct {EQueue Head,Tail;} Queue;

    typedef struct container{

        Plist *Lib; // Main Librairy
        Stack *BI; // Book Inventory
        Stack *SB; // Recently returned Book
        Queue *QU; // Borrow Queue Request 
    }Container;

///Stacks operations
///procedure InitStack
void InitStack(Stack *P)
{*P=NULL;}

///procedure Push
void Push(Stack *S, Book X)
{Stack V;
 V=malloc(sizeof(SNode));
 V->Data = X;
 V->Next = *S;
 *S = V;
}

///procedure Pop
void Pop(Stack *S, Book *X)
{Stack V;
 *X = (*S)->Data;
 V = *S;
 *S = (*S)->Next;
 free(V);
}

///Function isSEmpty
int isSEmpty(Stack S)
{if (S==NULL) return 1;
 else return 0;
}

///Function Top
Book Top(Stack S)
{return S->Data;}


///Queues operations
///procedure InitQueue
void InitQueue(Queue *Q)
{(*Q).Head=NULL; (*Q).Tail=NULL;}

///procedure Enqueue
void Enqueue(Queue *Q, User X)
{EQueue V;
 V=malloc(sizeof(QNode));
 V->Data = X;
 V->Next=NULL;
 if ((*Q).Head==NULL) (*Q).Head=V; //the Queue is empty
 else (*Q).Tail->Next=V;
 (*Q).Tail=V;
}

///procedure Dequeue
void Dequeue(Queue *Q, User *X)
{EQueue V;
 V = (*Q).Head;
 *X = V->Data; //ou  *x=(*Q).Head->Data;

 if ((*Q).Head==(*Q).Tail) {(*Q).Head=NULL; (*Q).Tail=NULL;}
 else (*Q).Head=(*Q).Head->Next; // ou  (*Q).Head=V->Next;
 free(V);
}

///Function isQEmpty
int isQEmpty(Queue Q)
{if (Q.Head==NULL) return 1;
 else return 0;
}

///Function Head
User Head(Queue Q)
{return Q.Head->Data;}
