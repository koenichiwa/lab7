// sample code accompanying the paper:
// "Analytical Methods for Squaring the Disc"
// http://arxiv.org/abs/1509.06344
// slight changes made by Koen van Wel, porting it from c++ to c
// sourced from: http://squircular.blogspot.com/2015/09/mapping-circle-to-square.html

#include <stdio.h>
#include <math.h>

// Elliptical Grid mapping
// mapping a circular disc to a square region
// input: (u,v) coordinates in the circle
// output: (x,y) coordinates in the square
void ellipticalDiscToSquare(float u, float v, float* x, float* y)
{
    float u2 = u * u;
    float v2 = v * v;
    
    float twosqrt2 = 2.0 * M_SQRT2;
    float subtermx = 2.0 + u2 - v2;
    float subtermy = 2.0 - u2 + v2;
    float termx1 = subtermx + u * twosqrt2;
    float termx2 = subtermx - u * twosqrt2;
    float termy1 = subtermy + v * twosqrt2;
    float termy2 = subtermy - v * twosqrt2;
    *x = 0.5 * sqrt(termx1) - 0.5 * sqrt(termx2);
    *y = 0.5 * sqrt(termy1) - 0.5 * sqrt(termy2);
    
}


// Elliptical Grid mapping
// mapping a square region to a circular disc
// input: (x,y) coordinates in the square
// output: (u,v) coordinates in the circle
void ellipticalSquareToDisc(float x, float y, float* u, float* v)
{
    *u = x * sqrt(1.0 - y*y/2.0);
    *v = y * sqrt(1.0 - x*x/2.0);    
}
