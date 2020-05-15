
#include <stdio.h>  // printf();
#include <stdlib.h> // exit()
#include <memory.h> // memset()

#include <X11/Xlib.h> // for native windowing
#include <X11/Xutil.h> // for visual and related api
#include <X11/XKBlib.h> // for keyboard api
#include <X11/keysym.h>

#include <GL/gl.h> // for OpenGL
#include <GL/glx.h> // for bridging api
#include <GL/glu.h> // for OpenGL utilities
#include "SOIL.h" // image loading lib

// global variable declarations
bool gbFullscreen = false;
int giWindowWidth = 800;
int giWindowHeight = 600;
Display *gpDisplay = NULL; // 'Display' is a struct (or connection) used by the X client to communicate with the X server (O.S)
XVisualInfo *gpXVisualInfo = NULL; // 'XVisualInfo' is a struct that stores information about the visual.
Colormap gColormap; // color palette
Window gWindow;
GLXContext gGLXcontext; // GLXContext is a data structure which acts like a state machinea which stores all the states associated with this instance of the OpenGL.
GLfloat anglePyramid = 0.0f; // angle of rotation for the pyramid
GLfloat angleCube = 0.0f; // angle of rotation for the cube
GLfloat angleSphere = 0.0f; // angle of rotation for the sphere
GLuint Texture_Kundali; // texture object for the Kundali texture
GLuint Texture_Stone; // texture object for the pyramid texture
GLuint Texture_Earth; // texture object for the earth texture
GLUquadric *quad;

// entry point function
int main(void){

	// function prototypes
	void createWindow(void);
	void toggleFullscreen(void);
	void initializeOpenGl(void);
	void display(void);
	void resize(int, int);
	void uninitialize(void);
	void spin(void);

	bool bDone = false; // this flag will be used to 'close' the window.
	int winWidth = giWindowWidth;
	int winHeight = giWindowHeight;

	// code
	// Step 1: Create the window
	createWindow();

	// Step 2: Initialize OpenGL
	initializeOpenGl();

	// Step 3: Handle events on this window
	XEvent event;
	KeySym keysym;

	while(bDone == false){

		while(XPending(gpDisplay)){ // XPending() returns the number of events that have been received from the X server but have not been removed from the event queue.
		// @gpDisplay - connection to the x server
		XNextEvent(gpDisplay, &event); // XNextEvent() copies the first event from the event queue into the specified 'event' objet and then removes it from the event queue.
		switch(event.type){

			case MapNotify: // This event is generated when the window changes the state from unmapped to mapped (similar to  WM_CREATE)
				break;

			case KeyPress: // This event is triggered when the keyboard button is pressed.
				keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0); // 3rd param - locale (0 is default), 4th param - 'Shift' key status
				switch(keysym){

					case XK_Escape: // XK - X Keycode
					bDone = true;
					// or
					// uninitialize();
					// exit(0);

					case XK_F:
					case XK_f:
						if(gbFullscreen == false){
							toggleFullscreen();
							gbFullscreen = true;
						} else{
							toggleFullscreen();
							gbFullscreen = false;
						}
						break;
					case XK_T:
					case XK_t:
						glEnable(GL_TEXTURE_2D);
						break;

					default:
						break;

					} // END: switch(keysym)
					break; // END: case KeyPress: 

				case ButtonPress: // Mouse button events
					switch(event.xbutton.button){
						case 1: // similar to WM_LBUTTONDOWN
						 break;
						case 2: // similar to WM_MBUTTONDOWN
							break;
						case 3: // similar to WM_RBUTTONDOWN
							break;
						case 4: // similar to MOUSE_WHEELUP
							break;
						case 5: // similar to MOUSE_WHEELDOWN
							break;
						} // END: switch(event.xbutton.button)	
						break; // END: ButtonPress

					case MotionNotify: // handles mouse motion events 
						break;

					case ConfigureNotify: // similar to WM_RESIZE
						winWidth = event.xconfigure.width;
						winHeight = event.xconfigure.height;
						resize(winWidth, winHeight);
						break;

					case DestroyNotify:
						break;

					case Expose:
						break;

					case 33: // handles a click on 'Close' box and also a click on sys menu 'Close'
						uninitialize();
						exit(0);

					} // END: switch(event.type)
				
				}// END: while (XPending(gpDisplay))
				spin();
				display(); // Rendering is done here
			
			} // END: while(bDone == false)

			uninitialize();
			return 0;
		}
		

void createWindow(void){ // Note: All the programs which create their own window are X clients of the X server (O.S)

	// function prototypes
	void uninitialize (void);

	// variable declarations
	XSetWindowAttributes winAttribs;

	int defaultScreen;
	int styleMask;

	static int frameBufferList[] = {
		GLX_RGBA, // consider only 'TrueColor' and 'DirectColor' visuals, otherwise PseudoColor and StaticColor visuals are considered
		GLX_RED_SIZE, 8, // the returned visual should support a buffer (1-bit atleast) of red color
		GLX_GREEN_SIZE, 8, // the returned visual should support a buffer (1-bit atleast) of green color
		GLX_BLUE_SIZE,8, // the returned visual should support a buffer (1-bit atleast) of blue color.
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_DOUBLEBUFFER, True,
		None // the frameBufferList array must be terminated by 0 and hence 'None' is used.
	};

	// code 
	// Step 1: Get the connection of the local display.
	gpDisplay = XOpenDisplay(NULL); // NULL - gives default local connection.
	if(gpDisplay == NULL){
		printf("ERROR: Unable to open X display.\n");
		uninitialize();
		exit(1); // abortive exit and hence a positive number used.
	}

	// Step 2: Get the default monitor/screen (from 'Display' struct) to which the graphic card is connected.
	defaultScreen = XDefaultScreen(gpDisplay);

	// Step 3: Get the XVisualInfo object that meets the minimum requirements
	// Note: A single display can support multiple screens. Each screen can have several different visual types supported at different depths.
	gpXVisualInfo = glXChooseVisual(gpDisplay, defaultScreen, frameBufferList); // glXChooseVisual() returns a pointer to a XVisualInfo struct that best meets the specified requirements.

	// Step 4: Set the window attributes
	winAttribs.border_pixel = 0; // 0 - default
	winAttribs.background_pixmap = 0; // background pixmap - images like cursor, icon , etc (0 - default)

	// Get the colormap ID
	// Note: The XCreateColormap() creates a colormap of the specified visual type for the screen on which the specified window resides and returns the colormap ID associated with it.
	winAttribs.colormap = XCreateColormap(gpDisplay,
		RootWindow(gpDisplay, gpXVisualInfo->screen),
		gpXVisualInfo->visual,
		AllocNone); // AllocNone - Don't allocate fixed memory

	gColormap = winAttribs.colormap;
	winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen)	; // set the background color
	
	// Specify the events which should be sent to this window
	winAttribs.event_mask = ExposureMask | // similar to WM_PAINT
		VisibilityChangeMask | // similar to WM_CREATE
		ButtonPressMask | 
		KeyPressMask |
		PointerMotionMask |
		StructureNotifyMask; // handles window resize events, similar to WM_SIZE (addressed in case 'ConfigureNotify')	

	// Specify window style
	styleMask = CWBorderPixel | CWBackPixel |CWEventMask | CWColormap; // CW - Create Window or Change Window

	// Step 5: Create the window
	gWindow = XCreateWindow(gpDisplay, // connection to the X server
		RootWindow(gpDisplay, gpXVisualInfo->screen), // parent window
		0, // x coordinate
		0, // y coordinate
		giWindowHeight,
		giWindowWidth,
		0, // border-width
		gpXVisualInfo->depth, // color depth of the window
		InputOutput, // type of the window
		gpXVisualInfo->visual, // type of the visual
		styleMask, // style
		&winAttribs); // attributes of the window

	if(!gWindow){
		printf("ERROR: Failed to create the window.\n");
		uninitialize();
		exit(1);
	}

	// Step 6: Name in the caption bar
	XStoreName(gpDisplay, gWindow, "Demo - Textures");

	// Step 7: Process the window 'close' event
	// Step 7.1: Create an atom to handle the window close event
	// Note: 'Atom' is a unique string and it stays in the memory until the app ends
	Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True); // XInternAtom() returns the atom id associated with "WM_DELETE_WINDOW" message/event (WM stands for Window Message), True - create an atom even if it exists.

	// Step 7.2: Ask the window manager to add windowManagerDelete atom to the set of protocols.
	XSetWMProtocols (gpDisplay, gWindow, &windowManagerDelete, 1); // 3rd param is an array of atoms, 4th param - number of ptotocols to set

	// Step 8: Map this window to the screen
	XMapWindow(gpDisplay, gWindow);
}

bool LoadTexture(GLuint *Texture, const char *path){

	int imgWidth;
	int imgHeight;
	unsigned char *imageData = NULL;

	// imageTexture
	imageData = SOIL_load_image(path, &imgWidth, &imgHeight, 0, SOIL_LOAD_RGBA);

	// set the pixel storage mode
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // specifies the alignment requirements for the start of each pixel row in memory.

	/* glGenTextures - generate texture names
		@n - The number of texture names to be generated.
		@textures - A pointer to the first element of an array in which the generated texture names are stored
	*/
	glGenTextures(1, Texture);

	/* glBindTexture - enables the creation of a named texture that is bound to a texture target.
	   @target - The target to which the texture is bound. Must have the value GL_TEXTURE_1D or GL_TEXTURE_2D.
	   @texture - The name of a texture (cannot be currently in use).	
	*/
	glBindTexture(GL_TEXTURE_2D, *Texture);

	/*
		glTexParameteri - Sets texture parameters
		@target - The target texture, which must be either GL_TEXTURE_1D or GL_TEXTURE_2D.
		@pname - The symbolic name of a single values texture paramter.
		(GL_TEXTURE_MAG_FILTER - The texture magnification function is used when the pixel being textured maps to an area less than or equal to one texture element. It sets the texture magnification function to either GL_NEAREST or GL_LINEAR).
		@param - The value of pname
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// GL_TEXTURE_MIN_FILTER - The texture minifying function is used whenever the pixel being textured maps to an area greater than one texture element.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	// gluBuild2DMipmaps - Builds 2D mipmaps
	/*
		@target - Specifies the target texture. Must be GLU_TEXTURE_2D
		@internalFormat - Requests the internal storage format of the texture image.
		@width & @height - Specifies the width and height in pixels, respectively of the texture image.
		@format - Specifies the data type for data.
		@data - Specifies a pointer to the image data in memory. 		 
	*/
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, imgWidth, imgHeight, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

	SOIL_free_image_data(imageData);

	return (true);

}

void initializeOpenGl(void){

	void resize(int, int);
	//LoadTexture(GLuint *, const char *);
	// code
	// Step 1: Create an OpenGL context
	/*
		glXCreateContext() creates a GLX rendering context and returns it's handle.
		@Display - specifies the connection to the X server.
		@XVisualInfo - specifies the visual that defines the framebuffer resources available to the rendering context
		@GLXContext - specifies the context with which to share display list (NULL indicates that the context is not shared)
		@Bool - specifies the direct rendering (TRUE - hardware rendering using the graphics card, false - software rendering by the O.S)
	*/
	gGLXcontext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);

	// Step 2: Make the 'gGLXContext' as the current context (i.e. attach it to the window).
	glXMakeCurrent(gpDisplay, gWindow, gGLXcontext);

	// specify the clear value for the 'Depth' buffer
	glClearDepth(1.0f);

	// Enable the OpenGL capabilities
	glEnable(GL_DEPTH_TEST); // GL_DEPTH_TEST - perform comparisons and update the 'Depth' buffers

	// Specify the value used for depth-buffer comparisons.
	glDepthFunc(GL_LEQUAL); // GL_LEQUAL - passes if the incoming z value is less than or equal to the stored z value.

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black

	LoadTexture(&Texture_Stone, "pyramid.bmp");
	LoadTexture(&Texture_Kundali, "kundali.bmp");
	LoadTexture(&Texture_Earth, "earth.bmp");


	resize(giWindowWidth, giWindowHeight);
}

void resize(int width, int height){

	// code
	if(height==0){
		height = 1;
	}
	if(width==0){
		width = 1;
	}

	// set the viewport according to the newer size of the window
	glViewport(0,0,(GLsizei) width, (GLsizei) height);

	// specify which matrix is the current matrix
	glMatrixMode(GL_PROJECTION);

	// convert the above matrix to the identity matrix.
	glLoadIdentity();

	// set up a perspective projection matrix
	/*
		gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
		@fovy - specifies the field of view angle, in degrees in y direction
		@aspect - specifies the aspect ratio of x(width) to y (height)
		@zNear - specifies the distance from the viewer to the near clipping plane(always +ve)
		@zFar - specifies the distance from the viewer to the far clipping plane (always +ve)

	*/
	gluPerspective(60.0f, (GLfloat) width / (GLfloat) height, 0.1f, 100.0f);

}

void spin(void){

	void display (void);
	// code
	anglePyramid = anglePyramid + 3.0f;
	if(anglePyramid >= 360.0f)
		anglePyramid = anglePyramid-360.0f;

	angleCube = angleCube + 3.0f;
	if(angleCube >= 360.0f)
		angleCube = angleCube - 360.0f;

	angleSphere = angleSphere + 0.5f;
	if(angleSphere >= 360.0f)
		angleSphere = angleSphere - 360.0f;

	//display();
}

void display(void){

	// code
	// Step 1: Clear all the pixels of the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// pyramid
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-2.5f, 1.0f, -6.0f);
	glRotatef(anglePyramid,0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, Texture_Stone);
	glBegin(GL_TRIANGLES);
	// Note:- Each face of a pyramid except the bottom/base is a triangle
	// glTextCoord2f() - sets the texture to the given coordinates
	glTexCoord2f(0.5f, 1.0f); // Repeat the texture after 1.0f
	glVertex3f(0.0f, 1.0f, 0.0f); // apex of the triangle

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0f,-1.0f,1.0f); // left-bottom tip of the triangle

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(1.0f,-1.0f,1.0f); // right-bottom tip of the triangle

	//******RIGHT FACE****
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f,1.0f,0.0f); // apex of the triangle

	glTexCoord2f(1.0f,0.0f); 
	glVertex3f(1.0f, -1.0f, 1.0f); // left-bottom tip of the triangle
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(1.0f,-1.0f,-1.0f); // right-bottom tip of the triangle
	//*****BACK FACE*****
	glTexCoord2f(0.5f,1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f); // apex of triangle

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f,-1.0f,-1.0f); //left-bottom tip of triangle

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0f,-1.0f,-1.0f); // right-bottom tip of triangle

	//****LEFT FACE******
	glTexCoord2f(0.5f,1.0f);
	glVertex3f(0.0f,1.0f,0.0f); // apex of the triangle

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0f,-1.0f,-1.0f); // left-bottom tip of triangle

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(-1.0f,-1.0f,1.0f); // right-bottom tip of triangle

	glEnd();

	// cube
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(2.0f,1.0f,-6.0f);
	glScalef(0.75f, 0.75f, 0.75f);
	glRotatef(angleCube, 1.0f, 1.0f, 1.0f); 
	glBindTexture(GL_TEXTURE_2D, Texture_Kundali);
	glBegin(GL_QUADS);
	// Note:- Each face of the cube is a square
		
	// ***** TOP FACE *****
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f,1.0f,-1.0f); // right-top of top face

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f); // left-top of top face

	glTexCoord2f(1.0f,0.0f); 
	glVertex3f(-1.0f,1.0f,1.0f); // left-bottom of top face

	glTexCoord2f(1.0f,1.0f);
	glVertex3f(1.0f,1.0f,1.0f); // right-bottom of top face
	
	// ***** BOTTOM FACE *****
	glTexCoord2f(1.0f,1.0f);
	glVertex3f(1.0f,-1.0f,1.0f); // right-top of bottom face

	glTexCoord2f(0.0f,1.0f); 
	glVertex3f(-1.0f,-1.0f,1.0f); // left-top of bottom face

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0f,-1.0f,-1.0f); // left-bottom of bottom face

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f,-1.0f,-1.0f); // right-bottom of bottom face

	// ***** LEFT FACE *****
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f,1.0f,1.0f); // right-top of front face

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f); // left-top of front face

	glTexCoord2f(1.0f,1.0f);
	glVertex3f(-1.0f,-1.0f,1.0f); // left-bottom of front face

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f,-1.0f,1.0f); // right-bottom of front face

	// *****BACK FACE *****
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f); // right-top of back face

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f,-1.0f,-1.0f); // left-top of back face

	glTexCoord2f(0.0f,1.0f);
	glVertex3f(-1.0f,1.0f,-1.0f); // left-bottom of back face

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f); // right-bottom of back face

	// ****LEFT FACE *****
	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0f,1.0f,1.0f); // right-top of left face

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f,-1.0f); // left-top of left face

	glTexCoord2f(1.0f,1.0f);
	glVertex3f(-1.0f,-1.0f,-1.0f); // left-bottom of left face

	glTexCoord2f(0.0f,1.0f);
	glVertex3f(-1.0f,-1.0f,1.0f); // right-bottom of left face

	// ****RIGHT FACE *****
	glTexCoord2f(1.0f,0.0f);
	glVertex3f(1.0f,1.0f,-1.0f); // right-top of right face

	glTexCoord2f(1.0f,1.0f);
	glVertex3f(1.0f, 1.0f,1.0f); // left-top of right face

	glTexCoord2f(0.0f,1.0f);
	glVertex3f(1.0f,-1.0f,1.0f); // left-bottom of right face

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f); // right-bottom of right face

	glEnd();

	// sphere
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,-2.0f, -6.0f);
	glScalef(0.5f,0.5f,0.5f);
	glRotatef(angleSphere, 0.0f, 2.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, Texture_Earth);
	quad = gluNewQuadric();
	// gluQuadricTexture - specifies if texture coordinates should be generated for quadrics rendered with quad
	gluQuadricTexture(quad, GL_TRUE);
	/*
		gluSphere(GLUquadric* quad, GLdouble radius, GLint slices, GLint stacks) - draw a sphere
		@quad - Specifies the quadrics object (created with gluNewQuadric)
		@radius - Specifies the radius of the sphere
		@slices - Specifies the number of subdivisions around the z axis (similar to lines of longitude)
		@stacks - Specifies the number of subdivisions along the z axis (similar to lines of latitude)
	*/
	gluSphere(quad, 2, 200, 200);

	// Step 3: Process the buffered OpenGL routines
	glXSwapBuffers(gpDisplay, gWindow);

}


void toggleFullscreen(void){

	// variable declaration
	Atom wm_state; // Atom to store the current state of the window
	Atom fullscreen; // Atom for the fullscreen
	XEvent xev = {0};

	// code
	// step 1: Get the current state of the window and save it in an atom
	wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False); // XInternAtom() will return the atom associated with the "_NET_WM_STATE" message where _NET stands for Network Compliant and WM stands for Window Message
	// @False - don't create a separate atom if it already exists

	// step 2: Create a custom event (also called as the 'client' event)
	// step 2.1: Allocate 0 memory to all the members of the 'event' obj (before it's use)
	memset(&xev,0,sizeof(xev));

	// step 2.2: set the values to this custom event
	xev.type = ClientMessage;
	xev.xclient.window = gWindow;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32; // 32-bit

	// Note:- 'data' is a union in XEvent and 'l' is the array inside it.
	xev.xclient.data.l[0] = gbFullscreen ? 0 : 1;

	// step 3: create an atom for fullscreen
	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xev.xclient.data.l[1] = fullscreen;
	XSendEvent( gpDisplay,
				RootWindow(gpDisplay, gpXVisualInfo->screen), // propogate the message to this window
				False, // do not propogate the message to the child window
				StructureNotifyMask, // This event_mask should be generated (similar to WM_SIZE)
				&xev); // custom_event
}

void uninitialize(void){
	// works like a destructor i.e. destroy in the reverse order of creation

	GLXContext currentGLXContext;
	currentGLXContext = glXGetCurrentContext();

	if((currentGLXContext != NULL) && (currentGLXContext == gGLXcontext)){
		glXMakeCurrent(gpDisplay, 0,0);
	}

	if(gGLXcontext){
		glXDestroyContext(gpDisplay, gGLXcontext);
	}

	if(gWindow){
		XDestroyWindow(gpDisplay, gWindow);
	}	

	if(gColormap){
		XFreeColormap(gpDisplay, gColormap);
	}

	if(gpXVisualInfo){
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;
	}

}