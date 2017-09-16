
/* sphere.c - by David Blythe, SGI */

/* Instead of tessellating a sphere by lines of longitude and latitude
   (a technique that over tessellates the poles and under tessellates
   the equator of the sphere), tesselate based on regular solids for a
   more uniform tesselation.

   This approach is arguably better than the gluSphere routine's
   approach using slices and stacks (latitude and longitude). -mjk */

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include <iostream>

#include <stdlib.h>
#include <math.h>
#include "sphere.h"

//#include <GL/glut.h>


/* vertices of a unit icosahedron */
static triangle icosahedron[20]= {
        /* front pole */
        { {Ip0, Ip1, Ip2}, },
        { {Ip0, Ip5, Ip1}, },
        { {Ip0, Ip4, Ip5}, },
        { {Ip0, Ip3, Ip4}, },
        { {Ip0, Ip2, Ip3}, },

        /* mid */
        { {Ip1, Im0, Im1}, },
        { {Im0, Ip1, Ip5}, },
        { {Ip5, Im4, Im0}, },
        { {Im4, Ip5, Ip4}, },
        { {Ip4, Im3, Im4}, },
        { {Im3, Ip4, Ip3}, },
        { {Ip3, Im2, Im3}, },
        { {Im2, Ip3, Ip2}, },
        { {Ip2, Im1, Im2}, },
        { {Im1, Ip2, Ip1}, },

        /* back pole */
        { {Im3, Im2, Im5}, },
        { {Im4, Im3, Im5}, },
        { {Im0, Im4, Im5}, },
        { {Im1, Im0, Im5}, },
        { {Im2, Im1, Im5}, },
};

/* normalize point r */
static void normalize(point *r) {
    float mag;

    mag = r->x * r->x + r->y * r->y + r->z * r->z;
    if (mag != 0.0f) {
        mag = 1.0f / sqrt(mag);
        r->x *= mag;
        r->y *= mag;
        r->z *= mag;
    }
}

/* linearly interpolate between a & b, by fraction f */
static void lerp(point *a, point *b, float f, point *r) {
    r->x = a->x + f*(b->x-a->x);
    r->y = a->y + f*(b->y-a->y);
    r->z = a->z + f*(b->z-a->z);
}

void sphere(int maxlevel) {
    int nrows = 1 << maxlevel;
    int s;

    /* iterate over the 20 sides of the icosahedron */
    for(s = 0; s < 20; s++) {
        int i;
        triangle *t = &icosahedron[s];
        for(i = 0; i < nrows; i++) {
            /* create a tstrip for each row */
            /* number of triangles in this row is number in previous +2 */
            /* strip the ith trapezoid block */
            point v0, v1, v2, v3, va, vb;
            int j;
            lerp(&t->pt[1], &t->pt[0], (float)(i+1)/nrows, &v0);
            lerp(&t->pt[1], &t->pt[0], (float)i/nrows, &v1);
            lerp(&t->pt[1], &t->pt[2], (float)(i+1)/nrows, &v2);
            lerp(&t->pt[1], &t->pt[2], (float)i/nrows, &v3);
            glBegin(GL_TRIANGLE_STRIP);

#define V(v)    { point x; x = v; normalize(&x);	glNormal3fv(&x.x); glVertex3fv(&x.x); }
            V(v0);
            V(v1);
            for(j = 0; j < i; j++) {
                /* calculate 2 more vertices at a time */
                lerp(&v0, &v2, (float)(j+1)/(i+1), &va);
                lerp(&v1, &v3, (float)(j+1)/i, &vb);
                V(va);
                V(vb);
            }
            V(v2);
#undef V
            glEnd();
        }
    }
}
