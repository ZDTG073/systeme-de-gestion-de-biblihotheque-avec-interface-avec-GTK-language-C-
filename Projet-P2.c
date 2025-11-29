#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <C:\Users\Dell\Dev\ACAD-B-1-4\Projet-P2\LibStackQueue.c>
#include <gtk/gtk.h>
#include <string.h>

static char UserName[30];  // User Name
static GtkWidget *entry1;  // Text field : Title
static GtkWidget *entry2;  // Text field : Author
static GtkWidget *list_box1;  // List box for books
static GtkWidget *list_box2;  // List box for borrowed books

static void on_window_destroy(GtkWidget *widget, gpointer data) { // Close window
    gtk_main_quit();
}

static void Update(Container *L) {
    if (!L) return;

    // Update list_box1 (Library)
    GList *children = gtk_container_get_children(GTK_CONTAINER(list_box1)); // delete all element
    for (GList *i = children; i != NULL; i = g_list_next(i)) {
        gtk_widget_destroy(GTK_WIDGET(i->data));
    }
    g_list_free(children);

    Plist *P = L->Lib;
    while (P != NULL) { // Refill all the element
        char book_info[200];
        snprintf(book_info, sizeof(book_info),
                 "Book: %s | Author: %s | ID: %d | Available: %s | Quantity: %d",
                 P->Data.Title, P->Data.Author, P->Data.ID,
                 P->Data.Status ? "Yes" : "No", P->Qte);

        GtkWidget *label = gtk_label_new(book_info);
        gtk_list_box_insert(GTK_LIST_BOX(list_box1), label, -1);
        P = P->Next;
    }

    // Update List box2 (Borrowed Book)
    GList *children2 = gtk_container_get_children(GTK_CONTAINER(list_box2));
    for (GList *i = children2; i != NULL; i = g_list_next(i)) {
        gtk_widget_destroy(GTK_WIDGET(i->data));
    }
    g_list_free(children2);

    if (L->BI) {
        Book b;
        Stack temp;
        InitStack(&temp);

        while (!isSEmpty(*(L->BI))) {
            Pop(L->BI, &b);
            char book_info[200];
            snprintf(book_info, sizeof(book_info),
                     "Book: %s | Author: %s | ID: %d",
                     b.Title, b.Author, b.ID);

            GtkWidget *label = gtk_label_new(book_info);
            gtk_list_box_insert(GTK_LIST_BOX(list_box2), label, -1);
            Push(&temp, b);
        }
        // Rebuild Stack
        while (!isSEmpty(temp)) {
            Pop(&temp, &b);
            Push(L->BI, b);
        }
    }

    gtk_widget_show_all(list_box1);
    gtk_widget_show_all(list_box2);
}

void ProcessRequest(GtkWidget *widget, gpointer data) {
Container **L = (Container **)data;

    
    if (!L || !(*L) || !(*L)->QU || !(*L)->Lib || !(*L)->SB) {
        g_print("Erreur : ProcessRequest - data\n");
        return;
    }

    Queue *Q = (*L)->QU;   
    Stack *S = (*L)->BI;   
    Plist *Lib = (*L)->Lib; 
    User X;
    Book *requestedBook = NULL;

    
    if (isQEmpty(*Q)) {
        g_print("No Request.\n");
        return;
    }

    Dequeue(Q, &X);
    
    Plist *current = Lib;
    while (current != NULL) {
        if (current->Data.ID == X.Book_ID) { // find the book
            
            if (current->Data.Status) {     // if available
                
                Push(S, current->Data); // add it to the BI 
                current->Qte--; 
                if (current->Qte == 0) {
                    current->Data.Status = false;
                }

                Update(*L);
                return;
            } else { //if not available

                Update(*L);
                Enqueue(Q, X); 
                return;
            }
        }
        current = current->Next;
    }

    Update(*L);


    g_print("The book with ID %d doesn't exist.\n", X.Book_ID); // Book not found
    Enqueue(Q, X); 
}


int GenerateUniqueID(Plist *L) {
    Plist *temp = L;
    bool isUnique;
    int num;

    do {
        num = rand() % 10000;
        isUnique = true;

        while (temp != NULL && isUnique) {
            if (temp->Data.ID == num) {
                isUnique = false;
            }
            temp = temp->Next;
        }
    } while (!isUnique);

    return num;
}

void AddBook(GtkWidget *widget, gpointer data) {
    Container **L = (Container **)data;
    if (!(*L)) return;

    Plist *H = (*L)->Lib;
    Plist *newBook = malloc(sizeof(Plist));
    if (!newBook) {
        fprintf(stderr, "Memory allocation error for new book.\n");
        return;
    }

    const char *T = gtk_entry_get_text(GTK_ENTRY(entry1));
    const char *A = gtk_entry_get_text(GTK_ENTRY(entry2));

    if (!T || strlen(T) == 0) { // Empty Text Field (title)
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)), // Pop up 
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Add Book Title");
        gtk_entry_set_text(GTK_ENTRY(entry1), ""); // Reset Texte Field
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        free(newBook);
        return;
    }

    if (!A || strlen(A) == 0) { // Empty Text Field (author)
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Add Book Author");
        gtk_entry_set_text(GTK_ENTRY(entry2), ""); // Reset Texte Field
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        free(newBook);
        return;
    }

    newBook->Data.ID = GenerateUniqueID((*L)->Lib); // ID
    snprintf(newBook->Data.Title, sizeof(newBook->Data.Title), "%s", T); //title
    snprintf(newBook->Data.Author, sizeof(newBook->Data.Author), "%s", A); //author
    newBook->Data.Status = true; //
    newBook->Qte = rand() % 5 + 1; 
    newBook->Next = NULL;
    newBook->Prev = NULL;

    if ((*L)->Lib == NULL) {
        (*L)->Lib = newBook;
    } else {
        do{
            if (strcmp(T, H->Data.Title) == 0 && strcmp(A, H->Data.Author) == 0){ // Book Already Exist
                H->Qte++;
                if (!H->Data.Status) {
                    H->Data.Status = true;
                }
                free(newBook);
                break;
            }
            H = H->Next;
        }while (H->Next != NULL); 

        H->Next = newBook;
        newBook->Prev = H;
    }

    gtk_entry_set_text(GTK_ENTRY(entry1), ""); // Reset Texte Field
    gtk_entry_set_text(GTK_ENTRY(entry2), ""); // Reset Texte Field


    Update(*L);
    g_print("Add Realesed\n");
    return;

}

void BorrowBook(GtkWidget *widget, gpointer data) {
    Container **L = (Container **)data;
    if (!(*L)) return;

    Plist *P = (*L)->Lib;
    Stack *S = (*L)->BI;
    Queue *Q = (*L)->QU;

    User Request;
    if(isSEmpty(*S)){InitStack(S);}

    //selected Book
    GtkListBoxRow *selected_row = gtk_list_box_get_selected_row(GTK_LIST_BOX(list_box1));
    if (!selected_row) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "No book selected.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    GtkWidget *label = gtk_bin_get_child(GTK_BIN(selected_row));
    const char *book_info = gtk_label_get_text(GTK_LABEL(label));

    // Extract th Book ID
    int bookID;
    sscanf(book_info, "Book: %*[^|]| Author: %*[^|]| ID: %d", &bookID);

    while (P != NULL) {
        if (P->Data.ID == bookID) { // Found book in the lib

            if (P->Data.Status) { // Book Available
                P->Qte--;
                Push(S, P->Data); // Push it in the BI
                if (P->Qte == 0) {
                    P->Data.Status = false;
                }
            } else { // Book Not Availble
                GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                           GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Book not available. You Will be added to The Request Queue");
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);

                // Enqueue in the Request Queue 
                snprintf(Request.Name,sizeof(Request.Name),"%s",UserName);
                g_print("%s Added\n",Request.Name);
                Request.Book_ID = bookID;
                g_print("%d Added\n",Request.Book_ID);
                Request.ID = GenerateUniqueID((*L)->Lib);
                g_print("%d Added\n",Request.ID);
                Enqueue(Q,Request);
            }
            break;
        }
        P = P->Next;
    }


    Update(*L);
    g_print("Borrow Realesed\n");

}

//3.
void ReturnBook(GtkWidget *widget, gpointer data){

    Container **L = (Container **)data;
    Plist *F = (*L)->Lib;
    Stack *S = (*L)->BI; Stack temp; InitStack(&temp);
    Book b;

    GtkListBoxRow *selected_row = gtk_list_box_get_selected_row(GTK_LIST_BOX(list_box2));
    if (!selected_row) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "No book selected.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    GtkWidget *label = gtk_bin_get_child(GTK_BIN(selected_row));
    const char *book_info = gtk_label_get_text(GTK_LABEL(label));

    int bookID;
    sscanf(book_info, "Book: %*[^|]| Author: %*[^|]| ID: %d", &bookID); // Traverse the string and ignore the text after book : and author : / taking value of ID

    while(!isSEmpty(*S)){

        Pop(S,&b);
        if(b.ID == bookID){
            Push((*L)->SB, b);

            if(F == NULL){
                return;
            }
            while(F != NULL && F->Data.ID != b.ID){
                F = F->Next;
            }
            if(F == NULL){
                return;
            }
            F->Data.Status = true;
            F->Qte++;
            
            break;
        }
        Push(&temp,b);   
    }

    while(!isSEmpty(*S)){
        Pop(S,&b);Push(&temp,b);
    }

    while(!isSEmpty(temp)){
        Pop(&temp,&b);
        Push((*L)->BI,b);
    }

    Update(*L);
    g_print("Return Realesed\n");

}

void DisplayQueue(GtkWidget *widget, gpointer data){ // Same As Display Stack
    
    if (!data) {
        g_print("Erreur : DisplayQ - data.\n");
        return;
    }

    Container **L = (Container **)data;
    if (!L || !(*L) || !(*L)->QU) return;

    Queue *Q = (*L)->QU;
    Queue temp;
    InitQueue(&temp); 


    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Queue Element",
        NULL,
        GTK_DIALOG_MODAL,
        ("Close"),
        GTK_RESPONSE_CLOSE,
        NULL
    );

    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 250);


    GtkWidget *listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), listbox);


    if (isQEmpty(*Q)) { 
        GtkWidget *label = gtk_label_new("The Queue Is Empty."); // Empty Queue
        gtk_list_box_insert(GTK_LIST_BOX(listbox), label, -1);
    } else {

        User t;
        while (!isQEmpty(*Q)){ // Display

            Dequeue(Q, &t);
            char user_info[200];
            snprintf(user_info, sizeof(user_info), "Name: %s | Book-ID: %d | ID: %d", t.Name, t.Book_ID, t.ID);
            GtkWidget *label = gtk_label_new(user_info);
            gtk_list_box_insert(GTK_LIST_BOX(listbox), label, -1);

            Enqueue(&temp, t);
        }


        while (!isQEmpty(temp)){ //Rebuild
            Dequeue(&temp, &t);
            Enqueue(Q, t);
        }
    }


    gtk_widget_show_all(dialog);
    
}



void DisplayStack(GtkWidget *widget, gpointer data) {
    Container **L = (Container **)data;

    GtkWidget *dialog, *listbox, *close_button;
    Stack *S = (*L)->SB; 
    Book t;
    Stack temp;
    InitStack(&temp);


    dialog = gtk_dialog_new_with_buttons("Recently Returned Book", NULL, GTK_DIALOG_MODAL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 250);


    listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), listbox);


    while (!isSEmpty(*S)) { //Display the Stack
        Pop(S, &t);
    
        char book_info[200];
        snprintf(book_info, sizeof(book_info), "Book: %s | Author: %s | ID: %d", t.Title, t.Author, t.ID);
        GtkWidget *label = gtk_label_new(book_info);
        gtk_list_box_insert(GTK_LIST_BOX(listbox), label, -1);

        Push(&temp, t);
    }


    while (!isSEmpty(temp)) { // Rebuild
        Pop(&temp, &t);
        Push(S, t);
    }


    gtk_widget_show_all(dialog);
}


void SearchBook(GtkWidget *widget, gpointer data){ // Same as Adding Book

    Container **L = (Container **)data;
    if (!(*L)) return;
 
    Plist *P = (*L)->Lib;

    const char *T = gtk_entry_get_text(GTK_ENTRY(entry1));

    if (!T || strlen(T) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Add Book Title");
        gtk_entry_set_text(GTK_ENTRY(entry1), ""); // Reset Texte Field
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    while(P != NULL){
        if(strcmp(P->Data.Title,T) == 0){ // Book Found

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s Found, Actually %s",P->Data.Title,P->Data.Status ? "Available" : "Unaivalable");
        gtk_entry_set_text(GTK_ENTRY(entry1), ""); // Reset Texte Field
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        g_print("Search Realesed \n");
        return;
        }
        P = P->Next;
    }
    if(P == NULL){ // Book Not Found
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s Doesn't Exist :/");
        gtk_entry_set_text(GTK_ENTRY(entry1), ""); // Reset Texte Field
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);   
        g_print("Search Realesed \n");
        return;
    }

}

void ConfirmUserNameB(GtkWidget *button, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(data);
    const char *name = gtk_entry_get_text(GTK_ENTRY(entry));

    if (g_strcmp0(name, "") != 0) {
        snprintf(UserName, sizeof(UserName), "%s", name);
        GtkWidget *dialog = gtk_widget_get_toplevel(entry);
        gtk_widget_destroy(dialog);
    }
}

void GetUserName(GtkWidget *parent) { // Pop UP for User Name
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Enter Your User Name",
                                                    GTK_WINDOW(parent),
                                                    GTK_DIALOG_MODAL, NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 100);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Name");

    GtkWidget *button = gtk_button_new_with_label("Confirm");
    gtk_box_pack_start(GTK_BOX(content_area), entry, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(content_area), button, FALSE, FALSE, 5);

    g_signal_connect(button, "clicked", G_CALLBACK(ConfirmUserNameB), entry);

    gtk_widget_show_all(dialog);
}

int main(int argc, char *argv[]) {

    // Container Initialisation
    Container *M = malloc(sizeof(Container));
    if (!M) {
        fprintf(stderr, "Memory allocation error for Container.\n");
        return 1;
    }

    M->Lib = NULL;
    M->BI = malloc(sizeof(Stack));
    M->SB = malloc(sizeof(Stack));
    M->QU = malloc(sizeof(Queue));

    if (!M->BI || !M->SB || !M->QU) {
        fprintf(stderr, "Error : Container-main.\n");
        free(M);
        return 1;
    }

    InitStack(M->BI);
    InitStack(M->SB);
    InitQueue(M->QU);

    // GTK initialisation
    gtk_init(&argc, &argv);

    // Window Creation
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Library Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 1000);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    entry1 = gtk_entry_new(); //creating the text field
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry1), "Book Title"); //placeholder
    gtk_box_pack_start(GTK_BOX(vbox), entry1, FALSE, FALSE, 5); // Adding the text field to the vertical box

    entry2 = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry2), "Book Author");
    gtk_box_pack_start(GTK_BOX(vbox), entry2, FALSE, FALSE, 5);

    GtkWidget *hboxB = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    // Button
    GtkWidget *button_add = gtk_button_new_with_label("Add Book");
    GtkWidget *button_borrow = gtk_button_new_with_label("Borrow Book");
    GtkWidget *button_Return = gtk_button_new_with_label("Return Book");
    GtkWidget *button_Search = gtk_button_new_with_label("Search Book");
    GtkWidget *button_PR = gtk_button_new_with_label("Process Borrow Requests");
    GtkWidget *button_DisplayStack = gtk_button_new_with_label("Display Recently Returned Book");
    GtkWidget *button_DisplayQueue = gtk_button_new_with_label("Display Borrow Queue Request");


    // adding the button to the box
    gtk_box_pack_start(GTK_BOX(hboxB), button_add, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hboxB), button_borrow, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hboxB), button_Return, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hboxB), button_Search, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hboxB), button_PR, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hboxB), button_DisplayStack, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hboxB), button_DisplayQueue, TRUE, TRUE, 5);


    gtk_box_pack_start(GTK_BOX(vbox), hboxB, FALSE, FALSE, 5);

    list_box1 = gtk_list_box_new(); // lib
    gtk_box_pack_start(GTK_BOX(vbox), list_box1, TRUE, TRUE, 5);

    list_box2 = gtk_list_box_new(); //BI
    gtk_box_pack_start(GTK_BOX(vbox), list_box2, TRUE, TRUE, 5);

    // Linking buttons with the function
    g_signal_connect(button_add, "clicked", G_CALLBACK(AddBook), &M);
    g_signal_connect(button_borrow, "clicked", G_CALLBACK(BorrowBook), &M);
    g_signal_connect(button_Return, "clicked", G_CALLBACK(ReturnBook), &M);
    g_signal_connect(button_Search, "clicked", G_CALLBACK(SearchBook), &M);
    g_signal_connect(button_PR, "clicked", G_CALLBACK(ProcessRequest), &M);
    g_signal_connect(button_DisplayStack, "clicked", G_CALLBACK(DisplayStack), &M);
    g_signal_connect(button_DisplayQueue, "clicked", G_CALLBACK(DisplayQueue), &M);


    GetUserName(GTK_WIDGET(window));

    gtk_widget_show_all(window);
    gtk_main();

    free(M->BI);
    free(M->SB);
    free(M->QU);
    free(M);

    return 0;
}
/*
Commande :
gcc $(pkg-config --cflags gtk+-3.0) -o main C:/Users/Dell/Dev/ACAD-B-1-4/Projet-P2/Projet-P2.c $(pkg-config --libs gtk+-3.0)
*/
