#ifndef MACRO_H
#define MACRO_H

//file di macro usate a lezione e in esercitazioni

//scall per valore standard error=-1
#define SCALL_ERROR -1
#define SCALL(r, c, e) do { if((r = c) == SCALL_ERROR) { perror(e); exit(EXIT_FAILURE); } } while(0)

//read per return: 0 se EOF, -1 error, per f che return int/ssize_t/read()/...
#define SCALLREAD(r, loop_cond_op, read_loop_op,e) do {while((r=loop_cond_op)>0) {read_loop_op;} if(r==SCALL_ERROR) {perror(e); exit(EXIT_FAILURE);}} while(0)

//SCALL per chiamate senza assegnamento
#define SCALL_VOID(c, e) do { if((c) == SCALL_ERROR) {perror(e); exit(EXIT_FAILURE); } } while(0)
//SCALL per pointer (errore: (void *) -1)
#define SCALL_PTR(r, c, e) do { if((r = c) == (void *) -1) { perror(e); exit(EXIT_FAILURE); } } while(0)

//SNCALL=system null call (error: return NULL), per funzioni che return puntatori
#define SNCALL(r, c, e) do {if((r = c) == NULL) {perror(e); exit(EXIT_FAILURE);}} while(0)
//gestione loop lettura
#define SNCALLREAD(r, loop_cond_op, read_loop_op,e) do {while((r=loop_cond_op)>0) {read_loop_op;} if(errno==SCALL_ERROR) {perror(e); exit(EXIT_FAILURE);}} while(0)
//opendir e readdir return NULL if error, closedir e stat return -1

//per le fork, se pid=0 esegue child
#define PARENT_OR_CHILD(pid,f_parent,f_child) do { if(pid == 0) { f_child; } else { f_parent; } } while(0)

#endif