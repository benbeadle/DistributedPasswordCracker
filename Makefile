CC=g++
CFLAGS= -lrt -lstdc++ -I/usr/include -g -lprotobuf-c -L/usr/lib 
DEPS = lsp.h 
OBJ =  lsp.o lsp_server.o lsp_client.o queue.o lspmessage.pb-c.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)