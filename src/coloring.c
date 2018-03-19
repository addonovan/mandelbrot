#include <coloring.h>
#include <bitmap.h>
#include <math.h>

#define SMOOTHING 1

#define INTERPOLATE(i, max)     SQRT( INVERSE( LINEAR( i, max ) ) )
#define LINEAR(i, max)          ( i / max )
#define QUADRATIC(i, max)       ( LINEAR( i, max ) * LINEAR( i, max ) )
#define CUBIC(i, max)           ( LINEAR( i, max ) * QUADRATIC( i, max ) )
#define LOGARITHMIC(i, max)     ( log( i ) / log( max ) )
#define EXPONENTIAL(i, max)     ( pow( 2, i ) / pow( 2, max ) ) 

#define INVERSE(a)              ( 1.0 - a ) 
#define SQRT(a)                 ( sqrt( a ) )


#define SCHEME_ENTRIES 18
static const int color_scheme[ SCHEME_ENTRIES + 1 ][ 3 ];

int iteration_to_color( int i, int max )
{
  double ratio = INTERPOLATE( ( double ) i, ( double ) max );

  if ( !SMOOTHING )
  {
    const int* color = color_scheme[ ( int ) ( SCHEME_ENTRIES * ratio ) ];

    return MAKE_RGBA( color[ 0 ], color[ 1 ], color[ 2 ], 0 );
  }
  else
  {
    int scheme_step = max / SCHEME_ENTRIES;

    const int* lower = color_scheme[ ( int ) ( SCHEME_ENTRIES * ratio ) ];
    const int* upper = color_scheme[ ( int ) ( SCHEME_ENTRIES * ratio ) + 1 ];

    double mixing = ( i % scheme_step ) / ( double ) scheme_step;

    return MAKE_RGBA(
        ( lower[ 0 ] * mixing ) + ( upper[ 0 ] * ( 1 - mixing ) ),
        ( lower[ 1 ] * mixing ) + ( upper[ 1 ] * ( 1 - mixing ) ),
        ( lower[ 2 ] * mixing ) + ( upper[ 2 ] * ( 1 - mixing ) ),
        0
    );
  }
}

//
// Color Scheme
//

static const int color_scheme[ SCHEME_ENTRIES + 1 ][ 3 ] = {
  {   0,   0,   0 },
  {  66,  30,  15 },
  {  25,   7,  26 },
  {   9,   1,  47 },
  {   4,   4,  73 },
  {  12,  44, 138 },
  {  24,  82, 177 },
  {  57, 125, 209 },
  { 134, 181, 229 },
  { 211, 236, 248 },
  { 241, 233, 191 },
  { 248, 201,  95 },
  { 255, 170,   0 },
  { 204, 128,   0 },
  { 153,  87,   0 },
  { 153,  87,   0 },
  { 106,  52,   3 },
  { 255, 255, 255 },
  { 255, 255, 255 },
};

