
#include <GLUT/glut.h>
#include <vector>
#include "riff.h"


float cur_width;
float cur_height;
uint32_t time_ptr;
std::vector<float> samples;

#define SUBDIVS 1000

void bitmapString( const char *s ) {
    while( *s ) {
        glutBitmapCharacter( GLUT_BITMAP_8_BY_13, *s );
        ++s;
    }
}

void display(void)
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f( 1.0f, 1.0f, 1.0f );
    
    glRasterPos2f( 0.0f, 0.0f );
    bitmapString( "(0,0)" );

    glRasterPos2f( -1.0f, -1.0f );
    char str[256];
    sprintf( str, "time: %d", time_ptr );
    bitmapString( str );

    glBegin( GL_LINES );
    glVertex2f( -1.0f, 0.0f );
    glVertex2f(  1.0f, 0.0f );

    glVertex2f( 0.0f, -1.0f );
    glVertex2f( 0.0f,  1.0f );
    glEnd();

    glBegin( GL_LINES );
    glColor3f( 0.0f, 1.0f, 0.0f );
    for( uint32_t t = 0; t < (SUBDIVS-1); ++t ) {
        float x0 = -1.0f + float(t) * 2.0f / float(SUBDIVS);
        float x1 = -1.0f + float(t+1) * 2.0f / float(SUBDIVS);
        float y0 = samples[time_ptr+t];
        float y1 = samples[time_ptr+t+1];

        glVertex2f( x0, y0 );
        glVertex2f( x1, y1 );
    }
    glEnd();

    glutSwapBuffers();
}

void reshape(int width, int height)
{
    cur_width = float(width);
    cur_height = float(height);
    glViewport(0, 0, width, height);
}

void idle(void)
{
    glutPostRedisplay();
    ++time_ptr;
    if ( time_ptr >= (samples.size() - SUBDIVS) ) {
        time_ptr = 0;
    }
}


void keyboard(unsigned char key, int x, int y) {
    switch(key) {
    case 0x1b: // esc
        exit( 0 ); // all done
        break;
    default:
        break;
    }
    glutPostRedisplay();
}
struct WaveFormat {
    uint16_t CompressionFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;

    void print() {
        const char *fmt = "";
        switch( CompressionFormat ) {
        case 1: fmt = "PCM/uncompressed"; break;
        case 2: fmt = "Microsoft ADPCM"; break;
        case 6: fmt = "ITU G.711 a-law"; break;
        case 7: fmt = "ITU G.711 Âµ-law"; break;
        case 17: fmt = "IMA ADPCM"; break;
        case 20: fmt = "ITU G.723 ADPCM (Yamaha)"; break;
        case 49: fmt = "GSM 6.10"; break;
        case 64: fmt = "ITU G.721 ADPCM"; break;
        case 80: fmt = "MPEG"; break;
        default: fmt = "error/unrecognized"; break;
        }
        printf( "    CompressionFormat: %s\n", fmt );
        printf( "    NumChannels: %d\n", NumChannels );
        printf( "    SampleRate: %d\n", SampleRate );
        printf( "    ByteRate: %d\n", ByteRate );
        printf( "    BlockAlign: %d\n", BlockAlign );
        printf( "    BitsPerSample: %d\n", BitsPerSample );
    };
};

int main( int argc, char **argv ) {
    time_ptr = 0;
    glutInit(&argc, argv);

    if ( argc == 1 ) {
        printf( "need wav filename as input\n" );
        return -1;
    }

    RIFF r( argv[1] );

    printf( "Loaded riff file of type: %s\n", (char*)(r.filetype) );

    uint32_t samples_per_second;
    uint32_t bits_per_sample;

    int count = 0;
    for( auto chunk: r.chunks ) {
        printf( "  Chunk %d: id %s\n", count++, (char*)(chunk->id) );

        if( chunk->id == "fmt " ) {
            assert( chunk->length == sizeof(WaveFormat) );

            
            (reinterpret_cast<WaveFormat *>(chunk->data))->print();

            samples_per_second = (reinterpret_cast<WaveFormat *>(chunk->data))->SampleRate;
            bits_per_sample = (reinterpret_cast<WaveFormat *>(chunk->data))->BitsPerSample;
        }
        if ( chunk->id == "data" ) {
            uint32_t bytes_per_sample = bits_per_sample / 8;
            uint32_t sample_count = chunk->length / bytes_per_sample;
            samples.reserve( sample_count );
            for( uint32_t i = 0; i < sample_count; ++i ) {
                int16_t raw = (reinterpret_cast<int16_t*>(chunk->data))[i];
                samples[i] = float(raw) / 32768.0f;
            }
        }
    }

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(640, 480);

    (void)glutCreateWindow("GLUT Program");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
