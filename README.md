# MMT 

# Build command
- Client: g++ -std=c++17 main.cpp server_socket.cpp client_socket.cpp -o client -lws2_32 -lgdiplus
- Server: g++ -std=c++17 main.cpp server_socket.cpp client_socket.cpp -o server -lws2_32 -lgdiplus
# Run command
- Client: \.client client
- Server: .\server server