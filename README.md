# Nodepp-HTTP-Socket
a simple chunk encoding websocket like implementation that does not require JS enabled.

## Client
```cpp
#include <nodepp/nodepp.h>
#include <hsocket/hs.h>

using namespace nodepp;

void onMain() {

    auto client = hs::client( "hs://localhost:8000" );

    client.onConnect([=]( hs_t cli ){

        cli.onClose([=](){ process::exit(); });

        cli.onData([=]( string_t data ){
            console::log( data );
        });

        console::log("Connected");
        cli.write("Hello World!");

    });
    
}
```

## Server
```cpp
#include <nodepp/nodepp.h>
#include <hsocket/hs.h>

using namespace nodepp;

void onMain() {

    auto server = hs::server();

    server.onConnect([=]( hs_t cli ){

        cli.onClose([=](){ process::exit(); });

        cli.onData([=]( string_t data ){
            console::log( data );
        });

        console::log("Connected");
        cli.write("Hello World!");

    });

    server.listen( "localhost", 8000, [=](...){
        console::log( "-> http://localhost:8000" );
    });
    
}
```
