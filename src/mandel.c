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
#include <pthread.h>
#include <stdbool.h>

//
// Definitions
//

typedef struct
{
  bitmap* bm;

  int width;
  int height;
  
  double x_min;
  double x_max;

  double y_min;
  double y_max;

  int max;
}
image_params_t;

typedef struct
{
  const image_params_t* params;

  int row_start;
  int row_end;
}
work_t;

int work_pool_index = 0;
work_t* work_pool = NULL;
pthread_mutex_t m_work_pool = PTHREAD_MUTEX_INITIALIZER;

//
// Declarations
//

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void* mandelbrot_compute( void* );
void show_help();

//
// Implementations
//

int main( int argc, char *argv[] )
{
  char c;

  // These are the default configuration values used
  // if no command line arguments are given.
  char* file_name = "mandel.bmp";
  double x_center = 0;
  double y_center = 0;
  double scale = 4;
  int image_width = 500;
  int image_height = 500;
  int max = 1000;
  int thread_count = 1;
  bool work_stealing = false;

  // For each command line argument given,
  // override the appropriate configuration value.

  while( ( c = getopt( argc, argv, "n:x:y:s:W:H:m:o:hw" ) ) != -1 ) 
  {
    switch( c )
    {
      case 'w':
        work_stealing = true;
        break;

      case 'n':
        thread_count = atoi( optarg );
        break;

      case 'x':
        x_center = atof( optarg );
        break;

      case 'y':
        y_center = atof( optarg );
        break;

      case 's':
        scale = atof( optarg );
        break;

      case 'W':
        image_width = atoi( optarg );
        break;

      case 'H':
        image_height = atoi( optarg );
        break;

      case 'm':
        max = atoi( optarg );
        break;

      case 'o':
        file_name = optarg;
        break;

      case 'h':
        show_help();
        exit( 0 );
        return 0;
    }
  }

  // Display the configuration of the image.
  printf( 
      "mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s threads=%d %s\n", 
      x_center,
      y_center,
      scale,
      max,
      file_name,
      thread_count,
      ( work_stealing ? "(work stealing)" : "" )
  );

  // create the generic params struct
  image_params_t params = {
    .x_min = x_center - scale,
    .x_max = x_center + scale,
    .y_min = y_center - scale,
    .y_max = y_center + scale,
    .bm = bitmap_create( image_width, image_height ),
    .width = image_width,
    .height = image_height,
    .max = max
  };

  // determine the size of our work pool (depends on work stealing)
  int work_size = thread_count;
  if ( work_stealing )
  {
    work_size = image_height;
  }

  // create the work pool
  pthread_mutex_init( &m_work_pool, NULL );
  work_pool = malloc( sizeof( work_t ) * work_size );
  work_pool_index = work_size - 1; // work from the back to the front

  int start_row = 0;
  int i;
  for ( i = 0; i < work_size; i++ )
  {
    work_pool[ i ].params = &params;
    work_pool[ i ].row_start = start_row;

    start_row += ( image_height / work_size );
    work_pool[ i ].row_end = start_row;
  }

  // make sure the entire image is generated
  work_pool[ work_size - 1 ].row_end = image_height + 1;

  // create the thread array
  pthread_t* threads = malloc( sizeof( pthread_t ) * thread_count );
  
  // spawn off all of the threads
  for ( i = 0; i < thread_count; i++ )
  {
    if ( pthread_create( threads + i, NULL, mandelbrot_compute, NULL ) )
    {
      perror( "Error creating thread: " );
      exit( EXIT_FAILURE );
    }
  }

  // wait for all threads to finish
  for ( i = 0; i < thread_count; i++ )
  {
    if ( pthread_join( threads[ i ], NULL ) )
    {
      perror( "Problem with pthread_join: " );
    }
  }

  // write the final image
  if( !bitmap_save( params.bm, file_name ) ) 
  {
    fprintf( 
        stderr, 
        "mandel: couldn't write to %s: %s\n",
        file_name,
        strerror( errno ) 
    );
    fflush( stderr );
    fflush( stdout );

    return 1;
  }

  return 0;
}

/**
 * Compute an entire Mandelbrot image, writing each point to the given bitmap.
 * Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
 */
void* mandelbrot_compute( void* _ )
{
  (void)(_);

  const work_t* work = NULL;

  // each thread will stay alive until there's no more work for it to do
  while ( work_pool_index >= 0 )
  {
    // get the next work item from the pool
    {
      pthread_mutex_lock( &m_work_pool ); 
      work = work_pool + work_pool_index;
      work_pool_index--;
      pthread_mutex_unlock( &m_work_pool );
    }

    int i, j;

    const image_params_t* info = work->params;

    int width = bitmap_width( info->bm );
    int height = bitmap_height( info->bm );

    // For every pixel in the image...

    for( j = work->row_start; j < work->row_end; j++ )
    {

      for( i = 0; i < width; i++ )
      {

        // Determine the point in x,y space for that pixel.
        double x = info->x_min + i * ( info->x_max - info->x_min ) / width;
        double y = info->y_min + j * ( info->y_max - info->y_min ) / height;

        // Compute the iterations at that point.
        int iters = iterations_at_point( x, y, info->max );

        // Set the pixel in the bitmap.
        // This seems dangerous (modifying shared data), but it's guaranteed that
        // we can't trample this memory because this row will only be edited by us
        bitmap_set( info->bm, i, j, iters );
      }
    }
  }

  return NULL;
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
  printf( "Use: mandel [options]\n" );
  printf( "\n" );
  printf( "Where options are:\n" );
  printf( "-m <max>     The maximum number of iterations per point. (default=1000)\n" );
  printf( "-x <coord>   X coordinate of image center point. (default=0)\n" );
  printf( "-y <coord>   Y coordinate of image center point. (default=0)\n" );
  printf( "-s <scale>   Scale of the image in Mandlebrot coordinates. (default=4)\n ");
  printf( "-W <pixels>  Width of the image in pixels. (default=500)\n ");
  printf( "-H <pixels>  Height of the image in pixels. (default=500)\n ");
  printf( "-o <file>    Set output file. (default=mandel.bmp)\n ");
  printf( "-n <threads> Sets the number of threads to run at one time (default=1)\n" );
  printf( "-w           Uses a work-stealing algorithm where each thread will grab\n" );
  printf( "             will grab an unprocessed row from a common pool until the\n" );
  printf( "             image has been finished\n" );
  printf( "-h           Show this help text.\n ");
  printf( "\n" );
  printf( "Some examples are:\n" );
  printf( "mandel -x -0.5 -y -0.5 -s 0.2\n" );
  printf( "mandel -x -.38 -y -.665 -s .05 -m 100\n" );
  printf( "mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n" );
}

