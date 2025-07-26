/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOfficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_HTTP_SOCKET
#define NODEPP_HTTP_SOCKET

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/http.h>
#include "generator.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class hs_t : public socket_t {
protected:

    struct NODE {
        generator::hs::write write;
        generator::hs::read  read ;
    };  ptr_t<NODE> hs;

public:

    template< class... T > 
    hs_t( const T&... args ) noexcept : socket_t( args... ), hs( new NODE() ){}

    virtual int _write( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; }
        while( hs->write( this, bf, sx )==1 ){ return -2; }
        return hs->write.data==0 ? -1 : hs->write.data;
    }

    virtual int _read ( char* bf, const ulong& sx ) const noexcept override {
        if( is_closed() ){ return -1; } if( sx==0 ){ return 0; }
        while( hs->read( this, bf, sx )==1 ){ return -2; }
        return hs->read.data==0 ? -1 : hs->read.data;
    }

};}

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { namespace hs {

    tcp_t server( const tcp_t& skt ){ skt.onSocket([=]( socket_t cli ){

        auto hrv = type::cast<http_t>(cli);
        if( !generator::hs::server( hrv ) )
          { skt.onConnect.skip(); return; }   

        process::add([=](){ 
            skt.onConnect.resume( );
            skt.onConnect.emit(cli); 
            return -1;
        });

    }); skt.onConnect([=]( hs_t cli ){
        cli.onDrain.once([=](){ cli.free(); });
        cli.set_timeout(0); cli.resume(); stream::pipe(cli); 
    }); return skt; }

    /*─······································································─*/

    tcp_t server( agent_t* opt=nullptr ){
    auto skt = http::server( [=]( http_t ){}, opt );
                 hs::server( skt ); return skt;
    }

    /*─······································································─*/

    tcp_t client( const string_t& uri, agent_t* opt=nullptr ){ 
    tcp_t skt   ( [=]( socket_t ){}, opt );
    skt.onSocket.once([=]( socket_t cli ){

        auto hrv = type::cast<http_t> (cli);
        if(!generator::hs::client( hrv, uri ) )
          { skt.onConnect.skip(); return; }   

        process::add([=](){ 
            skt.onConnect.resume( );
            skt.onConnect.emit(cli); 
            return -1;
        });

    }); skt.onConnect.once([=]( hs_t cli ){
        cli.onDrain.once([=](){ cli.free(); });
        cli.set_timeout(0); cli.resume(); stream::pipe(cli); 
    }); skt.connect( url::hostname(uri), url::port(uri) ); return skt; }

}}

#endif