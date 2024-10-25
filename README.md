# MMT 


# Build command
- Client: g++ -std=c++17 main.cpp server_socket.cpp client_socket.cpp -o client -lws2_32 -lgdi32 -lgdiplus -lole32
- Server: g++ -std=c++17 main.cpp server_socket.cpp client_socket.cpp -o server -lws2_32 -lgdi32 -lgdiplus -lole32
# Run command
- Client: .\client client
- Server: .\server server
## Build command
- Client:
  ```g++ -std=c++17 *.cpp -o client -lws2_32 -lgdi32 -lgdiplus -lole32 -lcurl```
- Server:
  ```g++ -std=c++17 *.cpp -o server -lws2_32 -lgdi32 -lgdiplus -lole32 -lcurl```
### credential.json format
```
{
	"accessToken" : "ya29.a0AcM612witTMR5HHMWELhr0dLcE1s7CyThP2nErmkPO44sS4VH4COFCLkvqAWgqxFZgeJulT-V0L_aOrKH0uaqL7krq7cwehAwGNdW2-0vAqvr643iCrUFP9Fr0l-ScXYUl_b6bwotGJUHUM75OjFUrkappnj3bOrofxsxsViHwaCgYKAYgSARASFQHGX2Mizdz2G86W0-LuelGHa_eaJg0177",
	"clientID" : "895952228884-9c0ic1rovu7a94imbkf5iqglbjo852eo.apps.googleusercontent.com",
	"clientSecret" : "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",
	"refreshToken" : "1//0gfPWJb6Pda0-CgYIARAAGBASNwF-L9Ir-sjA1GFPj8V8gn1U4sI79rF_rjPP3u78_ThuFUVYEIFml3FWZAIixOFMyew0BqP0GhA"
}
```

