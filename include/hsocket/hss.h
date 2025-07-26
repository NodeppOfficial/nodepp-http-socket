/*
 * Copyright 2023 The Nodepp Project Authors. All Rights Reserved.
 *
 * Licensed under the MIT (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://github.com/NodeppOficial/nodepp/blob/main/LICENSE
 */

/*────────────────────────────────────────────────────────────────────────────*/

#ifndef NODEPP_HTTPS_SOCKET
#define NODEPP_HTTPS_SOCKET

/*────────────────────────────────────────────────────────────────────────────*/

#include <nodepp/https.h>
#include "generator.h"

/*────────────────────────────────────────────────────────────────────────────*/

namespace nodepp { class hss_t : public ssocket_t {
protected:

    struct NODE {
        generator::hs::write write;
        generator::hs::read  read ;
    };  ptr_t<NODE> hs;

public:

    template< class... T > 
    hss_t( const T&... args ) noexcept : ssocket_t( args... ), hs( new NODE() ){}

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

namespace nodepp { namespace hss {

    tls_t server( const tls_t& skt ){ skt.onSocket([=]( ssocket_t cli ){
       
        auto hrv = type::cast<https_t>(cli);
        if(!generator::hs::server( hrv ) )
          { skt.onConnect.skip(); return; }   

        process::add([=](){ 
            skt.onConnect.resume( );
            skt.onConnect.emit(cli); 
            return -1;
        });

    }); skt.onConnect([=]( hss_t cli ){
        cli.onDrain.once([=](){ cli.free(); });
        cli.set_timeout(0); cli.resume(); stream::pipe(cli); 
    }); return skt; }

    /*─······································································─*/

    tls_t server( const ssl_t* ssl, agent_t* opt=nullptr ){
    auto skt = https::server( [=]( https_t ){}, ssl, opt );
                 hss::server( skt ); return skt;
    }

    /*─······································································─*/

    tls_t client( const string_t& uri, const ssl_t* ssl, agent_t* opt=nullptr ){
    tls_t skt   ( [=]( ssocket_t ){}, ssl, opt ); 
    skt.onSocket.once([=]( ssocket_t cli ){

        auto hrv = type::cast<https_t>(cli);
        if(!generator::hs::client( hrv, uri ) )
          { skt.onConnect.skip(); return; }   

        process::add([=](){ 
            skt.onConnect.resume( );
            skt.onConnect.emit(cli); 
            return -1;
        });

    }); skt.onConnect.once([=]( hss_t cli ){
        cli.onDrain.once([=](){ cli.free(); });
        cli.set_timeout(0); cli.resume(); stream::pipe(cli); 
    }); skt.connect( url::hostname(uri), url::port(uri) ); return skt; }

}}

#endif