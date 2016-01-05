/*
 *		This Code Was Published By Jeff Molofee 2000
 *		Code Was Created By David Nikdel For NeHe Productions
 *		If You've Found This Code Useful, Please Let Me Know.
 *		Visit My Site At nehe.gamedev.net
 *      Modified for LCCWin32 compiler by Robert Wishlaw 2002/11/24
 */

/**         Comments manageable by Doxygen
*
*  Modified smoothly by Thierry DECHAIZE
*
*  Paradigm : obtain one source (only one !) compatible for multiple free C Compilers
*    and provide for all users an development environment on Windows (64 bits compatible),
*    the great Code::Blocks manager (version 13.12), and don't use glaux.lib or glaux.dll.
*
*	 a) Mingw 32 and 64 bits : downloadable http://sourceforge.net/projects/mingw/
*    b) Mingw 32 DEV-CPP  : included in old Dev-Cpp http://sourceforge.net/projects/devcpp/
*    c) Mingw 32 CB : included in package Code::Blocks (version 13.12) http://sourceforge.net/projects/codeblocks/files/Binaries/13.12/Windows/
*    d) Borland C (version 5.5) : downloadable  http://edn.embarcadero.com/article/20633
*    e) Digital Mars Compiler C : version 8.57 downloadable http://www.digitalmars.com
*    f) OpenWatcom : version 1.9 downloadable http://openwatcom.mirror.fr/    Version 2.0 64bits seems very buggy !!!
*	 g) Lcc and Lcc64 : downloadable http://www.cs.virginia.edu/~lcc-win32/
*	 h) Cygwin : downloadable http://www.cygwin.com/install.html (setup.exe)
*	 i) Visual Studio 2005 + 64bits: express edition for free (my version 2005 is full not express) http://www.microsoft.com/france/visual-studio/essayez/express.aspx
*    j) TDM GCC 64 bits : downloadable on http://sourceforge.net/projects/TDM-GCC
*
*  Important remark : All install in C partition, "no space" and "no special characters" in the name of directories
*            (it's reason to not install in C:\Program Files (x86) ... by default proposed on windows 64 bits)
*
*  Add resource file end resource header for restitute version + icon OpenGL.ico for fun
*  because versionning is important, same for freeware :-) !
*
*  Date : 2016/01/03
*
* \file            Lesson28.c
* \author          David Nikdel & Jeff Molofee (NeHe) originely, Modified for LCCWin32 compiler by Robert Wishlaw 2002/11/24
* \author          After adapted by Thierry Dechaize to verify paradigm : one source for multiple C Compilers
* \version         1.0.1.2
* \date            3 janvier 2016
* \brief           Ouvre une fenêtre Windows et dessine une forme traversée par les lignes de Bezier que l'on peut faire tourner avec les flêches de déplacement clavier.
* \details         Ce programme ne gére que trois événements : le clic sur le bouton "Fermé" de la fenêtre, la sortie par la touche ESC et les flêches de déplacement clavier.
*
*
*/

#if defined __CYGWIN__ || defined __LCC__
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>		// Header File For Windows
#if defined(__LCC__)
#ifndef WINGDIAPI
#   define WINGDIAPI __stdcall
#endif
#endif

#include <math.h>			// Header File For Math Library Routines
#include <stdio.h>			// Header File For Standard I/O Routines
#include <stdlib.h>			// Header File For Standard Library
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glext.h>		// Header File For The GLuext Library
/* #include <gl\glaux.h>		                    // Header File For The Glaux Library  */

#if defined __GNUC__ || defined __LCC__ || defined _MSC_VER || defined __TURBOC__
#include <stdbool.h>
#endif

#ifdef __WATCOMC__
#include <stdbool.h>
typedef BOOLEAN         bool;
#endif

#ifdef __DMC__
typedef BOOL         bool;
#endif

typedef struct point_3d {			// Structure for a 3-dimensional point (NEW)
	double x, y, z;
} POINT_3D;

typedef struct bpatch {				// Structure for a 3rd degree bezier patch (NEW)
	POINT_3D	anchors[4][4];			// 4x4 grid of anchor points
	GLuint		dlBPatch;				// Display List for Bezier Patch
	GLuint		texture;				// Texture for the patch
} BEZIER_PATCH;

HDC				hDC=NULL;			// Private GDI Device Context
HGLRC			hRC=NULL;			// Permanent Rendering Context
HWND			hWnd=NULL;			// Holds Our Window Handle
HINSTANCE		hInstance;			// Holds The Instance Of The Application

DEVMODE			DMsaved;			// Saves the previous screen settings (NEW)

BOOL			keys[256];			// Array Used For The Keyboard Routine
BOOL			active=TRUE;		// Window Active Flag Set To TRUE By Default
BOOL			fullscreen=TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default

GLfloat			rotz = 0.0f;		// Rotation about the Z axis
BEZIER_PATCH	mybezier;			// The bezier patch we're going to use (NEW)
BOOL			showCPoints=TRUE;	// Toggles displaying the control point grid (NEW)
int				divs = 7;			// Number of intrapolations (conrols poly resolution) (NEW)
GLuint		    texid;				// Texture for the patch

static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		0,  										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
    };

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

/************************************************************************************/

// Adds 2 points. Don't just use '+' ;)
POINT_3D pointAdd(POINT_3D p, POINT_3D q) {
	p.x += q.x;		p.y += q.y;		p.z += q.z;
	return p;
}

// Multiplies a point and a constant. Don't just use '*'
POINT_3D pointTimes(double c, POINT_3D p) {
	p.x *= c;	p.y *= c;	p.z *= c;
	return p;
}

// Function for quick point creation
POINT_3D makePoint(double a, double b, double c) {
	POINT_3D p;
	p.x = a;	p.y = b;	p.z = c;
	return p;
}


// Calculates 3rd degree polynomial based on array of 4 points
// and a single variable (u) which is generally between 0 and 1
POINT_3D Bernstein(float u, POINT_3D *p) {
	POINT_3D	a, b, c, d, r;

	a = pointTimes(pow(u,3), p[0]);
	b = pointTimes(3*pow(u,2)*(1-u), p[1]);
	c = pointTimes(3*u*pow((1-u),2), p[2]);
	d = pointTimes(pow((1-u),3), p[3]);

	r = pointAdd(pointAdd(a, b), pointAdd(c, d));

	return r;
}

bool NeHeLoadBitmap(LPTSTR szFileName, GLuint texid)					// Creates Texture From A Bitmap File
{
	HBITMAP hBMP;														// Handle Of The Bitmap
	BITMAP	BMP;														// Bitmap Structure

	hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL), szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );

	if (!hBMP)															// Does The Bitmap Exist?
		return FALSE;													// If Not Return False

	GetObject(hBMP, sizeof(BMP), &BMP);									// Get The Object
																		// hBMP:        Handle To Graphics Object
																		// sizeof(BMP): Size Of Buffer For Object Information
																		// &BMP:        Buffer For Object Information
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);								// Pixel Storage Mode (Word Alignment / 4 Bytes)

	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, texid);								// Bind To The Texture ID
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Min Filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Mag Filter
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	DeleteObject(hBMP);													// Delete The Object

	return TRUE;														// Loading Was Successful
}

// Generates a display list based on the data in the patch
// and the number of divisions
GLuint genBezier(BEZIER_PATCH patch, int divs) {
	int			u = 0, v;
	float		py, px, pyold;
	GLuint		drawlist = glGenLists(1);		// make the display list
	POINT_3D	temp[4];
	POINT_3D	*last = (POINT_3D*)malloc(sizeof(POINT_3D)*(divs+1));
												// array of points to mark the first line of polys

	if (patch.dlBPatch != 0)					// get rid of any old display lists
		glDeleteLists(patch.dlBPatch, 1);

	temp[0] = patch.anchors[0][3];				// the first derived curve (along x axis)
	temp[1] = patch.anchors[1][3];
	temp[2] = patch.anchors[2][3];
	temp[3] = patch.anchors[3][3];

	for (v=0;v<=divs;v++) {						// create the first line of points
		px = ((float)v)/((float)divs);			// percent along y axis
	// use the 4 points from the derives curve to calculate the points along that curve
		last[v] = Bernstein(px, temp);
	}

	glNewList(drawlist, GL_COMPILE);				// Start a new display list
	glBindTexture(GL_TEXTURE_2D, patch.texture);	// Bind the texture

	for (u=1;u<=divs;u++) {
		py	  = ((float)u)/((float)divs);			// Percent along Y axis
		pyold = ((float)u-1.0f)/((float)divs);		// Percent along old Y axis

		temp[0] = Bernstein(py, patch.anchors[0]);	// Calculate new bezier points
		temp[1] = Bernstein(py, patch.anchors[1]);
		temp[2] = Bernstein(py, patch.anchors[2]);
		temp[3] = Bernstein(py, patch.anchors[3]);

		glBegin(GL_TRIANGLE_STRIP);					// Begin a new triangle strip

		for (v=0;v<=divs;v++) {
			px = ((float)v)/((float)divs);			// Percent along the X axis

			glTexCoord2f(pyold, px);				// Apply the old texture coords
			glVertex3d(last[v].x, last[v].y, last[v].z);	// Old Point

			last[v] = Bernstein(px, temp);			// Generate new point
			glTexCoord2f(py, px);					// Apply the new texture coords
			glVertex3d(last[v].x, last[v].y, last[v].z);	// New Point
		}

		glEnd();									// END the triangle srip
	}

	glEndList();								// END the list

	free(last);									// Free the old vertices array
	return drawlist;							// Return the display list
}

/************************************************************************************/

void initBezier(void) {
	mybezier.anchors[0][0] = makePoint(-0.75,	-0.75,	-0.5);	// set the bezier vertices
	mybezier.anchors[0][1] = makePoint(-0.25,	-0.75,	0.0);
	mybezier.anchors[0][2] = makePoint(0.25,	-0.75,	0.0);
	mybezier.anchors[0][3] = makePoint(0.75,	-0.75,	-0.5);
	mybezier.anchors[1][0] = makePoint(-0.75,	-0.25,	-0.75);
	mybezier.anchors[1][1] = makePoint(-0.25,	-0.25,	0.5);
	mybezier.anchors[1][2] = makePoint(0.25,	-0.25,	0.5);
	mybezier.anchors[1][3] = makePoint(0.75,	-0.25,	-0.75);
	mybezier.anchors[2][0] = makePoint(-0.75,	0.25,	0.0);
	mybezier.anchors[2][1] = makePoint(-0.25,	0.25,	-0.5);
	mybezier.anchors[2][2] = makePoint(0.25,	0.25,	-0.5);
	mybezier.anchors[2][3] = makePoint(0.75,	0.25,	0.0);
	mybezier.anchors[3][0] = makePoint(-0.75,	0.75,	-0.5);
	mybezier.anchors[3][1] = makePoint(-0.25,	0.75,	-1.0);
	mybezier.anchors[3][2] = makePoint(0.25,	0.75,	-1.0);
	mybezier.anchors[3][3] = makePoint(0.75,	0.75,	-0.5);
	mybezier.dlBPatch = 0;
    glGenTextures(1, &(mybezier.texture));											// Create The Texture
}

/************************************************************************************/
// (no changes)

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

/************************************************************************************/

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.05f, 0.05f, 0.05f, 0.5f);			// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	initBezier();											// Initialize the Bezier's control grid
	NeHeLoadBitmap("../../data/NeHe.bmp",mybezier.texture);	// Load the texture
	mybezier.dlBPatch = genBezier(mybezier, divs);			// Generate the patch

	return TRUE;										// Initialization Went OK
}

/************************************************************************************/

int DrawGLScene(GLvoid)	{								// Here's Where We Do All The Drawing
	int i, j;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix
	glTranslatef(0.0f,0.0f,-4.0f);						// Move Left 1.5 Units And Into The Screen 6.0
	glRotatef(-75.0f,1.0f,0.0f,0.0f);
	glRotatef(rotz,0.0f,0.0f,1.0f);						// Rotate The Triangle On The Z axis ( NEW )

	glCallList(mybezier.dlBPatch);						// Call the Bezier's display list
														// this need only be updated when the patch changes

	if (showCPoints) {									// If drawing the grid is toggled on
		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f,0.0f,0.0f);
		for(i=0;i<4;i++) {								// draw the horizontal lines
			glBegin(GL_LINE_STRIP);
			for(j=0;j<4;j++)
				glVertex3d(mybezier.anchors[i][j].x, mybezier.anchors[i][j].y, mybezier.anchors[i][j].z);
			glEnd();
		}
		for(i=0;i<4;i++) {								// draw the vertical lines
			glBegin(GL_LINE_STRIP);
			for(j=0;j<4;j++)
				glVertex3d(mybezier.anchors[j][i].x, mybezier.anchors[j][i].y, mybezier.anchors[j][i].z);
			glEnd();
		}
		glColor3f(1.0f,1.0f,1.0f);
		glEnable(GL_TEXTURE_2D);
	}

	return TRUE;										// Keep Going
}

/************************************************************************************/

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		if (!ChangeDisplaySettings(NULL,CDS_TEST)) { // if the shortcut doesn't work
			ChangeDisplaySettings(NULL,CDS_RESET);		// Do it anyway (to get the values out of the registry)
			ChangeDisplaySettings(&DMsaved,CDS_RESET);	// change it to the saved settings
		} else {
			ChangeDisplaySettings(NULL,CDS_RESET);
		}

		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL",hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									// Set hInstance To NULL
	}
}

/************************************************************************************/

/**	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(char* title, int width, int height, int bits, BOOL fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DMsaved); // save the current display state (NEW)

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								title,								// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								hInstance,							// Instance
								NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

    pfd.cColorBits = bits;

    if (!(hDC=GetDC(hWnd)))                         // Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

/************************************************************************************/
// (no changes)

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active=TRUE;						// Program Is Active
			}
			else
			{
				active=FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:							// Intercept System Commands
		{
			switch (wParam)							// Check System Calls
			{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
				return 0;							// Prevent From Happening
			}
			break;									// Exit
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

/************************************************************************************/

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done=FALSE;								// Bool Variable To Exit Loop

	// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;							// Windowed Mode
	}

	// Create Our OpenGL Window
	if (!CreateGLWindow("David Nikdel & NeHe's Bezier Tutorial",640,480,16,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done=TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
			}


			if (keys[VK_LEFT])	rotz -= 0.8f;		// rotate left
			if (keys[VK_RIGHT])	rotz += 0.8f;		// rotate right
			if (keys[VK_UP]) {						// resolution up
				divs++;
				mybezier.dlBPatch = genBezier(mybezier, divs);	// Update the patch
				keys[VK_UP] = FALSE;
			}
			if (keys[VK_DOWN] && divs > 1) {
				divs--;
				mybezier.dlBPatch = genBezier(mybezier, divs);	// Update the patch
				keys[VK_DOWN] = FALSE;
			}
			if (keys[VK_SPACE]) {					// SPACE toggles showCPoints
				showCPoints = !showCPoints;
				keys[VK_SPACE] = FALSE;
			}


			if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1]=FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("David Nikdel & NeHe's Bezier Tutorial",640,480,16,fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
