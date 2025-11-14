#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>


#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


// For initlist() ---------------------
#define XSIDE  20.f              // length of the x side of the grid
#define X0    (-XSIDE/2.f)       // where one side starts
#define NX     200                // how many points in x
#define DX    (XSIDE/(float)NX)   // change in x between the points
#define YFLOOR  (-3.f)
#define YGRID  (YFLOOR)        // y-height of the grid

#define ZSIDE	20.f		// length of the z side of the grid
#define Z0      (-ZSIDE/2.)		// where one side starts
#define NZ	     200 		// how many points in z
#define DZ	( ZSIDE/(float)NZ )	// change in z between the points

#define WALL_H   6.f            // wall height above the floor
#define Y0       (YFLOOR)       // wall base sits on the floor
#define NY       180            // how many points vertically
#define DY       (WALL_H/(float)NY)

// Ligt 
int LightMode = 0;  // 0 = point, 1 = spot
float LightX = 3.0f, LightY = 4.0f, LightZ = 2.0f;
float LightR = 1.0f, LightG = 1.0f, LightB = 1.0f;  // start white


// --------------------------- 

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"

//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Michelle Nieto

// title of these windows:

const char *WINDOWTITLE = "OpenGL / GLUT Sample -- Michelle Nieto";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 1000;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// Views 
enum Views { VIEW_OUTSIDE, VIEW_INSIDE };
int NowView;

// Global variables for the cow circle

const int   NUMSEGS = 24;      // more = smoother circle
const float RADIUS  = 2.0f;   // circle radius
// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 seconds


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	CircleList;				// object display list
GLuint Salmon;                 // Salmon object
GLuint Vase;                 
GLuint Teapot;    // Teapot object
GLuint GridDL;       // Walls
GLuint BackWallDL; 
GLuint RightWallDL; 
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

//menu 
void DoViewMenu(int);

// --- cake helpers (prototypes) ---
// void DrawDisk(float r, float y, int slices, bool top);
// void DrawCylinder(float r, float h, int slices);
/** void DrawTier(float r, float h, float y, int slices, float icingLip);
void BeadBorderCyl(float R, float y, int N, float beadR, float beadH, int slices, float cr, float cg, float cb);**/


void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);


// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}


float
Ranf( float low, float high )
{
        float r = (float) rand();               // 0 - RAND_MAX
        float t = r  /  (float) RAND_MAX;       // 0. - 1.

        return   low  +  t * ( high - low );
}

// call this if you want to force your program to use
// a different random number sequence every time you run it:
void
TimeOfDaySeed( )
{
	struct tm y2k;
	y2k.tm_hour = 0;    y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 2000; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  now;
	time( &now );
	double seconds = difftime( now, mktime(&y2k) );
	unsigned int seed = (unsigned int)( 1000.*seconds );    // milliseconds
	srand( seed );
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
//#include "osucube.cpp"
//#include "osucylindercone.cpp"
//#include "osutorus.cpp"
#include "bmptotexture.cpp"
#include "loadobjmtlfiles.cpp"
#include "keytime.cpp"
//#include "glslprogram.cpp"
//#include "vertexbufferobject.cpp"

// Keytime Xpositions
// Camera qualities
Keytimes EyeX, EyeY, LookY;

// Object #1 (Salmon): 3 quantities
Keytimes Salmon_Tx, Salmon_Ry, Salmon_S;

// Object #2 (Vase): 3 quantities
Keytimes Vase_Tz, Vase_Rx, Vase_S;


// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_SMOOTH);

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );

	if (NowView == VIEW_INSIDE) {
		// Inside view MUST be perspective
		gluPerspective(70.f, 1.f, 0.1f, 1000.f);
	} else {
		// Outside view: use user's selection (ortho or persp)
		if (NowProjection == ORTHO)
			glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
		else
			gluPerspective(70.f, 1.f, 0.1f, 1000.f);
	}

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:

	// place the objects into the scene:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	int msec   = glutGet(GLUT_ELAPSED_TIME) % MS_PER_CYCLE;  // 0..9999
	float nowSec = msec / 1000.f;                            // 0.0..10.0

	// --- camera per view ---
	if (NowView == VIEW_INSIDE) {
	
		float eyeY  = 0.3f;            // eye height
		float lookR = 8.f;            // look target distance
		float dir   = 0.0f;           
		gluLookAt(0.f, eyeY, 0.f,
				lookR * cosf(dir), 0.5f, lookR * sinf(dir),
				0.f, 1.f, 0.f);
				

	} else { // VIEW_OUTSIDE

    	float eyeX  = EyeX.GetValue(nowSec);
    	float eyeY  = EyeY.GetValue(nowSec);
    	float eyeZ  = 6.0f;
    	float lookX = 0.0f;
    	float lookY = LookY.GetValue(nowSec);
   		float lookZ = 0.0f;
    	gluLookAt(eyeX, eyeY, eyeZ,   lookX, lookY, lookZ,   0.f, 1.f, 0.f);

		//gluLookAt(0.f, 0.f, 6.f,    0.f, 0.0f, 0.f,   0.f, 1.f, 0.f);

		glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
		glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);

		// Apply user scale:
		if (Scale < MINSCALE) 
		Scale = MINSCALE;
		glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	}


	// set the fog parameters:
	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[NowColor][0] );
		glCallList( AxesList );
	}

	// --- LIGHT MOVEMENT ---
	float lightRadius = 3.f;
	float lightAngle  = 2.f * M_PI * Time; 
	float lx = lightRadius * cosf(lightAngle);
	float lz = lightRadius * sinf(lightAngle);
	float ly = 1.f;  

	glDisable(GL_LIGHTING);
	glColor3f(LightR, LightG, LightB);
	glPushMatrix();
		glTranslatef(lx, ly, lz);
		glutSolidSphere(0.1, 16, 16); 
	glPopMatrix();
	glEnable(GL_LIGHTING);

	// --- SET LIGHT ---
	// ----- PARTY LIGHT COLOR CHANGE -----
	float t = fmodf(nowSec, 3.0f) / 3.0f;   // 0.0 → 1.0 every 3 seconds

	// Convert that 0–1 range into RGB using sine waves (for smooth color shifts)
	float r = 0.5f + 0.5f * sinf(2.0f * M_PI * (t + 0.0f));  // Red
	float g = 0.5f + 0.5f * sinf(2.0f * M_PI * (t + 0.33f)); // Green (offset)
	float b = 0.5f + 0.5f * sinf(2.0f * M_PI * (t + 0.66f)); // Blue (offset)

	LightR = r;
	LightG = g;
	LightB = b;

	if (LightMode == 0) {
		SetPointLight(GL_LIGHT0, lx, ly, lz, LightR, LightG, LightB);
	} else {
		SetSpotLight(GL_LIGHT0, lx, ly, lz,
					-lx, -ly * 0.5f, -lz,
					LightR, LightG, LightB);
	}

	// since we are using glScalef( ), be sure the normals get unitized:

	//glEnable( GL_NORMALIZE );
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);


	SetMaterial(0.6f, 0.8f, 0.6f, 30.f);
	glCallList(GridDL);  

	glCallList(BackWallDL);
	glCallList(RightWallDL);

	// --- Salmon (Object #1): Tx, Ry, S ---
	SetMaterial(0.85f, 0.25f, 0.25f, 8.f);
	glPushMatrix();
		glTranslatef( Salmon_Tx.GetValue(nowSec), 0.f, 1.0f ); // animated X
		glRotatef(   Salmon_Ry.GetValue(nowSec), 0.f, 1.f, 0.f ); // animated Y-rot
		float s1 = Salmon_S.GetValue(nowSec);
		glScalef(s1, s1, s1);
		glCallList(Salmon);
	glPopMatrix();

	// --- Vase (Object #2): Tz, Rx, S ---
	SetMaterial(0.2f, 0.9f, 0.9f, 90.f);
	glPushMatrix();
		glTranslatef( -1.5f, 0.f, Vase_Tz.GetValue(nowSec) );   // animated Z
		glRotatef(   Vase_Rx.GetValue(nowSec), 1.f, 0.f, 0.f ); // animated X-rot
		float s2 = Vase_S.GetValue(nowSec);
		glScalef(s2, s2, s2);
		glCallList(Vase);
	glPopMatrix();

	// Teapot
	// SetMaterial(0.9f, 0.75f, 0.2f, 40.f);
	// glPushMatrix();
	//     glTranslatef( -1.8f, -0.8f, 0.f);
	//     glScalef(0.6f, 0.6f, 0.6f);
	//     glCallList(Teapot);
	// glPopMatrix();



	glDisable(GL_LIGHTING);


#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void DoViewMenu(int id) 
{ 
	NowView = id; 
	glutPostRedisplay(); 
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}

// Keyframes init

static void InitKeytimes() {
    // Camera
    EyeX.Init(); EyeX.AddTimeValue(0.0f,-3.5f); EyeX.AddTimeValue(1.5f,-1.5f); EyeX.AddTimeValue(3.5f,0.0f);
                 EyeX.AddTimeValue(6.5f, 2.0f); EyeX.AddTimeValue(8.5f, 1.0f); EyeX.AddTimeValue(10.0f,-3.5f);
    EyeY.Init(); EyeY.AddTimeValue(0.0f, 2.8f); EyeY.AddTimeValue(2.0f, 3.8f); EyeY.AddTimeValue(5.0f,1.8f);
                 EyeY.AddTimeValue(7.0f, 3.2f); EyeY.AddTimeValue(9.0f, 2.6f); EyeY.AddTimeValue(10.0f, 2.8f);
    LookY.Init();LookY.AddTimeValue(0.0f, 0.0f); LookY.AddTimeValue(2.0f, 0.5f); LookY.AddTimeValue(5.0f,-0.4f);
                 LookY.AddTimeValue(7.0f, 0.2f); LookY.AddTimeValue(9.0f, 0.1f); LookY.AddTimeValue(10.0f, 0.0f);

    // Salmon
    Salmon_Tx.Init(); Salmon_Tx.AddTimeValue(0.0f,-2.0f); Salmon_Tx.AddTimeValue(2.0f,-0.5f); Salmon_Tx.AddTimeValue(4.5f,1.2f);
                      Salmon_Tx.AddTimeValue(7.5f,2.2f);  Salmon_Tx.AddTimeValue(9.0f, 0.5f); Salmon_Tx.AddTimeValue(10.0f,-2.0f);
    Salmon_Ry.Init(); Salmon_Ry.AddTimeValue(0.0f,0.0f); Salmon_Ry.AddTimeValue(2.0f,120.0f); Salmon_Ry.AddTimeValue(4.0f,240.0f);
                      Salmon_Ry.AddTimeValue(6.0f,360.0f); Salmon_Ry.AddTimeValue(8.0f,520.0f); Salmon_Ry.AddTimeValue(10.0f,720.0f);
    Salmon_S.Init();  Salmon_S.AddTimeValue(0.0f,0.50f); Salmon_S.AddTimeValue(2.0f,0.70f); Salmon_S.AddTimeValue(4.0f,0.95f);
                      Salmon_S.AddTimeValue(6.5f,0.65f); Salmon_S.AddTimeValue(8.5f,0.80f); Salmon_S.AddTimeValue(10.0f,0.50f);

    // Vase
    Vase_Tz.Init();   Vase_Tz.AddTimeValue(0.0f, 2.0f); Vase_Tz.AddTimeValue(2.0f, 0.8f); Vase_Tz.AddTimeValue(4.5f,-0.5f);
                      Vase_Tz.AddTimeValue(7.0f,-1.8f); Vase_Tz.AddTimeValue(9.0f,-0.3f); Vase_Tz.AddTimeValue(10.0f, 2.0f);
    Vase_Rx.Init();   Vase_Rx.AddTimeValue(0.0f,0.0f);  Vase_Rx.AddTimeValue(2.0f,150.0f); Vase_Rx.AddTimeValue(5.0f,330.0f);
                      Vase_Rx.AddTimeValue(7.0f,480.0f); Vase_Rx.AddTimeValue(9.0f,600.0f); Vase_Rx.AddTimeValue(10.0f,720.0f);
    Vase_S.Init();    Vase_S.AddTimeValue(0.0f,0.40f); Vase_S.AddTimeValue(2.5f,0.55f); Vase_S.AddTimeValue(5.0f,0.80f);
                      Vase_S.AddTimeValue(7.5f,0.45f); Vase_S.AddTimeValue(9.0f,0.60f); Vase_S.AddTimeValue(10.0f,0.40f);
}


// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:
	InitKeytimes();

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );

	// create the object:

	// create the object (a circle)

	CircleList = glGenLists(1);
	glNewList(CircleList, GL_COMPILE);

		glColor3f(0.95f, 0.70f, 0.78f);
		float dang = 2.*M_PI/(float)(NUMSEGS-1);
		float ang = 0.;
		glBegin(GL_LINE_LOOP);
			for(int i = 0; i < NUMSEGS; i++)
			{
				glVertex3f(RADIUS*cos(ang), 0., RADIUS*sin(ang));
				ang += dang;
			}
		glEnd();

	glEndList();

	GridDL = glGenLists( 1 );
	glNewList( GridDL, GL_COMPILE );
			glShadeModel(GL_SMOOTH);
			glEnable(GL_NORMALIZE); 
			SetMaterial(0.6f, 0.8f, 0.6f, 80.f); 
        	glNormal3f( 0., 1., 0. );		// for each floor vertex, pointing straight up

        	for( int i = 0; i < NZ; i++ )
        	{
                	glBegin( GL_QUAD_STRIP );
                	for( int j = 0; j <= NX; j++ )
                	{
                        	glVertex3f( X0 + DX*(float)j, YGRID, Z0 + DZ*(float)(i+0) );
                        	glVertex3f( X0 + DX*(float)j, YGRID, Z0 + DZ*(float)(i+1) );
                	}
                	glEnd( );
        	}
	glEndList( );

	RightWallDL = glGenLists(1);
	glNewList(RightWallDL, GL_COMPILE);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_NORMALIZE);
		SetMaterial(0.80f, 0.60f, 0.60f, 80.f);  
		glNormal3f( 1.f, 0.f, 0.f);  
		const float xRight = X0; 
		for (int i = 0; i < NY; ++i) {
			glBegin(GL_QUAD_STRIP);
			for (int k = 0; k <= NZ; ++k) { 
				float z  = Z0 + DZ * (float)k;
				float y0 = Y0 + DY * (float)i;
				float y1 = y0 + DY;
				glVertex3f(xRight, y0, z);
				glVertex3f(xRight, y1, z);
			}
			glEnd();
		}
	glEndList();

	BackWallDL = glGenLists(1);
	glNewList(BackWallDL, GL_COMPILE);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_NORMALIZE);
		SetMaterial(0.60f, 0.60f, 0.80f, 10.f); 

		glNormal3f(0.f, 0.f,  1.f);     
		const float zBack = Z0; 
		
		for (int i = 0; i < NY; ++i) {
			glBegin(GL_QUAD_STRIP);
			for (int j = 0; j <= NX; ++j) {  
				float x  = X0 + DX * (float)j;
				float y0 = Y0 + DY * (float)i;
				float y1 = y0 + DY;
				glVertex3f(x, y0, zBack);
				glVertex3f(x, y1, zBack);
			}
			glEnd();
		}
	glEndList();




	// create the 3 objects hehehe
	Salmon = LoadObjMtlFiles ((char*)"/Users/michellevanessapinonieto/Downloads/SampleMac/salmon.obj");
	Vase = LoadObjMtlFiles ((char*)"/Users/michellevanessapinonieto/Downloads/SampleMac/vase.obj");
	Teapot = LoadObjMtlFiles ((char*)"/Users/michellevanessapinonieto/Downloads/SampleMac/teapot.obj");
	

	// create the axes:
	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int viewmenu = glutCreateMenu( DoViewMenu );
	glutAddMenuEntry("Outside", VIEW_OUTSIDE);
	glutAddMenuEntry("Inside",  VIEW_INSIDE);

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);
	glutAddSubMenu("View", viewmenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'p':
		case 'P':
			LightMode = 0;
			break;

		case 's':
		case 'S':
			LightMode = 1;
			break;

		// color menu
		case 'w': LightR = 1.f; LightG = 1.f; LightB = 1.f; break;
		case 'r': LightR = 1.f; LightG = 0.f; LightB = 0.f; break;
		case 'o': LightR = 1.f; LightG = 0.5f; LightB = 0.f; break;
		case 'y': LightR = 1.f; LightG = 1.f; LightB = 0.f; break;
		case 'g': LightR = 0.f; LightG = 1.f; LightB = 0.f; break;
		case 'c': LightR = 0.f; LightG = 1.f; LightB = 1.f; break;
		case 'm': LightR = 1.f; LightG = 0.f; LightB = 1.f; break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	NowView = VIEW_OUTSIDE;
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
