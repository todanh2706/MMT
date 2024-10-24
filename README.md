# MMT 

## Build command
- Client: g++ -std=c++17 main.cpp server_socket.cpp client_socket.cpp -o client -lws2_32 -lgdi32 -lgdiplus -lole32 -lcurl
- Server: g++ -std=c++17 main.cpp server_socket.cpp client_socket.cpp -o server -lws2_32 -lgdi32 -lgdiplus -lole32
### credential.json format
```
{
	"accessToken" : "abc",
	"clientID" : "895952228884-9c0ic1rovu7a94imbkf5iqglbjo852eo.apps.googleusercontent.com",
	"clientSecret" : "abc",
	"refreshToken" : "abc"
}



## Run command
- Client: \.client client
- Server: .\server server
