#compilatore e opzioni
CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -g -D_POSIX_C_SOURCE=200809L #funzioni POSIX
#librerie esterne per threads e message queue
LIBS = -lpthread -lrt

#file sorgenti .c
SRCS = main.c \
       parse_env.c \
       parse_rescuers.c \
       parse_emergency_types.c \
       emergency_id.c \
       logger.c \
       dispatcher.c \
       emergency_manager.c \
       emergency_thread.c \
       message_queue.c

#da SRCS .c a .o
OBJS = $(SRCS:.c=.o)
#eseguibile main
TARGET = main
#eseguibile client
CLIENT = client

# target che non corrispondono a file
.PHONY: all clean run client log valgrind

#compila tutto, main e client
all: $(TARGET) $(CLIENT)
#compila main con i file .o, $@ = target, $^ = tutte le dipendenze .o
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
#compila client
client: client.c
	$(CC) $(CFLAGS) -o $(CLIENT) client.c $(LIBS)
#compila ogni .c in .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
# Esegue target (main)
run: $(TARGET)
	./$(TARGET)
#mostra contenuto del log
log:
	@echo "Contenuto log.txt:"
	@cat log.txt
#rimuove eseguibili e log
clean:
	rm -f $(OBJS) $(TARGET) $(CLIENT) log.txt
#valgrind per rilevare memory leak
valgrind: $(TARGET)
	valgrind --leak-check=full ./$(TARGET)

