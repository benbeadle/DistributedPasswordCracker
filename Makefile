CC=g++
CFLAGS= -I -x.
DEPS = lsp.h 
OBJ =  lsp.o lsp_server.o queue.o lspmessage.pb-c.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

