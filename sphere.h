/*
 *  sphere.h
 *  GLUTBasics
 *
 *  Created by Peter Behroozi on 3/22/10.
 *
 */

#ifndef _SPHERE_H_
#define _SPHERE_H_

typedef struct {
    float x, y, z;
} point;

typedef struct {
    point pt[3];
} triangle;

typedef struct {
    point pt[3];
    point n[3];
	point color;
} triangle_c;


/* six equidistant points lying on the unit sphere */
#define XPLUS {  1,  0,  0 }    /*  X */
#define XMIN  { -1,  0,  0 }    /* -X */
#define YPLUS {  0,  1,  0 }    /*  Y */
#define YMIN  {  0, -1,  0 }    /* -Y */
#define ZPLUS {  0,  0,  1 }    /*  Z */
#define ZMIN  {  0,  0, -1 }    /* -Z */

/* for icosahedron */
#define CZ (0.89442719099991)   /*  2/sqrt(5) */
#define SZ (0.44721359549995)   /*  1/sqrt(5) */
#define C1 (0.951056516)        /* cos(18),  */
#define S1 (0.309016994)        /* sin(18) */
#define C2 (0.587785252)        /* cos(54),  */
#define S2 (0.809016994)        /* sin(54) */
#define X1 (C1*CZ)
#define Y1 (S1*CZ)
#define X2 (C2*CZ)
#define Y2 (S2*CZ)

#define Ip0     {0.,    0.,     1.}
#define Ip1     {-X2,   -Y2,    SZ}
#define Ip2     {X2,    -Y2,    SZ}
#define Ip3     {X1,    Y1,     SZ}
#define Ip4     {0,     CZ,     SZ}
#define Ip5     {-X1,   Y1,     SZ}

#define Im0     {-X1,   -Y1,    -SZ}
#define Im1     {0,     -CZ,    -SZ}
#define Im2     {X1,    -Y1,    -SZ}
#define Im3     {X2,    Y2,     -SZ}
#define Im4     {-X2,   Y2,     -SZ}
#define Im5     {0.,    0.,     -1.}


void sphere(int maxlevel);

#endif /*_SPHERE_H_*/

