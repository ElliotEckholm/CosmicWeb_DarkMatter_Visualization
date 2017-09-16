/*
 *  main.h
 *  From Apple GLUTBasics example
 *
 *  Created by Peter Behroozi on 3/22/10.
 *	Changes by (C)2016 Elliot Eckholm
 *  Copyright 2010. BSD License.
 *
 */

#ifndef _GLUTBASICS_H_
#define _GLUTBASICS_H_

#define DTOR 0.0174532925
#define CROSSPROD(p1,p2,p3) \
p3.x = p1.y*p2.z - p1.z*p2.y; \
p3.y = p1.z*p2.x - p1.x*p2.z; \
p3.z = p1.x*p2.y - p1.y*p2.x

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define KINECT_HEAD_OFFSET 0.15
#define KINECT_EYE_OFFSET 0.00015
#define LG_TV_WIDTH 1.43
#define LG_TV_HEIGHT  0.80
#define USER_WAIST 0.6
#define USER_WAIST_TO_SHOULDER .47
#define FAR_CLIP 5.0
#define NEAR_CLIP 0.01

enum {
    kTextureSize = 256
};

typedef struct {
	GLdouble x,y,z;
} recVec;

typedef struct {
  float x1,y1,z1;
  float x2,y2,z2;
  int   e1, e2;
} line;


typedef struct {
    recVec viewPos; // View position
    recVec viewDir; // View direction vector
    recVec viewUp; // View up direction
    recVec rotPoint; // Point to rotate about
    GLdouble focalLength; // Focal Length along view direction
    GLdouble aperture; // gCamera aperture
    GLdouble eyeSep; // Eye separation
    GLint screenWidth,screenHeight; // current window/screen height and width
} recCamera;

#define MMP_HALO 0
#define MERGED_HALO 1
#define PHANTOM_HALO 2
#define DEAD_HALO 3

typedef struct {
	float x,y,z;
	float vx,vy,vz;
  float R;
  float bta, cta, bta500c, cta500c;
	float rvir, mvir, vmax, rs, vrms;
  float t1, t2, t3, v1,v2, v3;
	int type, id, descid, mmp, phantom, track;
  float xrot, zrot, xrot500c, zrot500c;
 
  float scale;
  float almm, lambda_peebles, tidal_force, rs_klypin, x_off, v_off, lambda_bullock;
  float ttu;
} halo;

typedef struct {
	float x,y,z;
	
	
} voxel;

/*
typedef struct {
	float x,y,z;
  float color;
} galaxy;
*/


typedef struct {
  line * lines;
  int numlines;
} mst_list;

typedef struct {
  mst_list * type0_msts; 
  mst_list * type1_msts; 
  mst_list * type2_msts; 
  mst_list * type3_msts; 
} multi_mst;

typedef struct {
	halo *halos;
	int num_halos;
	float time, end_time, start_time, scale;
} halo_list;

typedef struct {
	voxel *voxels;
	int num_voxels;
	
} voxel_list;

typedef struct {
	int id;
	voxel_list *voxel_lists;
} filament;

/*
typedef struct {
	galaxy *galaxies;
	int num_galaxies;
} galaxy_list;
*/
void build_sphere_lists(void);
void render_halos(void);

template <typename T>
int readBinaryFile1D (std::string fileName, T * &data, int &num_lines) {

    std::cout << "Reading binary file: \"" << fileName << "\"" << std::endl;

    std::ifstream inpt(fileName,std::ios::in|std::ios::binary);

    long int header[1];

    // read header
    inpt.read((char *)header, sizeof(long int));

    num_lines = header[0];

    std::cout << "Number of lines: " << num_lines << std::endl;

    data = (T *) malloc (num_lines * sizeof(T));

    inpt.read((char *)data, sizeof(T)*num_lines);

    inpt.close();

    std::cout << " complete." << std::endl;

    return 0;
}



#endif /* _GLUTBASICS_H_ */
