#include <nodepp/nodepp.h>
#include <hsocket/hss.h>
#include <nodepp/fs.h>

using namespace nodepp;

void onMain() {


    auto ssl    = ssl_t(); auto cin = fs::std_input(); 
    auto client = hss::client( "hss://localhost:8000", &ssl );

    client.onConnect([=]( hss_t cli ){

        cli.onClose([=](){ process::exit(); });

        cin.onData([=]( string_t data ){
            cli.write( data );
        });

        cli.onData([=]( string_t data ){
            console::log( data );
        });

        console::log("Connected");

    });
    
    stream::pipe( cin );

}