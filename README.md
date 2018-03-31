# mandelbrot

Exercise in concurrency in C for my Operating Systems class (CSE 3320 @ UTA).

This is composed of two programs: `mandel` and `mandelseries`.

## mandel

This program will take an image's specification from the command-line and
generate the mandelbrot fractal acoording to them. The image will be split
across the specified number of threads. 

By default, the program will split the workload by assigning each thread a
starting and ending row, and after they are finished computing, the thread
will terminate. However, I also added an optional work-stealing algorithm
which will split the image into individual rows. When a thread finishes 
processing a row, it will check to see if there is another row it could start
working on. With this algorithm, a thread will only exit after all of the work
has either been done or is being worked on.

The program should be invoked like this:
```
./bin/mandel [-w] [options]
```

Where the `-w` flag will enable the workstealing algorithm.

These are the valid options for the program:

| Flag | Argument | Default | Meaning |
| ---- | -------- | ------- | ------- |
| -W   | uint | 500 | the width of the output images |
| -H   | uint | 500 | the height of the output images |
| -x   | double | 0.0 | the x coordinate of the center of the image |
| -y   | double | 0.0 | the y coordinate of the center of the image |
| -s   | double | 4.0 | the scale of the image |
| -m   | uint | 1000 | the maximum number of iterations to try at a point |
| -o   | string | "mandel.bmp" | the output image file. A number will be added before the extension denoting which image in the series it is |
| -n   | uint | 1 | the number of threads to process the image with |

## mandelseries

This program will take an image's specification from command-line arguments
and generate a series of 50 images which gradually zoom in on the specified
image.

The program should be invoked like this:
```
./bin/mandelseries [options] {number_of_processes}
```

Where `{number_of_processes}` is the maximum number of concurrent child processes 
to let run at any time.

These are the valid options for the program:  

| Flag | Argument | Default | Meaning |
| ---- | -------- | ------- | ------- |
| -W   | uint | 500 | the width of the output images |
| -H   | uint | 500 | the height of the output images |
| -x   | double | 0.0 | the x coordinate of the center of the image |
| -y   | double | 0.0 | the y coordinate of the center of the image |
| -s   | double | 4.0 | the scale of the image |
| -m   | uint | 1000 | the maximum number of iterations to try at a point |
| -o   | string | "mandel.bmp" | the output image file. A number will be added before the extension denoting which image in the series it is |
