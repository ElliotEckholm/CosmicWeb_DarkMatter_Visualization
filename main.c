/* main.c: main program code.
   based on the "glutBasics.cpp" code from Apple Computer
   Changes by (C)2010 Peter Behroozi and (C)2016 Elliot Eckholm
   Copying license is BSD.
 */

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include <iostream>
#include <fstream>

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <vector>

#include <float.h>
 
//#include <GL/glut.h>
//#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "sphere.h"
#include "main.h"

#include "trackball.h"

#include <assert.h>

#include "Cube.h"

#include "Shader.h"


#define MED_COMPLEXITY_LIMIT 7e12    // Mass limits for refinement levels 
#define SMALL_COMPLEXITY_LIMIT 7e11  // (higher mass objects are drawn with more polygons)

#define RECENTER(p) halos[i].p -= c.p; \
			if (halos[i].p > BOX_SIZE/2.0) halos[i].p -= BOX_SIZE; \
			if (halos[i].p < -BOX_SIZE/2.0) halos[i].p += BOX_SIZE;\
			r += halos[i].p*halos[i].p;


/* Globals related to camera position and viewing frustrum */
GLfloat gShapeSize = 11.0f;
GLint gDollyPanStartPoint[2] = {0, 0};
GLfloat gTrackBallRotation [4] = {0.0, 0.0, 0.0, 0.0};
GLboolean gDolly = GL_FALSE;
GLboolean gPan = GL_FALSE;
GLboolean gTrackBall = GL_FALSE;
GLfloat gWorldRotation [4] = {155.0, 0.0, -1.0, 0.0};
recCamera gCamera;
recVec gOrigin = {0.0, 0.0, 0.0};

/* Info displays and program modes */
GLboolean gShowHelp = GL_FALSE;
GLboolean gShowInfo = GL_TRUE;
GLboolean gRecordMovie = GL_FALSE;

GLfloat xMAX = 0;
GLfloat xMIN = 1e6;

GLfloat mouseX = 0;
GLfloat mouseY = 0;

halo recenter;

int tweb = 1;
int gLastKey = ' ';
int fullScreen = 0;
point oldScreenSize;
int frame = 0;
unsigned char *frameBuffer = NULL;

GLboolean small_cut_mode = FALSE;
GLboolean big_cut_mode   = FALSE;



GLboolean rotation_enabled = FALSE;
float rotation_scale = 0.0;
float rotation_degrees = 0.01;

float global_scale = 1;
int gMainWindow = 0;
int list_selector = 5;
/* Pre-rendered polygon lists to increase rendering speed for spheres. */
GLuint gComplexSphereList = 0;
GLuint gMediumSphereList = 0;
GLuint gSimpleSphereList = 0;
GLuint gCubeList = 0;
GLuint gLinesList = 0;


//Number of environment types
float type0 = 0;
float type1 = 0;
float type2 = 0;
float type3 = 0;
float lambda_cut = 0.2;


/* Timing variables for halo movies */
float distance_scales [3] = {0.0, 0.0, 0.0};
float rotation_direction [3] = {0.0, 1.0, 0.0};
float time_now = 0;
float cur_scale = 1;
float timer_rate = 0;
float base_time = 0;
float eye_adjust = 0.001;
/* Info about the currently displayed timestep. */
halo *halos = NULL;

//galaxy *galaxies = NULL;
long num_halos = 0;

float selector_size = 0.001;
float move_scale = 0.01;
float distance_scale = 1;
float inv_velocity_scale = 1e-3;
int colormode = 2;
int radiusmode = 0;
int brightness = 0;


//list of halos 
halo_list hlist;
halo_list hlist2;
GLboolean renderHalo = TRUE;
GLfloat haloAlpha = 1.0f;


GLboolean renderWall = TRUE;
GLfloat wallAlpha = 1.0;
GLfloat wallScale = 0.002f;
GLboolean renderWall0 = FALSE;
GLboolean renderWall1 = FALSE;
GLboolean renderWall2 = TRUE;


filament *filaments = NULL;
GLfloat filamentAlpha = 1.0;
GLfloat filamentScale0 = 0.002f;
GLfloat filamentScale1 = 0.004f;
GLfloat filamentScale2 = 0.006f;
GLboolean renderFilament = TRUE;
GLboolean renderFilament0 = FALSE;
GLboolean renderFilament1 = FALSE;
GLboolean renderFilament2 = TRUE;

GLboolean renderSubCube4577 = FALSE;


voxel *voxels = NULL;
long num_voxels = 0;
int *** voxelData;

std::vector<voxel> flist0;
std::vector<voxel> flist1;
std::vector<voxel> flist2;

std::vector<voxel> wlist0;
std::vector<voxel> wlist1;
std::vector<voxel> wlist2;

std::vector<voxel> slist402;
std::vector<voxel> slist402_wall;

bool keys[256];

GLboolean toggleWallID = FALSE;


void gCameraReset(void) {
   gCamera.aperture = 40;
   gCamera.focalLength = 15;
   gCamera.rotPoint = gOrigin;

   gCamera.viewPos.x = 0.0;
   gCamera.viewPos.y = 0.0;
   gCamera.viewPos.z = -1.5;
   gCamera.viewDir.x = 0.0; 
   gCamera.viewDir.y = 0.0; 
   gCamera.viewDir.z = 0.0;

   gCamera.viewUp.x = 0;  
   gCamera.viewUp.y = 1; 
   gCamera.viewUp.z = 0;
}


void switchOnOffRotate(void) {
    rotation_enabled = !rotation_enabled; 
}

void rotate(float degrees, float x, float y, float z) {
	glRotatef (degrees, x, y ,z);
}


void drawGLString(GLfloat x, GLfloat y, char *string) {
    int len, i;

    glRasterPos2f(x, y);
    len = (int) strlen(string);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string[i]);
    }
}

void SetLighting(unsigned int mode) {
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_shininess[] = {25.0};

	GLfloat position[4] = {0.0,0.0,4.0,0.0};
	GLfloat ambient[4]  = {0.0,0.0,0.0,1.0};
	GLfloat diffuse[4]  = {0.45,0.45,0.45,1.0};
	GLfloat specular[4] = {0.5,0.5,0.5,1.0};
  GLfloat FogCol[3]   ={0.0f,0.0f,0.0f}; // Define a nice light grey
	
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

	switch (mode) {
		case 0:
			break;
		case 1:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			break;
		case 2:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			break;
		case 3:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			break;
		case 4:
			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			break;
	}
	
	glLightfv(GL_LIGHT0,GL_POSITION,position);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glEnable(GL_LIGHT0);

  position[0] = 0.0;
  position[1] = 5.0;
  position[2] = 5.0;
  glLightfv(GL_LIGHT1,GL_POSITION,position);
	glLightfv(GL_LIGHT1,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT1,GL_SPECULAR,specular);
	glEnable(GL_LIGHT1);

  position[0] = 5.0;
  position[1] = -5.0;
  position[2] = 0.0;
  glLightfv(GL_LIGHT2,GL_POSITION,position);
	glLightfv(GL_LIGHT2,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT2,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT2,GL_SPECULAR,specular);
	glEnable(GL_LIGHT2);

  //glEnable(GL_FOG);

  
  glFogfv(GL_FOG_COLOR,FogCol);     // Set the fog color

  glFogi(GL_FOG_MODE, GL_EXP); // Note the 'i' after glFog - the GL_LINEAR constant is an integer.
  glFogf(GL_FOG_START, 1.0f);
  glFogf(GL_FOG_END, 3.0f);
}


void drawMainText (GLint window_width, GLint window_height) {
	char outString [256] = "";
	GLint lineSpacing = 13;
	GLint line = 0;
	GLint startOffest = 60;
  float dx = 1600;
  float dy = 500;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glScalef(2.0f / window_width, -2.0f / window_height, 1.0f);
	glTranslatef(-window_width / 2.0f, -window_height / 2.0f, 0.0f);
	
	// draw 
	glDisable(GL_LIGHTING);
	glColor3f (1.0, 1.0, 1.0);
	if (gShowInfo) {
		sprintf_s (outString, 256, "Camera Position: (%0.1f, %0.1f, %0.1f)", gCamera.viewPos.x, gCamera.viewPos.y, gCamera.viewPos.z);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf_s (outString, 256, "Number of Halos: (%0.2f)", hlist.num_halos);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf_s (outString, 256, "Viwing Center: (%0.2f, %0.2f, %0.2f)", gCamera.viewDir.x, gCamera.viewDir.y, gCamera.viewDir.z);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf_s (outString, 256, "Aperture: %0.1f", gCamera.aperture);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
  	sprintf_s (outString, 256, "Voids: %0.1f", type3*100);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
      sprintf_s (outString, 256, " Sheets: %0.1f", type2*100);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
     sprintf_s (outString, 256, " Filaments: %0.1f", type1*100);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf_s (outString, 256, " Nodes: %0.1f", type0*100);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
		sprintf_s (outString, 256, "Environment Lambda cut: %.3f", (double)lambda_cut);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
    sprintf_s (outString, 256, "Radius mode: %.3f  1 = Viral R  0 = 500c", (double)radiusmode);
		drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);

    if (tweb) {
         sprintf_s (outString, 256, "Displaying T-WEB");
    } else {
         sprintf_s (outString, 256, "Displaying V-WEB");
    }
    drawGLString (10, window_height - (lineSpacing * line++) - startOffest, outString);
	}


	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void init (void) {
	 
	glewExperimental = GL_TRUE;
	glewInit();
    glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glShadeModel(GL_SMOOTH);
    glFrontFace(GL_CCW);
	glDepthFunc(GL_LESS);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(1.0,1.0,1.0);

    glClearColor(0.0,0.0,0.0,1.0);         /* Background recColor */
    gCameraReset ();
	
    glPolygonOffset (1.0, 1.0);
    SetLighting(4);
    glEnable(GL_LIGHTING);
}

void reshape (int w, int h) {
    glViewport(0,0,(GLsizei)w,(GLsizei)h);
    gCamera.screenWidth = w;
    gCamera.screenHeight = h;
    glutPostRedisplay();
}



int getHaloIndex(float x, float y , float z, int index) {
    int i, j;
    float dx, dy, dz, r, minR;
    halos = hlist.halos;
    num_halos = hlist.num_halos;
    minR = 100000;
    j = 0;
	  for (i=0; i<num_halos; i++) {
        dx = halos[i].x - x;
        dy = halos[i].y - y;
        dz = halos[i].z - z;
        r = dx*dx + dy*dy + dz*dz;
        if ( r < minR ) {
            minR = r;
            j = i;
        }
    }
    return j;
}


void rescale_bolshoi_halos() {
    int i, j;
    float scale_conv, inv_distance_scale, translation;
  
    scale_conv = 1.0/(2000 * xMAX);
    inv_distance_scale = 1.0/(xMAX * 3);
    translation = 0.0;


        halos = hlist.halos;
        num_halos = hlist.num_halos;

        for (i=0; i<num_halos; i++) {
          	halos[i].rvir *= scale_conv;
            halos[i].x = halos[i].x * inv_distance_scale - translation;
            halos[i].y = halos[i].y * inv_distance_scale - translation;
            halos[i].z = halos[i].z * inv_distance_scale - translation;
        }
}

void rescale_filaments() {
    int i, j;
    float scale_conv, inv_distance_scale, translation;
  
    scale_conv = 1.0/(2000 * xMAX);
    inv_distance_scale = 1.0/(xMAX * 3);
    translation = 0.0;
	for (i=0; i<flist0.size(); i++) {
            flist0[i].x = flist0[i].x * inv_distance_scale - translation;
            flist0[i].y = flist0[i].y * inv_distance_scale - translation;
            flist0[i].z = flist0[i].z * inv_distance_scale - translation;
    }
	for (i=0; i<flist1.size(); i++) {
            flist1[i].x = flist1[i].x * inv_distance_scale - translation;
            flist1[i].y = flist1[i].y * inv_distance_scale - translation;
            flist1[i].z = flist1[i].z * inv_distance_scale - translation;
    }
	for (i=0; i<flist2.size(); i++) {
            flist2[i].x = flist2[i].x * inv_distance_scale - translation;
            flist2[i].y = flist2[i].y * inv_distance_scale - translation;
            flist2[i].z = flist2[i].z * inv_distance_scale - translation;
    }
}

void rescale_walls() {
    int i, j;
    float scale_conv, inv_distance_scale, translation;
  
    scale_conv = 1.0/(2000 * xMAX);
    inv_distance_scale = 1.0/(xMAX * 3);
    translation = 0.0;
	for (i=0; i<wlist0.size(); i++) {
            wlist0[i].x = wlist0[i].x * inv_distance_scale - translation;
            wlist0[i].y = wlist0[i].y * inv_distance_scale - translation;
            wlist0[i].z = wlist0[i].z * inv_distance_scale - translation;
    }
	for (i=0; i<wlist1.size(); i++) {
            wlist1[i].x = wlist1[i].x * inv_distance_scale - translation;
            wlist1[i].y = wlist1[i].y * inv_distance_scale - translation;
            wlist1[i].z = wlist1[i].z * inv_distance_scale - translation;
    }
	for (i=0; i<wlist2.size(); i++) {
            wlist2[i].x = wlist2[i].x * inv_distance_scale - translation;
            wlist2[i].y = wlist2[i].y * inv_distance_scale - translation;
            wlist2[i].z = wlist2[i].z * inv_distance_scale - translation;
    }
	for (i=0; i<slist402.size(); i++) {
            slist402[i].x = slist402[i].x * inv_distance_scale - translation;
            slist402[i].y = slist402[i].y * inv_distance_scale - translation;
            slist402[i].z = slist402[i].z * inv_distance_scale - translation;
    }
	for (i=0; i<slist402_wall.size(); i++) {
            slist402_wall[i].x = slist402_wall[i].x * inv_distance_scale - translation;
            slist402_wall[i].y = slist402_wall[i].y * inv_distance_scale - translation;
            slist402_wall[i].z = slist402_wall[i].z * inv_distance_scale - translation;
    }
}




void process_bolshoi_halos() {
    int i, j;

	  
        halos = hlist.halos;
        num_halos = hlist.num_halos;

        for (i=0; i<num_halos; i++) {
             if (halos[i].x > xMAX) { xMAX = halos[i].x; }
             if (halos[i].x < xMIN) { xMIN = halos[i].x; }
 
             halos[i].vx = 0.0;         
        
		}
}





void build_sphere_lists(void) {
	gComplexSphereList = glGenLists(1);
	gMediumSphereList = glGenLists(1);
	gSimpleSphereList = glGenLists(1);
	
	glNewList(gComplexSphereList, GL_COMPILE);
	sphere(2);
	glEndList();
	
	glNewList(gMediumSphereList, GL_COMPILE);
	sphere(1);
	glEndList();
	
	glNewList(gSimpleSphereList, GL_COMPILE);
	sphere(0);
	glEndList();
	
	gCubeList = glGenLists(1);
	glNewList(gCubeList, GL_COMPILE);
	render_Cube2();
	glEndList();
	
	
}






//format - x y z mvir rvir b_to_a c_to_a xrot zrot b_to_a_500c c_to_a_500c xrot500c zrot500c Te1 Te2 Te3 Vse1 Vse2 Vse3
void load_bolshoi_env_halos(char *fn)
{
	int i, j = 0;
	char buffer[4000];
	FILE *input;
	halo h;


  
  
   hlist.halos = (halo *)realloc(hlist.halos, sizeof(halo)*(1000000));

  fopen_s(&input, fn, "r");
	if (!input) {
		printf("Failed to open file %s for reading!\n", fn);
		exit(1);
	}
		
	while (fgets(buffer, 4000, input)) {
		if (buffer[0] == '#') continue;
    i = sscanf_s(buffer, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
				   &(h.scale), &(h.id), &(h.mvir), &(h.descid), &(h.rvir), 
				   &(h.rs), &(h.almm), &(h.vmax), &(h.x), 
           &(h.y), &(h.z), &(h.vx), &(h.vy),
		   &(h.vz), &(h.lambda_peebles), 
		   &(h.tidal_force), &(h.rs_klypin), 
		   &(h.x_off), &(h.v_off), 
		   &(h.lambda_bullock), &(h.bta), &(h.cta), 
		   &(h.bta500c), &(h.cta500c), &(h.ttu), 
		   &(h.xrot), &(h.zrot));
		if (i<19) {
			printf("Couldn't parse line!\n%s\n", buffer);
			continue;
		} 
    if (h.cta == 0 || h.bta == 0) {
      h.cta = 1;
      h.bta = 1;
    }
    hlist.halos[j] = h;
    j++;
  }

  hlist.num_halos = j;
	fclose(input);
}
  
void render_bolshoi_halos(void) {
	point offset;
	float scale, dx, dy, dz, alpha, color_scale;
	float scale_conv, inv_distance_scale;
	int i;
 

  glEnable(GL_NORMALIZE);
  halos = hlist.halos;
  num_halos = hlist.num_halos;

	for (i=0; i<num_halos; i++) {
		glPushMatrix();
		scale = halos[i].rvir;
		
		offset.x = halos[i].x ;
		offset.y = halos[i].y ;
		offset.z = halos[i].z ;
		
		
		//printf("\nx: %f", halos[i].x); 
		//printf("\ny: %f", halos[i].y);
		//printf("\nz: %f", halos[i].z);
		
		if (halos[i].id == 5){
			  glColor4f(1.0f, 0.0f, 0.0f, haloAlpha);
		}
		else {
			  glColor4f(1.0f, 1.0f, 1.0f, haloAlpha);
		}
	 
   

	   glTranslatef(halos[i].x ,halos[i].y, halos[i].z );

 
        glRotatef(halos[i].xrot, 1.0, 0.0, 0.0);
        glRotatef(halos[i].zrot, 0.0, 0.0, 1.0);

		//glScalef(scale*halos[i].bta, scale, scale*halos[i].cta);
		glScalef(scale, scale, scale);
		
    
		
	//printf("\nOther Halo: %f", scale);
	//printf("\n scale * halos[i].bta500c: %f", scale*halos[i].bta500c);
	//printf("\n scale: %f", scale);
	//printf("\n scale * halos[i].cta500c: %f", scale*halos[i].cta500c);
	                     
	//printf("\n offset.x: %f", offset.x);
	//printf("\n offset.y: %f", offset.y);
	//printf("\n offset.z: %f", offset.z);
	
	

		

		glCallList(gMediumSphereList);

		glPopMatrix();


	}
  
}

void freeVoxelData(int dim) {

	for (int i = 0; i < dim; i++) {
		for (int j = 0; j < dim; j++) {
			free(voxelData[i][j]);
		}
		free(voxelData[i]);
	}
	free(voxelData);
}

void load_filamentVoxels0(){
	int i, j, k;
	
	
	// file name of input file
	std::string fileName = "Data/Filaments/subFilamentCube10044_smallerVolume.bin";
	
	// dimension of input file (e.g. 128^3 or 1024^3)
	
	std::cout << "Reading binary file: \"" << fileName << "\"" << std::endl;

	// open input file stream (may need <iostream> <fstream>)
	std::ifstream inpt(fileName,std::ios::in|std::ios::binary);

	int dim = 0;

	inpt.read((char *)&dim, sizeof(int));

	printf("%d\n", dim);
	

	// create variable to store data. allocate dynamically so
	// that its easier to change dimensions if desired.
	voxelData = (int ***) malloc (dim * sizeof (int **));

	// keeps track of progress reading file
	int dot_freq = int(ceil(dim/10.));



	// allocate space for data and read in one dimension at a time
	for ( i = 0; i < dim; i++) {
		if (i % dot_freq == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		voxelData[i] = (int **)malloc(dim * sizeof(int *));

		for ( j = 0; j < dim; j++) {

			voxelData[i][j] = (int *)malloc(dim * sizeof(int));
			memset(voxelData[i][j],0,dim * sizeof(int));
			inpt.read((char *)voxelData[i][j],dim * sizeof(int));
		}   
	}	
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			for (k = 0; k < dim; k++){
				
				if (voxelData[i][j][k] != -1) {
					voxel v;
					
					
					v.x = (i +0.5) * (250.0/1024.0); 
					v.y = (j +0.5) * (250.0/1024.0); 
					v.z = (k +0.5) * (250.0/1024.0);
					flist0.push_back(v);
				}		
			}

	inpt.close();
	std::cout << " \ncomplete." << std::endl;
	 
	freeVoxelData(dim);
}
void load_filamentVoxels1(){
	int i, j, k;
	int dim = 128;
	
	// file name of input file
	std::string fileName = "Data/Filaments/spine.fila.1.0-128.bin";
	
	// dimension of input file (e.g. 128^3 or 1024^3)
	
	std::cout << "Reading binary file: \"" << fileName << "\"" << std::endl;

	// open input file stream (may need <iostream> <fstream>)
	std::ifstream inpt(fileName,std::ios::in|std::ios::binary);

	// create variable to store data. allocate dynamically so
	// that its easier to change dimensions if desired.
	voxelData = (int ***) malloc (dim * sizeof (int **));

	// keeps track of progress reading file
	int dot_freq = int(ceil(dim/10.));

	// allocate space for data and read in one dimension at a time
	for ( i = 0; i < dim; i++) {
		if (i % dot_freq == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		voxelData[i] = (int **)malloc(dim * sizeof(int *));

		for ( j = 0; j < dim; j++) {

			voxelData[i][j] = (int *)malloc(dim * sizeof(int));
			memset(voxelData[i][j],0,dim * sizeof(int));
			inpt.read((char *)voxelData[i][j],dim * sizeof(int));
		}   
	}	
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			for (k = 0; k < dim; k++){
				
				if (voxelData[i][j][k] != -1) {
					voxel v;
					
					
					v.x = (i +0.5) * (250.0/1024.0); 
					v.y = (j +0.5) * (250.0/1024.0); 
					v.z = (k +0.5) * (250.0/1024.0);
					flist1.push_back(v);
				}		
			}

	inpt.close();
	std::cout << " \ncomplete." << std::endl;

	freeVoxelData(dim);
}
void load_filamentVoxels2(){
	int i, j, k;
	int dim = 128;
	
	// file name of input file
	std::string fileName = "Data/Filaments/spine.fila.2.0-128.bin";
	
	// dimension of input file (e.g. 128^3 or 1024^3)
	
	std::cout << "Reading binary file: \"" << fileName << "\"" << std::endl;

	// open input file stream (may need <iostream> <fstream>)
	std::ifstream inpt(fileName,std::ios::in|std::ios::binary);

	// create variable to store data. allocate dynamically so
	// that its easier to change dimensions if desired.
	voxelData = (int ***) malloc (dim * sizeof (int **));

	// keeps track of progress reading file
	int dot_freq = int(ceil(dim/10.));

	// allocate space for data and read in one dimension at a time
	for ( i = 0; i < dim; i++) {
		if (i % dot_freq == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		voxelData[i] = (int **)malloc(dim * sizeof(int *));

		for ( j = 0; j < dim; j++) {

			voxelData[i][j] = (int *)malloc(dim * sizeof(int));
			memset(voxelData[i][j],0,dim * sizeof(int));
			inpt.read((char *)voxelData[i][j],dim * sizeof(int));
		}   
	}	
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			for (k = 0; k < dim; k++){
				
				if (voxelData[i][j][k] != -1) {
					voxel v;
					
					
					v.x = (i +0.5) * (250.0/1024.0);
					v.y = (j +0.5) * (250.0/1024.0);
					v.z = (k +0.5) * (250.0/1024.0);
					flist2.push_back(v);
				}		
			}

	inpt.close();
	std::cout << " \ncomplete." << std::endl;

	freeVoxelData(dim);
}


void render_filamentVoxels0(std::vector<voxel> list) {
	point offset;
	float scale;
	int i;

  glEnable(GL_NORMALIZE);
  

  for (i=0; i<list.size(); i++) {
		glPushMatrix();
		//filamentScale = 0.002f;
		
		//printf("%f", scale);
		
		glTranslatef(list[i].x, list[i].y, list[i].z);
		 
		//printf("\nx: %f", flist[i].x);
		//printf("\ny: %f", flist[i].y);
		//printf("\nz: %f", flist[i].z);

		
		glColor4f(0.0f, 0.0f, 1.0f, filamentAlpha);
		
		glScalef(filamentScale0, filamentScale0, filamentScale0);

		glCallList(gSimpleSphereList);
		glPopMatrix();

		
	}

	
}
void render_filamentVoxels1(void) {
	point offset;
	float scale;
	int i;

  glEnable(GL_NORMALIZE);
  

  for (i=0; i<flist1.size(); i++) {
		glPushMatrix();
		//filamentScale = 0.002f;
		
		//printf("%f", scale);
		
		glTranslatef(flist1[i].x, flist1[i].y, flist1[i].z); 
		
		//printf("\nx: %f", flist1[i].x);
		//printf("\ny: %f", flist1[i].y);
	//	printf("\nz: %f", flist1[i].z);	
		
		glColor4f(1.0f, 1.0f, 0.0f, filamentAlpha);
		
		
		glScalef(filamentScale1, filamentScale1, filamentScale1);


		glCallList(gSimpleSphereList);
		glPopMatrix();

		
	}

	
}
void render_filamentVoxels2(std::vector<voxel> list) {
	point offset;
	float scale;
	int i;

  glEnable(GL_NORMALIZE);
  

  for (i=0; i<list.size(); i++) {
		glPushMatrix();
		//filamentScale = 0.002f;
		
		//printf("%f", scale);
		
		glTranslatef(list[i].x, list[i].y, list[i].z);
		 
		//printf("\nx: %f", flist[i].x);
		//printf("\ny: %f", flist[i].y);
		//printf("\nz: %f", flist[i].z);

		
		glColor4f(0.0f, 1.0f, 0.9f, filamentAlpha);
		
		
		glScalef(filamentScale2, filamentScale2, filamentScale2);


		glCallList(gSimpleSphereList);
		glPopMatrix();

		
	}

	
}

void load_wallVoxels0(){
	int i, j, k;
	int dim = 128;
	
	// file name of input file
	std::string fileName = "Data/Walls/spine.wall.0.0-128.bin";
	
	// dimension of input file (e.g. 128^3 or 1024^3)
	
	std::cout << "Reading binary file: \"" << fileName << "\"" << std::endl;

	// open input file stream (may need <iostream> <fstream>)
	std::ifstream inpt(fileName,std::ios::in|std::ios::binary);

	

	// create variable to store data. allocate dynamically so
	// that its easier to change dimensions if desired.
	voxelData = (int ***) malloc (dim * sizeof (int **));

	// keeps track of progress reading file
	int dot_freq = int(ceil(dim/10.));

	// allocate space for data and read in one dimension at a time
	for ( i = 0; i < dim; i++) {
		if (i % dot_freq == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		voxelData[i] = (int **)malloc(dim * sizeof(int *));

		for ( j = 0; j < dim; j++) {

			voxelData[i][j] = (int *)malloc(dim * sizeof(int));
			memset(voxelData[i][j],0,dim * sizeof(int));
			inpt.read((char *)voxelData[i][j],dim * sizeof(int));
		}   
	}	
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			for (k = 0; k < dim; k++){
				
				if (voxelData[i][j][k] != -1) {
					voxel v;
					
					
					v.x = (i +0.5) * (250.0/1024.0); 
					v.y = (j +0.5) * (250.0/1024.0); 
					v.z = (k +0.5) * (250.0/1024.0);
					wlist0.push_back(v);
				}		
			}

	inpt.close();
	std::cout << " \ncomplete." << std::endl;
	 
	freeVoxelData(dim);

}
void load_wallVoxels1(){
	int i, j, k;
	int dim = 128;
	
	// file name of input file
	std::string fileName = "Data/Walls/spine.wall.1.0-128.bin";
	
	// dimension of input file (e.g. 128^3 or 1024^3)
	
	std::cout << "Reading binary file: \"" << fileName << "\"" << std::endl;

	// open input file stream (may need <iostream> <fstream>)
	std::ifstream inpt(fileName,std::ios::in|std::ios::binary);

	// create variable to store data. allocate dynamically so
	// that its easier to change dimensions if desired.
	voxelData = (int ***) malloc (dim * sizeof (int **));

	// keeps track of progress reading file
	int dot_freq = int(ceil(dim/10.));

	// allocate space for data and read in one dimension at a time
	for ( i = 0; i < dim; i++) {
		if (i % dot_freq == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		voxelData[i] = (int **)malloc(dim * sizeof(int *));

		for ( j = 0; j < dim; j++) {

			voxelData[i][j] = (int *)malloc(dim * sizeof(int));
			memset(voxelData[i][j],0,dim * sizeof(int));
			inpt.read((char *)voxelData[i][j],dim * sizeof(int));
		}   
	}	
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			for (k = 0; k < dim; k++){
				
				if (voxelData[i][j][k] != -1) {
					voxel v;
					
					
					
					v.x = (i +0.5) * (250.0/1024.0); 
					v.y = (j +0.5) * (250.0/1024.0); 
					v.z = (k +0.5) * (250.0/1024.0);
					wlist1.push_back(v);
				}		
			}

	inpt.close();
	std::cout << " \ncomplete." << std::endl;
	 
	freeVoxelData(dim);

}
void load_wallVoxels2(){
	int i, j, k;
	int dim = 128;
	
	// file name of input file
	std::string fileName = "Data/Walls/spine.wall.2.0-128.bin";
	
	// dimension of input file (e.g. 128^3 or 1024^3)
	
	std::cout << "Reading binary file: \"" << fileName << "\"" << std::endl;

	// open input file stream (may need <iostream> <fstream>)
	std::ifstream inpt(fileName,std::ios::in|std::ios::binary);

	// create variable to store data. allocate dynamically so
	// that its easier to change dimensions if desired.
	voxelData = (int ***) malloc (dim * sizeof (int **));

	// keeps track of progress reading file
	int dot_freq = int(ceil(dim/10.));

	// allocate space for data and read in one dimension at a time
	for ( i = 0; i < dim; i++) {
		if (i % dot_freq == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		voxelData[i] = (int **)malloc(dim * sizeof(int *));

		for ( j = 0; j < dim; j++) {

			voxelData[i][j] = (int *)malloc(dim * sizeof(int));
			memset(voxelData[i][j],0,dim * sizeof(int));
			inpt.read((char *)voxelData[i][j],dim * sizeof(int));
		}   
	}	
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			for (k = 0; k < dim; k++){
				
				if (voxelData[i][j][k] != -1) {
					voxel v;
					
					
					
					v.x = (i +0.5) * (250.0/1024.0); 
					v.y = (j +0.5) * (250.0/1024.0); 
					v.z = (k +0.5) * (250.0/1024.0);
					wlist2.push_back(v);
				}		
			}

	inpt.close();
	std::cout << " \ncomplete." << std::endl;
	 
	freeVoxelData(dim);

}

void render_wallVoxels0(void) {
	point offset;
	float scale;
	int i;

	glEnable(GL_NORMALIZE);

	for (i = 0; i<wlist0.size(); i++) {
		glPushMatrix();
		
		//printf("%f", scale);

		glTranslatef(wlist0[i].x, wlist0[i].y, wlist0[i].z);
		glColor4f(1.0f, 0.4f, 0.0f, wallAlpha);
		glScalef(wallScale, wallScale, wallScale);

		glCallList(gSimpleSphereList);
		glPopMatrix();


	}


}
void render_wallVoxels1(void) {
	point offset;
	float scale;
	int i;

	glEnable(GL_NORMALIZE);

	for (i = 0; i<wlist1.size(); i++) {
		glPushMatrix();
		
		//printf("%f", scale);

		glTranslatef(wlist1[i].x, wlist1[i].y, wlist1[i].z);
		glColor4f(1.0f, 0.0f, 0.0f, wallAlpha);
		glScalef(wallScale, wallScale, wallScale);

		glCallList(gSimpleSphereList);
		glPopMatrix();


	}


}
void render_wallVoxels2(void) {
	point offset;
	float scale;
	int i;

	glEnable(GL_NORMALIZE);

	for (i = 0; i<wlist2.size(); i++) {
		glPushMatrix();
		
		//printf("%f", scale);

		glTranslatef(wlist2[i].x, wlist2[i].y, wlist2[i].z);
		glColor4f(1.0f, 0.0f, 1.0f, wallAlpha);
		glScalef(wallScale, wallScale, wallScale);

		glCallList(gSimpleSphereList);
		glPopMatrix();


	}


}


void load_subWall_4577(){
	int i, j, k;
	//int dim = 31;
	
	// file name of input file
	std::string fileName = "Data/Walls/subCube10044_smallerVolume.bin";
	
	// dimension of input file (e.g. 128^3 or 1024^3)
	
	std::cout << "Reading binary file: \"" << fileName << "\"" << std::endl;

	// open input file stream (may need <iostream> <fstream>)
	std::ifstream inpt(fileName,std::ios::in|std::ios::binary);

	int dim = 0;

	inpt.read((char *)&dim, sizeof(int));

	printf("%d\n", dim);
	

	// create variable to store data. allocate dynamically so
	// that its easier to change dimensions if desired.
	voxelData = (int ***) malloc (dim * sizeof (int **));

	// keeps track of progress reading file
	int dot_freq = int(ceil(dim/10.));

	// allocate space for data and read in one dimension at a time
	for ( i = 0; i < dim; i++) {
		if (i % dot_freq == 0) {
			std::cout << ".";
			std::cout.flush();
		}
		voxelData[i] = (int **)malloc(dim * sizeof(int *));

		for ( j = 0; j < dim; j++) {

			voxelData[i][j] = (int *)malloc(dim * sizeof(int));
			memset(voxelData[i][j],0,dim * sizeof(int));
			inpt.read((char *)voxelData[i][j],dim * sizeof(int));
		}   
	}	
	for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			for (k = 0; k < dim; k++){
				
				if (voxelData[i][j][k] != -1) {

					if (voxelData[i][j][k] == 10044){
						voxel v1;
						v1.x = (i +0.5) * (250.0/1024.0); 
						v1.y = (j +0.5) * (250.0/1024.0); 
						v1.z = (k +0.5) * (250.0/1024.0);
						slist402_wall.push_back(v1);
					}
					else {
						voxel v;
						v.x = (i +0.5) * (250.0/1024.0); 
						v.y = (j +0.5) * (250.0/1024.0); 
						v.z = (k +0.5) * (250.0/1024.0);
						slist402.push_back(v);
					}
				}		
			}

	inpt.close();
	std::cout << " \ncomplete." << std::endl;
	 
	freeVoxelData(dim);

}
void render_subCube4577(void) {
	point offset;
	float scale;
	int i;

	glEnable(GL_NORMALIZE);

	if (toggleWallID == TRUE) {

		for (i = 0; i<slist402.size(); i++) {
			glPushMatrix();
		
			//printf("%f", scale);

			glTranslatef(slist402[i].x, slist402[i].y, slist402[i].z);

			glColor4f(1.0f, 0.4f, 0.0f, wallAlpha - .6f);
			glScalef(wallScale*(0.7f), wallScale*(0.7f), wallScale*(0.7f));

			glCallList(gSimpleSphereList);
			glPopMatrix();


		}

	}

	for (i = 0; i<slist402_wall.size(); i++) {
		glPushMatrix();
		
		//printf("%f", scale);

		glTranslatef(slist402_wall[i].x, slist402_wall[i].y, slist402_wall[i].z);

		glColor4f(0.0f, 1.0f, 0.0f, wallAlpha);
		glScalef(wallScale, wallScale, wallScale);

		glCallList(gSimpleSphereList);
		glPopMatrix();


	}
	


}

void setupProjectionMatrix(GLboolean eye) {
    
  GLdouble xmin, xmax, ymin, ymax;
    // far frustum plane
    GLdouble zFar = -gCamera.viewPos.z + gShapeSize * 0.5;
    // near frustum plane clamped at 1.0
    GLdouble zNear = MIN (-gCamera.viewPos.z - gShapeSize * 0.5, 1.0);
    // window aspect ratio
    GLdouble aspect = gCamera.screenWidth / (GLdouble)gCamera.screenHeight; 

    GLdouble eyeOffSet = KINECT_EYE_OFFSET/2.0 + eye_adjust;
    if (eye) eyeOffSet = -KINECT_EYE_OFFSET/2.0 - eye_adjust;
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    zNear = 0.01;
    zFar = 1.5;

    if (aspect > 1.0) {
	ymax = zNear * tan (gCamera.aperture * 0.5 * DTOR);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
    } else {
	xmax = zNear * tan (gCamera.aperture * 0.5 * DTOR);
	xmin = -xmax;
	ymin = xmin / aspect;
	ymax = xmax / aspect;
    }
    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);

    glTranslatef(eyeOffSet, 0.0,0.0);
    
  
}

void setupModelViewMatrix(GLboolean eye) {
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    gluLookAt (gCamera.viewPos.x, gCamera.viewPos.y, gCamera.viewPos.z,
      /*gCamera.viewPos.x +*/ 0.0,//-gCamera.viewDir.x,
      /*gCamera.viewPos.y +*/ 0.0, //gCamera.viewDir.y,
			/*gCamera.viewPos.z +*/  0.0,//gCamera.viewDir.z,
			gCamera.viewUp.x, gCamera.viewUp.y ,gCamera.viewUp.z);	
  
    // track ball rotation
    glRotatef (gTrackBallRotation[0], gTrackBallRotation[1], gTrackBallRotation[2], gTrackBallRotation[3]);	
    glRotatef (gWorldRotation[0], gWorldRotation[1], gWorldRotation[2], gWorldRotation[3]);
    if (rotation_enabled) {
        rotation_scale += rotation_degrees;
    }
    rotate(rotation_scale, rotation_direction[0], rotation_direction[1], rotation_direction[2]);
}

void setupScene(GLboolean eye) {
   
	
	glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
	
    setupProjectionMatrix(eye);
    setupModelViewMatrix(eye);
    
    if (eye) {
        glDrawBuffer(GL_BACK_RIGHT);
    }
    else {
        glDrawBuffer(GL_BACK_LEFT);
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
  
    glClearColor (0.0f, 0.0f, 0.1f, 1.0f);	// clear the surface
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
	

	if (renderHalo == TRUE) {
		render_bolshoi_halos();
		//render_bolshoi_halos2();
	}
	if (renderFilament == TRUE) {
		if (renderFilament2 == TRUE)
			render_filamentVoxels2(flist2);
		if (renderFilament1 == TRUE)
			render_filamentVoxels1();
		if (renderFilament0 == TRUE)
			render_filamentVoxels0(flist0);
	}
   
	if (renderWall == TRUE) {
		if (renderWall2 == TRUE)
			render_wallVoxels2();
		if (renderWall1 == TRUE)
			render_wallVoxels1();
		if (renderWall0 == TRUE)
			render_wallVoxels0();
		if (renderSubCube4577 == TRUE)
			render_subCube4577();
	}
  
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    drawMainText (gCamera.screenWidth, gCamera.screenHeight);
}

void maindisplay(void)
{
	setupScene(TRUE);
    setupScene(FALSE);
    glutSwapBuffers();
	//glutPostRedisplay();
}

void special(int key, int px, int py)
{
  gLastKey = key;
  switch (key) {
	case GLUT_KEY_UP: // arrow forward, close in on world
		gCamera.focalLength -= 0.5f;
		if (gCamera.focalLength < 0.0f)
			gCamera.focalLength = 0.0f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN: // arrow back, back away from world
		gCamera.focalLength += 0.5f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT: // arrow left, smaller aperture
		gCamera.aperture -= 1.0f;
		if (gCamera.aperture < 0.0f)
			gCamera.aperture = 0.0f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT: // arrow right, larger aperture
		gCamera.aperture += 1.0f;
		glutPostRedisplay();
		break;
	
  }
  
}

void mouseDolly (int x, int y)
{
	if (gDolly) {
		GLfloat dolly = (gDollyPanStartPoint[1] - y) * -gCamera.viewPos.z / 200.0f;
		GLfloat eyeRelative = gCamera.eyeSep / gCamera.focalLength;
		gCamera.focalLength += gCamera.focalLength / gCamera.viewPos.z * dolly; 
		if (gCamera.focalLength < 1.0)
			gCamera.focalLength = 1.0;
		gCamera.eyeSep = gCamera.focalLength * eyeRelative;
		gCamera.viewPos.z += dolly;
		if (gCamera.viewPos.z == 0.0) // do not let z = 0.0
			gCamera.viewPos.z = 0.0001;
		gDollyPanStartPoint[0] = x;
		gDollyPanStartPoint[1] = y;
		glutPostRedisplay();
	}
}

void mousePan (int x, int y)
{
	if (gPan) {
		GLfloat panX = (gDollyPanStartPoint[0] - x) / (900.0f / -gCamera.viewPos.z);
		GLfloat panY = (gDollyPanStartPoint[1] - y) / (900.0f / -gCamera.viewPos.z);
		gCamera.viewPos.x -= panX;
		gCamera.viewPos.y -= panY;
		gDollyPanStartPoint[0] = x;
		gDollyPanStartPoint[1] = y;
		glutPostRedisplay();
	}
}

void mouseTrackball (int x, int y)
{
	if (gTrackBall) {
		rollToTrackball (x, y, gTrackBallRotation);
		glutPostRedisplay();
	}
}

void mouse (int button, int state, int x, int y)
{
  mouseX = x;
  mouseY = y;
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		if (gDolly) { // if we are currently dollying, end dolly
			mouseDolly (x, y);
			gDolly = GL_FALSE;
			glutMotionFunc (NULL);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
			glutMotionFunc (NULL);
		} else if (gPan) {
			mousePan (x, y);
			gPan = GL_FALSE;
			glutMotionFunc (NULL);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
			glutMotionFunc (NULL);
		}
    startTrackball (x, y, 0.0, 0.0, gCamera.screenWidth, gCamera.screenHeight);
		glutMotionFunc (mouseTrackball);
		gTrackBall = GL_TRUE;
	} else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		gTrackBall = GL_FALSE;
		glutMotionFunc (NULL);
		rollToTrackball (x, y, gTrackBallRotation);
		if (gTrackBallRotation[0] != 0.0)
			addToRotationTrackball (gTrackBallRotation, gWorldRotation);
		gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		if (gTrackBall) {// if we are currently trackballing, end trackball
			gTrackBall = GL_FALSE;
			glutMotionFunc (NULL);
			rollToTrackball (x, y, gTrackBallRotation);
			if (gTrackBallRotation[0] != 0.0)
				addToRotationTrackball (gTrackBallRotation, gWorldRotation);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
		} else if (gPan) {
			mousePan (x, y);
			gPan = GL_FALSE;
			glutMotionFunc (NULL);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
			glutMotionFunc (NULL);
		}
		gDollyPanStartPoint[0] = x;
		gDollyPanStartPoint[1] = y;
		glutMotionFunc (mouseDolly);
		gDolly = GL_TRUE;
	} else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
		mouseDolly (x, y);
		gDolly = GL_FALSE;
		glutMotionFunc (NULL);
		gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
		glutMotionFunc (NULL);
	}
	else if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN)) {
		if (gTrackBall) {// if we are currently trackballing, end trackball
			gTrackBall = GL_FALSE;
			glutMotionFunc (NULL);
			rollToTrackball (x, y, gTrackBallRotation);
			if (gTrackBallRotation[0] != 0.0)
				addToRotationTrackball (gTrackBallRotation, gWorldRotation);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
		} else if (gDolly) {
			mouseDolly (x, y);
			gDolly = GL_FALSE;
			glutMotionFunc (NULL);
			gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
			glutMotionFunc (NULL);
		}
		gDollyPanStartPoint[0] = x;
		gDollyPanStartPoint[1] = y;
		glutMotionFunc (mousePan);
		gPan = GL_TRUE;
	} else if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP)) {
		mousePan (x, y);
		gPan = GL_FALSE;
		glutMotionFunc (NULL);
		gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
		glutMotionFunc (NULL);
	}
}


void key(unsigned char inkey, int px, int py)
{
  gLastKey = inkey;
  switch (inkey) {
	case 27:
	case 'q':
	case 'Q':
		exit(0);
		break;
	case 'z': // toggle wallID
		if (toggleWallID == TRUE)
			toggleWallID = FALSE;
		else if (toggleWallID == FALSE)
			toggleWallID = TRUE;
		glutPostRedisplay();
		break;	
	case '1': // render filament smoothing 1
		if (renderFilament0 == TRUE)
			renderFilament0 = FALSE;
		else if (renderFilament0 == FALSE)
			renderFilament0 = TRUE;
		glutPostRedisplay();
		break;	
	case '2': // render filament smoothing 1
		if (renderFilament1 == TRUE)
			renderFilament1 = FALSE;
		else if (renderFilament1 == FALSE)
			renderFilament1 = TRUE;
		glutPostRedisplay();
		break;
	case '3': // render filament smoothing 1
		if (renderFilament2 == TRUE)
			renderFilament2 = FALSE;
		else if (renderFilament2 == FALSE)
			renderFilament2 = TRUE;
		glutPostRedisplay();
		break;
	case '4': // render wall smoothing 1
		if (renderWall0 == TRUE)
			renderWall0 = FALSE;
		else if (renderWall0 == FALSE)
			renderWall0 = TRUE;
		glutPostRedisplay();
		break;
	case '5': // render wall smoothing 1
		if (renderWall1 == TRUE)
			renderWall1 = FALSE;
		else if (renderWall1 == FALSE)
			renderWall1 = TRUE;
		glutPostRedisplay();
		break;
	case '6': // render wall smoothing 1
		if (renderWall2 == TRUE)
			renderWall2 = FALSE;
		else if (renderWall2 == FALSE) 
			renderWall2 = TRUE;
		glutPostRedisplay();
		break;
	case'k': // increase alpha of filaments
		filamentAlpha += 0.1f;
		if (filamentAlpha > 1.0f)
			filamentAlpha = 1.0f;
		glutPostRedisplay();
		break;
	case'j': // decrease alpha of filaments
		filamentAlpha -= 0.1f;
		if (filamentAlpha < 0.0f)
			filamentAlpha = 0.0f;
		glutPostRedisplay();
		break;
	case'g': // decrease scale of filaments
		filamentScale0 -= 0.0005f;
		if (filamentScale0 < 0.0f)
			filamentScale0 = 0.0f;
		filamentScale1 -= 0.0005f;
		if (filamentScale1 < 0.0f)
			filamentScale1 = 0.001f;
		filamentScale2 -= 0.0005f;
		if (filamentScale2 < 0.0f)
			filamentScale2 = 0.002f;
		glutPostRedisplay();
		break;
	case'h': // increase scale of filaments
		filamentScale0 += 0.001f;
		if (filamentScale0 > 0.05f)
			filamentScale0 = 0.05f;
		filamentScale1 += 0.001f;
		if (filamentScale1 > 0.05f)
			filamentScale1 = 0.05f;
		filamentScale2 += 0.001f;
		if (filamentScale2 > 0.05f)
			filamentScale2 = 0.05f;
		glutPostRedisplay();
		break;
	case'f': // toggle filaments
		if (renderFilament == TRUE)
			renderFilament = FALSE;
		else if (renderFilament == FALSE)
			renderFilament = TRUE;
		glutPostRedisplay();
		break;
	case'y': // increase alpha of walls
		wallAlpha += 0.1f;
		if (wallAlpha > 1.0f)
			wallAlpha = 1.0f;
		glutPostRedisplay();
		break;
	case't': // decrease alpha of walls
		wallAlpha -= 0.1f;
		if (wallAlpha < 0.0f)
			wallAlpha = 0.0f;
		glutPostRedisplay();
		break;
	case'e': // decrease scale of walls
		wallScale -= 0.0005f;
		if (wallScale < 0.0f)
			wallScale = 0.0f;
		glutPostRedisplay();
		break;
	case'r': // increase scale of walls
		wallScale += 0.0005f;
		if (wallScale > 0.05f)
			wallScale = 0.05f;
		glutPostRedisplay();
		break;
	case'w': // toggle walls
		if (renderWall == TRUE)
			renderWall = FALSE;
		else if (renderWall == FALSE)
			renderWall = TRUE;
		glutPostRedisplay();
		break;
	case'x': // toggle halos
		if (renderHalo == TRUE)
			renderHalo = FALSE;
		else if (renderHalo == FALSE)
			renderHalo = TRUE;
		glutPostRedisplay();
		break;
	case'c': // decrease alpha of halos
		haloAlpha -= 0.1f;
		if (haloAlpha < 0.0f)
			haloAlpha = 0.0f;
		glutPostRedisplay();
		break;
	case'v': // increase alpha of halos
		haloAlpha += 0.1f;
		if (haloAlpha > 1.0f)
			haloAlpha = 1.0f;
		glutPostRedisplay(); 
		break;
	case'7': // toggle wall 402
		if (renderSubCube4577 == TRUE)
			renderSubCube4577 = FALSE;
		else if (renderSubCube4577 == FALSE)
			renderSubCube4577 = TRUE;
		glutPostRedisplay();
		break;
	case'a': // toggle wall 402
		  if (fullScreen) {
			  glutReshapeWindow(oldScreenSize.x,oldScreenSize.y);
			  fullScreen=0;
		  }
		  else {
			  oldScreenSize.x = gCamera.screenWidth;
			  oldScreenSize.y = gCamera.screenHeight;
			  glutFullScreen();
			  fullScreen=1;
		  }
		glutPostRedisplay();
		break;
       
  }

}


int main (int argc, char * argv[])
{
	
	//setting diplsay modes and creating GL window
    glutInit(&argc, (char **)argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STEREO); // non-stereo for main window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize (1920, 1080);
    gMainWindow = glutCreateWindow("Halo Viewer");

	
	//standard GL init
    init(); 

  
    // Load Halo files and place in a halo_List struc
  //  load_bolshoi_env_halos("data/BP_Rockstar.1.00230.EnvTypes.N1024.s0.25.20-60FIXED.haloview");
//	load_bolshoi_env_halos("Data/Halos/halos.z0.0-128.dat");
	//load_bolshoi_env_halos("Data/Halos/ hlist_1.00231.list.vizformat.dat");
  
	readBinaryFile1D("Data/Halos/subHaloCube10044_smallerVolume.bin", hlist.halos, hlist.num_halos);
	


    load_filamentVoxels0();
	load_filamentVoxels1();
	load_filamentVoxels2();
	load_wallVoxels0();
	load_wallVoxels1();
	load_wallVoxels2();
	load_subWall_4577();

	printf("\n\nLoaded voxels\n\n");

    // Counts number of halos in the list of  halos, finds the x max and x min of each halo list which is then used for scaling
    process_bolshoi_halos();
	

	//makes complex, meduim, and simple sphere display lists using GlNewList and runs the subsequent commands for the given display list
    build_sphere_lists();
    
	// rescale x, y, z coordinates and distance scales
	rescale_bolshoi_halos();
	rescale_filaments();
	rescale_walls();
	
    glutReshapeFunc  (reshape); // callback triggered when window is resized 
    glutMouseFunc    (mouse); // callback triggered with mouse events
    glutDisplayFunc  (maindisplay); // maindisplay contains setupScene, which renders the halos, and also calls swapBuffers()
    glutKeyboardFunc (key); //callback triggered with key events
    glutSpecialFunc  (special); //allows users to exit program with esc key or 'q'

	 
   
    glutMainLoop(); //enters the GLUT processing loop, interrupted only by callbacks
    return 0;
}
