#ifndef NODEPP_HTTP_SOCKET_GENERATOR
#define NODEPP_HTTP_SOCKET_GENERATOR

#include <nodepp/encoder.h>

namespace nodepp { namespace _hs_ {

    /*─······································································─*/

    template< class T > bool server( T& cli ) {
        auto data = cli.read(); cli.set_borrow( data );

        if( cli.read_header()!=0 ){ return 0; }
        if( cli.headers["Upgrade"].to_lower_case() == "http-socket" ){

            cli.write_header( 101, header_t({
                { "Transfer-Encoding", "chunked" },
                { "Upgrade", "http-socket" },
                { "Connection", "upgrade" }
            }) );

            cli.stop();             return 1;
        }   cli.set_borrow( data ); return 0;
    }

    /*─······································································─*/

    template< class T > bool client( T& cli, string_t url ) {

        header_t header ({
            { "Transfer-Encoding", "chunked" },
            { "Upgrade", "http-socket" },
            { "Connection", "upgrade" }
        });

        cli.write_header( "GET", url::path(url), "HTTP/1.0", header );

        if( cli.read_header()!=0 ){
            _EERROR(cli.onError,"Could not connect to server");
            cli.close(); return false;
        }

        if( cli.status != 101 || cli.headers["Upgrade"].to_lower_case() != "http-socket" ){
            _EERROR(cli.onError,string::format("Can't connect to Server -> status %d",cli.status));
            cli.close(); return false;
        }   cli.stop();  return true;

    }

    /*─······································································─*/

    GENERATOR( read ){
    protected:
    ptr_t<char>   fb=ptr_t<char>(4);
        string_t  bff;
        ulong     size =0, sz=0;
        int       state=0;
    public: ulong data =0;

    template<class T> coEmit( T* str, char* bf, const ulong& sx ) {
        if( str->is_closed() ){ return -1; }
    gnStart

        memset( bf, 0, sx ); data=0; size=0; while( bf[0]!='\n' ){
        coWait((state=str->__read( bf, 1 ))==-2 );
            if( state<=0 ){ data=0; coEnd; }
        bff.push(bf[0]); }

        size=encoder::hex::btoa<ulong>( bff.slice(0,-2) );
        if( size==0 ){ data=0; coEnd; } bff.clear(); coYield(1);

        while ( size > 0 ){  sz = min( sx, size );
        coWait((state=str->__read( bf,sz ))==-2 );
            if( state<=0 ){ data=0; coEnd; }
            size-= min( sz, (ulong)state );
            data = min( sz, (ulong)state );
        coStay(1); }

    coWait( str->__read( fb.get(),2 )==-2 );
    coGoto(0) ; gnStop
    }};

    GENERATOR( write ){
    protected:
            string_t bff;
            ulong size=0;
    public: ulong data=0;

    template<class T> coEmit( T* str, char* bf, const ulong& sx ) {
        if( str->is_closed() ){ return -1; }
    gnStart

        bff=encoder::hex::get(sx)+"\r\n"+string_t(bf,sx)+"\r\n"; data=0;size=0;
        coWait( str->_write_( bff.get(),bff.size(),size ) ==1 ); data = sx;

    gnStop
    }};

}}

#endif
