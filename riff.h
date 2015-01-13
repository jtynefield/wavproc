#ifndef __RIFF_H_
#define __RIFF_H_

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cinttypes>
#include <cstring>
#include <deque>

struct FourCC {
    char d[5];
    FourCC() { d[4] = 0; };
    FourCC( const char *c ) { memcpy( d, c, 4 ); d[4] = 0; };
    bool operator== ( const char *c ) const { return 0 == memcmp( d, c, 5 ); };
    operator char *() { return d; };
};

struct Chunk {
    FourCC id;
    uint32_t length;
    char *data;

    void serialize( FILE *file ) {
        printf( "writing chunk: %s length: %d\n", (char*)id, length );
        assert( 4 == fwrite( id.d, 1, 4, file ) );        
        assert( 4 == fwrite( &length, 1, 4, file ) );        
        uint32_t padded_length = length + (length&1);
        assert( padded_length == fwrite( data, 1, padded_length, file ) );        
    }

    ~Chunk() { delete data; };
};

struct RIFF {
    RIFF( const char *fname ) {
        FILE *file = fopen( fname, "rb" );
        assert( file != NULL );

        FourCC hdr;
        assert( 4 == fread( hdr.d, 1, 4, file ) );
        assert( hdr == "RIFF" );

        uint32_t file_size;
        assert( 4 == fread( &file_size, 1, 4, file ) );

        assert( 4 == fread( filetype.d, 1, 4, file ) ); // read file type id

        while( 4 == fread( hdr.d, 1, 4, file ) ) {
            Chunk *c = new Chunk;
            c->id = hdr;
            assert( 4 == fread( &c->length, 1, 4, file ) );
            // printf( "reading chunk: %s length: %d\n", (char*)c->id, c->length );
            uint32_t padded_length = c->length + (c->length&1);
            c->data = new char[padded_length];
            assert( padded_length == fread( c->data, 1, padded_length, file ) );
            chunks.push_back( c );
        }
        if ( !feof( file ) ) {
            assert( 0 && "weird error or unexpected pad bytes at end of file" );
        }
    };

    RIFF( const uint8_t *buf );
    RIFF();
    ~RIFF() {
        for( auto c: chunks ) {
            delete c->data;
        }
        chunks.clear();
    };
    
    FourCC filetype;
    std::deque<Chunk*> chunks;

    void serialize( const char *fname ) const {
        FILE *file = fopen( fname, "wb" );

        assert( 4 == fwrite( "RIFF", 1, 4, file ) );

        // compute size of file
        unsigned long file_size = 4;
        for( auto c: chunks ) {
            file_size += 8 + c->length;
        }
        assert( 4 == fwrite( &file_size, 1, 4, file ) );

        assert( 4 == fwrite( &filetype, 1, 4, file ) );

        for( auto c: chunks ) {
            c->serialize( file );
        }
        
        fclose( file );
    };

    void serialize( uint8_t *buf ) const;

    void addChunk( const Chunk *c );
    void deleteChunk( unsigned int seq_num ) { // super unsafe for now...
        chunks.erase( chunks.begin() + seq_num );
    }
};


#endif
