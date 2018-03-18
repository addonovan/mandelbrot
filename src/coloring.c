#include <coloring.h>
#include <bitmap.h>

#define SCHEME_ENTRIES 2
static const int color_scheme[ SCHEME_ENTRIES ][ 3 ];

int iteration_to_color( int i, int max )
{
  const int* color = color_scheme[ SCHEME_ENTRIES * i / max ];
  return MAKE_RGBA( color[ 0 ], color[ 1 ], color[ 2 ], 0 );
}

//
// Color Scheme
//

static const int color_scheme[ SCHEME_ENTRIES ][ 3 ] = {
  {   0,   0,   0 },
  { 255, 255, 255 },
};

