#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <bitmap.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct
{
  char *file_name;
  double x_center;
  double y_center;
  double scale;
  int image_width;
  int image_height;
  int max;
  int process_count;
}
options_t;

typedef struct 
{
  int pid;
  char* file_name;
  
  bitmap* bm;
  
  double x_min;
  double x_max;

  double y_min;
  double y_max;

  int max;
}
mandelbrot_t;

int iteration_to_color( int i, int max );

int iterations_at_point( double x, double y, int max );

void spawn_children( options_t options );

void mandelbrot_compute( mandelbrot_t* this );

void show_help();

//
// Implementations
//

int main( int argc, char *argv[] )
{
  char c;

  // These are the default configuration values used
  // if no command line arguments are given.
  options_t options;
  options.file_name = "mandel.bmp";
  options.x_center = 0;
  options.y_center = 0;
  options.scale = 4;
  options.image_width = 500;
  options.image_height = 500;
  options.max = 1000;
  options.process_count = 1;

  // For each command line argument given,
  // override the appropriate configuration value.

  while( ( c = getopt( argc, argv, "x:y:s:W:H:m:o:h" ) ) != -1 ) 
  {
    switch( c )
    {
      case 'x':
        options.x_center = atof( optarg );
        break;

      case 'y':
        options.y_center = atof( optarg );
        break;

      case 's':
        options.scale = atof( optarg );
        break;

      case 'W':
        options.image_width = atoi( optarg );
        break;

      case 'H':
        options.image_height = atoi( optarg );
        break;

      case 'm':
        options.max = atoi( optarg );
        break;

      case 'o':
        options.file_name = optarg;
        break;

      case 'h':
        show_help();
        return 0;
    }
  }

  int i;
  for ( i = optind; i < argc; i++ )
  {
    options.process_count = atoi( argv[ i ] );
  }

  // Display the configuration of the image.
  printf( 
      "mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s processes=%d\n", 
      options.x_center,
      options.y_center,
      options.scale,
      options.max,
      options.file_name,
      options.process_count
  );

  // take everything before the extension in the file name, then create a new
  // format string from that
  const char* file_name = strtok( options.file_name, "." );
  char* file_name_format = calloc( sizeof( char ), strlen( file_name ) + 7 );
  sprintf( file_name_format, "%s%%d.bmp", file_name );
  options.file_name = file_name_format;
   
  spawn_children( options );

  return 0;
}

void spawn_children( options_t options )
{
  int remaining = 50;
  int active = 0;
  double scale = 2.0;
  double step = ( 2.0 - options.scale ) / remaining;

  mandelbrot_t mandel;
  mandel.bm = bitmap_create( options.image_width, options.image_height );
  bitmap_reset( mandel.bm, MAKE_RGBA( 0, 0, 255, 0 ) );
  
  mandel.max = options.max;
  
  for ( ; remaining > 0; remaining-- )
  {
    pid_t child = fork();

    // failed to fork
    if ( child < 0 )
    {
      fprintf( stderr, "Failed to spawn process!\n" );
      exit( 1 );
    }
    // we're in the child
    else if ( child == 0 )
    {
      mandel.file_name = calloc( sizeof( char ), 2048 );
      sprintf( mandel.file_name, options.file_name, remaining );

      mandel.pid = remaining;
      mandel.x_min = options.x_center - scale;
      mandel.x_max = options.x_center + scale;
      mandel.y_min = options.y_center - scale;
      mandel.y_max = options.y_center + scale;
      fflush( stdout );

      mandelbrot_compute( &mandel );
      exit( 0 );
    }

    // => we're in the parent, so continue dispatching new images
    scale -= step;
    active++;

    // if we have too many running processes, then we're going to wait
    // until we get down to the point where we can spawn another process
    while ( active >= options.process_count )
    {
      wait( NULL );
      active--;
    }
  }

  // wait for any stragglers
  while ( wait( NULL ) > 0 );
}

/**
 * Compute an entire Mandelbrot image, writing each point to the given bitmap.
 * Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
 */
void mandelbrot_compute( mandelbrot_t* this )
{
  int i,j;

  int width = bitmap_width( this->bm );
  int height = bitmap_height( this->bm );

  // For every pixel in the image...

  for( j = 0; j < height; j++ )
  {

    for( i = 0; i < width; i++ )
    {

      // Determine the point in x,y space for that pixel.
      double x = this->x_min + i * ( this->x_max - this->x_min ) / width;
      double y = this->y_min + j * ( this->y_max - this->y_min ) / height;

      // Compute the iterations at that point.
      int iters = iterations_at_point( x, y, this->max );

      // Set the pixel in the bitmap.
      bitmap_set( this->bm, i, j, iters );
    }
  }

  // Save the image in the stated file.
  if( !bitmap_save( this->bm, this->file_name ) ) 
  {
    fprintf( 
        stderr, 
        "%d [%d] mandel: couldn't write to %s: %s\n",
        this->pid,
        getpid(),
        this->file_name,
        strerror( errno ) 
    );
    fflush( stderr );
    fflush( stdout );

    exit( 1 );
    return;
  }

  fflush( stdout );
}

/**
 * Return the number of iterations at point x, y
 * in the Mandelbrot space, up to a maximum of max.
 */
int iterations_at_point( double x, double y, int max )
{
  double x0 = x;
  double y0 = y;

  int iter = 0;

  while( ( x * x + y * y <= 4 ) && iter < max ) {

    double xt = x * x - y * y + x0;
    double yt = 2 * x * y + y0;

    x = xt;
    y = yt;

    iter++;
  }

  return iteration_to_color( iter, max );
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
  int gray = 255 * i / max;
  return MAKE_RGBA( gray, gray, gray, 0 );
}

void show_help()
{
  printf( "Use: mandel [options] <process_count>\n" );
  printf( "Where process_count is the number of processes to run at one time\n" );
  printf( "\n" );
  printf( "Where options are:\n" );
  printf( "-m <max>    The maximum number of iterations per point. (default=1000)\n" );
  printf( "-x <coord>  X coordinate of image center point. (default=0)\n" );
  printf( "-y <coord>  Y coordinate of image center point. (default=0)\n" );
  printf( "-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n ");
  printf( "-W <pixels> Width of the image in pixels. (default=500)\n ");
  printf( "-H <pixels> Height of the image in pixels. (default=500)\n ");
  printf( "-o <file>   Set output file. (default=mandel.bmp)\n ");
  printf( "-h          Show this help text.\n ");
  printf( "\n" );
  printf( "Some examples are:\n" );
  printf( "mandel -x -0.5 -y -0.5 -s 0.2\n" );
  printf( "mandel -x -.38 -y -.665 -s .05 -m 100\n" );
  printf( "mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n" );
}

