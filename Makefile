CC = g++ -pthread
OUT_DIR = output
MKDIR_P = mkdir -p
MSSV = ProxyServer

all: directories program

clean: 
	rm -f $@ $(OUT_DIR)/*.o 

	
directories: ${OUT_DIR}
${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}


program: socket.o helper.o util.o ProxyServer.o ClientConnection.o ServicePools.o
	$(CC) -std=c++17 ${OUT_DIR}/*.o -o $(MSSV) -lstdc++fs

socket.o: Socket.cpp
	$(CC) -c Socket.cpp -o ${OUT_DIR}/socket.o

helper.o: helper.cpp Socket.cpp util.cpp
	$(CC) -c helper.cpp -o ${OUT_DIR}/helper.o

util.o: util.cpp
	$(CC) -c util.cpp -o ${OUT_DIR}/util.o

ProxyServer.o: ProxyServer.cpp util.cpp Socket.cpp helper.cpp
	$(CC) -c ProxyServer.cpp -o $(OUT_DIR)/http.o

ClientConnection.o: Socket.cpp
	$(CC) -c ClientConnection.cpp -o $(OUT_DIR)/ClientConnection.o
ServicePools.o: ClientConnection.cpp
	$(CC) -c ServicePools.cpp -o $(OUT_DIR)/ServicePools.o
