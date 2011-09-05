#include "Game.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <stdio.h>


Game *g_App;

Game::Game():done(false),active(false),currentY(5), currentX(0), fallSpeedMS(600), moveSpeedMS(200), map()
{
	hDC = NULL;
	hRC = NULL;
	hWnd = NULL;
	hInstance = NULL;
	memset(keys, 0, sizeof(int) * 256);
	g_App = this;
	fallSpeedMS = 1500;
	moveSpeedMS = 200;
	leftDown = false;
	rightDown = false;
}


Game::~Game()
{
	KillGLWindow();
	//g_App = NULL;
}

int Game::InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	return TRUE;										// Initialization Went OK
}

int Game::DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix

	glPushMatrix();
	glTranslatef(-5.0f,0.0f,-30.0f);

	map.Render();
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2f(10.0f, 10.0f);
	glPrint("Lines - %d", map.GetLines());

	glPopMatrix();
	return TRUE;										// Everything Went OK
}


GLvoid Game::KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
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

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
BOOL Game::CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
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
	wc.lpfnWndProc		= (WNDPROC)(Game::WndProc);					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}
	
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

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
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
	
	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
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
	BuildFont();

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

GLvoid Game::ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
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


LRESULT CALLBACK Game::WndProc(	HWND	hWnd,			// Handle For This Window
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
				g_App->active=TRUE;						// Program Is Active
			}
			else
			{
				g_App->active=FALSE;						// Program Is No Longer Active
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
			g_App->keys[wParam] = TRUE;					// If So, Mark It As TRUE

			if (wParam == 88)
				g_App->map.RotatePiece(1);
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			if (wParam == VK_RIGHT)
				g_App->rightDown = false;

			if (wParam == VK_LEFT)
				g_App->leftDown = false;

			g_App->keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			g_App->ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void Game::Destroy()
{
	glDeleteLists(base, 96);				// Delete All 96 Characters ( NEW )
	done = true;

	//DestroyWindow(hWnd);
}

void Game::Update(DWORD deltaMS)
{
	// Only want to move the blocks after a short period, so that it doesn't shoot over.
	static int cumulativeMS = 0, moveMS=0;
	float decrease = (float)(deltaMS / 1000.0f);
	cumulativeMS = cumulativeMS + deltaMS;
	moveMS = moveMS + deltaMS;

	int y=0, x=0;

	if (cumulativeMS > fallSpeedMS)
		y = -1;

	if (keys[VK_RIGHT] && (moveMS > moveSpeedMS || !rightDown))
	{
		rightDown = true;
		x += -1;
	}

	if (keys[VK_LEFT]  && (moveMS >  moveSpeedMS || !leftDown))
	{
		leftDown = true;
		x += 1;
	}

	if (keys[VK_DOWN])
	{
		y = -1;
		keys[VK_DOWN] = false;
	}

	map.MovePiece(x, y);

	if (!map.HasPiece())
	{
		Type t = (Type)(rand() % 7);
		map.CreateNewPiece(t);
	}

	if (cumulativeMS >  fallSpeedMS)
		cumulativeMS = 0;

	if (moveMS > moveSpeedMS)
		moveMS = 0;
}

int Game::Run()
{
	srand( timeGetTime() );
	map.PopulateMap();
	MSG msg;
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
			if (active)								// Program Active?
			{
				if (keys[VK_ESCAPE])				// Was ESC Pressed?
				{
					done=TRUE;						// ESC Signalled A Quit
				}
				else								// Not Time To Quit, Update Screen
				{
					DWORD time = timeGetTime();
					DWORD lapse = time - currentTime;
					currentTime = time;
					Update(lapse);
					DrawGLScene();					// Draw The Scene
					SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
				}
			}

			if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1]=FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow("NeHe's OpenGL Framework",640,480,16,fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}
		}
	}

	return 0;
}

GLvoid Game::BuildFont(GLvoid)					// Build Our Bitmap Font
{
	HFONT	font;						// Windows Font ID
	HFONT	oldfont;					// Used For Good House Keeping

	base = glGenLists(96);					// Storage For 96 Characters ( NEW )
	font = CreateFont(	  -24,				// Height Of Font ( NEW )
							0,				// Width Of Font
							0,				// Angle Of Escapement
							0,				// Orientation Angle
						FW_BOLD,			// Font Weight
						FALSE,				// Italic
						FALSE,				// Underline
						FALSE,				// Strikeout
					ANSI_CHARSET,			// Character Set Identifier
					OUT_TT_PRECIS,			// Output Precision
				CLIP_DEFAULT_PRECIS,		// Clipping Precision
				ANTIALIASED_QUALITY,		// Output Quality
				FF_DONTCARE|DEFAULT_PITCH,	// Family And Pitch
					"Courier New");			// Font Name

		oldfont = (HFONT)SelectObject(hDC, font);		// Selects The Font We Want
	wglUseFontBitmaps(hDC, 32, 96, base);			// Builds 96 Characters Starting At Character 32
	SelectObject(hDC, oldfont);				// Selects The Font We Want
	DeleteObject(font);					// Delete The Font
}

GLvoid Game::glPrint(const char *fmt, ...)				// Custom GL "Print" Routine
{
	char		text[256];				// Holds Our String
	va_list		ap;						// Pointer To List Of Arguments

	if (fmt == NULL)					// If There's No Text
		return;							// Do Nothing
	va_start(ap, fmt);					// Parses The String For Variables
	    vsprintf(text, fmt, ap);		// And Converts Symbols To Actual Numbers
	va_end(ap);							// Results Are Stored In Text
	glPushAttrib(GL_LIST_BIT);				// Pushes The Display List Bits		( NEW )
	glListBase(base - 32);					// Sets The Base Character to 32	( NEW )	
	
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);	// Draws The Display List Text	( NEW )
	glPopAttrib();						// Pops The Display List Bits	( NEW )
}


//////////////////////////////////////////////////////////////
//////////// Map  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////



Map::Map()
{
	memset(map, 0, sizeof(int) * width * height);
	currentX = currentY = 0;
	currentPiece = NULL;
}

void Map::Render()
{
	if (currentPiece != NULL)
	{
		glPushMatrix();
		glTranslatef(-(currentX-3),currentY-2,0.0f);	
		currentPiece->Render();
		glPopMatrix();
	}
	float halfHeight = (height / 2.0f);
	float halfWidth = (width / 2.0f);
	glBegin(GL_LINES);
		for (int i = 0; i <= height; i++)
		{
			if (i == 0 || i == height)
				glColor3f( 1.0f, 1.0f, 1.0f);
			else
				glColor3f( 0.3f, 0.3f, 0.3f);
			glVertex3f(-halfWidth + 0.5f, -halfHeight + 0.5f + i, 0.0f);
			glVertex3f(halfWidth + 0.5f, -halfHeight + 0.5f + i, 0.0f);
		}
		for (int i = 0; i <= width; i++)
		{
			if (i == 0 || i == width)
				glColor3f( 1.0f, 1.0f, 1.0f);
			else
				glColor3f( 0.3f, 0.3f, 0.3f);
			glVertex3f(-halfWidth + 0.5f + i, -halfHeight + 0.5f, 0.0f);
			glVertex3f(-halfWidth + 0.5f + i, halfHeight + 0.5f, 0.0f);
		}
	glEnd();

	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			if (map[i][j] > 0)
			{
				glPushMatrix();
				glTranslatef((width/2.0f) - (float)i,(height/2.0f) - (float)j,0.0f);	
				glBegin(GL_TRIANGLE_STRIP);
					glColor3f( (i+1)/(float)width, (j+1)/(float)height, 0.0f);
					glVertex3f(-0.5f, -0.5f, 0.0f);
					glVertex3f(-0.5f, 0.5f, 0.0f);
					glVertex3f(0.5f, -0.5f, 0.0f);
					glVertex3f(0.5f, 0.5f, 0.0f);
				glEnd();
				glPopMatrix();
			}
		}
	}
}

void Map::CreateNewPiece(Type t)
{
	if (currentPiece == NULL)
	{
		currentY = height / 2 + 1;
		currentX = width / 2 - 2;
		currentPiece = new Piece(t);
	}
}

void Map::MovePiece(int x, int y)
{
	currentX += x;
	if (currentX > width-1)
		currentX = width -1;
	// HERE IS THE PROBLEM.
	// Not any more, sucker!
	if (currentX < -(width / 2.0f))
		currentX = -(width / 2.0f);

	int response = TestCollision(currentX, currentY);
	if (response > 0)
	{
		if (response == 2)
		{
			PlacePiece(currentX, currentY);
		}
		currentX -= x;
	}

	currentY += y;
	if (currentY <= -(height / 2.0f))
		PlacePiece(currentX, currentY-y);

	response = TestCollision(currentX, currentY);
	if (response > 0)
	{
		PlacePiece(currentX, currentY - y);
		currentY -= y;
	}

	if (response > 0 && currentY > (height / 2.0f))
	{
		// Game Over!
		MessageBox(NULL, "Game Over!", "Game Over!", MB_OK);
		g_App->done = true;
	}
}

void Map::PopulateMap()
{
	//map[0][9] = 1;
	//map[1][9] = 1;
	//map[2][9] = 1;
	//map[3][9] = 1;
	//map[4][9] = 1;
	//map[6][9] = 1;
	//map[7][9] = 1;
	//map[8][9] = 1;
	//map[9][9] = 1;

	//map[0][8] = 1;
	//map[1][8] = 1;
	//map[2][8] = 1;
	//map[3][8] = 1;
	//map[4][8] = 1;
	//map[6][8] = 1;
	//map[7][8] = 1;
	//map[8][8] = 1;
	//map[9][8] = 1;

	//map[0][6] = 1;
	//map[1][6] = 1;
	//map[2][6] = 1;
	//map[3][6] = 1;
	//map[4][6] = 1;
	//map[6][6] = 1;
	//map[7][6] = 1;
	//map[8][6] = 1;
	//map[9][6] = 1;
}

int Map::TestCollision(int x, int y)
{
	int value = 0;
	int cx = x;
	int cy = (height / 2.0f) - y;

	if (currentPiece == NULL)
		return value;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (currentPiece->area[i][j] && (j+cy >= height))
				return 2;
			else if (currentPiece->area[i][j] && (i+x >= width))
				return 1;
			else if (currentPiece->area[i][j] && (i+x < 0))
				return 1;
			else if (currentPiece->area[i][j] && (i+cx >= 0) && (i+cx < width) && (j+cy >= 0) && (j+cy < height))
			{
				if(map[i+cx][j+cy])
					return 1;
			}
		}
	}
	return value;
}

void Map::PlacePiece(int x, int y)
{
	int cx = x;
	int cy = (height / 2.0f) - y;

	if (currentPiece == NULL)
		return;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (currentPiece->area[i][j] && (i+cx >= 0) && (i+cx < width) && (j+cy >= 0) && (j+cy < height))
			{
				map[i+cx][j+cy] = 1;
			}
		}
	}

	CheckMapForRows();

	delete(currentPiece);
	currentPiece = NULL;
}

void Map::CheckMapForRows()
{
	bool row = true;
	int cy = (height - 1) / 2.0f - currentY;
	int max = ((cy+(height /2.0f) -1) > height ? height : cy+((height / 2.0f) -1));

	for (int i = cy; i < max ; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (i < height && !map[j][i])
			{
				row = false;
				break;
			}
		}

		if (row)
		{
			lines++;
			for (int h = i; h > 1; h--)
				for (int k = 0; k < width; k++)
					map[k][h] = map[k][h-1];
		}

		row = true;
	}
}

//////////////////////////////////////////////////////////////
//////////// Piece ///////////////////////////////////////////
//////////////////////////////////////////////////////////////

Piece::Piece()
{
	CreatePiece();
}

Piece::Piece(Type t)
{
	CreatePiece(t);
}

void Piece::CreatePiece(Type t)
{
	memset(area, 0, sizeof(int) * 4 * 4);

	switch (t)
	{
	case Square:
		area[2][1] = 1;
		area[2][2] = 1;
		area[1][1] = 1;
		area[1][2] = 1;
		break;

	case L:
		area[2][1] = 1;
		area[2][2] = 1;
		area[2][0] = 1;
		area[1][2] = 1;
		break;

	case Z:
		area[2][1] = 1;
		area[1][1] = 1;
		area[1][2] = 1;
		area[0][2] = 1;
		break;

	case S:
		area[1][1] = 1;
		area[2][1] = 1;
		area[2][2] = 1;
		area[3][2] = 1;
		break;

	case T:
		area[1][1] = 1;
		area[2][1] = 1;
		area[2][2] = 1;
		area[3][1] = 1;
		break;

	case Line:
		area[1][0] = 1;
		area[1][1] = 1;
		area[1][2] = 1;
		area[1][3] = 1;
		break;

	case BL:
		area[1][0] = 1;
		area[1][1] = 1;
		area[1][2] = 1;
		area[2][2] = 1;
		break;
	}
}



void Piece::Render()
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if (area[i][j] > 0)
			{
				glPushMatrix();
				glTranslatef(2.0f - (float)i,2.0f - (float)j,0.0f);	
				glBegin(GL_TRIANGLE_STRIP);
					glColor3f( 1.0f, 0.0f, 0.0f);
					glVertex3f(-0.5f, -0.5f, 0.0f);
					glVertex3f(-0.5f, 0.5f, 0.0f);
					glVertex3f(0.5f, -0.5f, 0.0f);
					glVertex3f(0.5f, 0.5f, 0.0f);
				glEnd();
				glPopMatrix();
			}
		}
	}
}

void Map::RotatePiece(int direction)
{
	if (currentPiece == NULL)
		return;

	int rotation[4][4];
	
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			if (direction > 0)
				rotation[j][3-i] = currentPiece->area[i][j];
			else
				rotation[3-j][i] = currentPiece->area[i][j];

	memcpy(currentPiece->area, rotation, sizeof(int) * 4 * 4);

	int result = TestCollision(currentX, currentY);
	int tempX = -1;
	for (int i = 0; i < 5 && result > 0; i++)
	{
		tempX = currentX + (2 - i);
		result = TestCollision(tempX, currentY);
	}

	// Can't rotate it
	if (result > 0)
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				if (direction <= 0)
					currentPiece->area[j][3-i] = rotation[i][j];
				else
					currentPiece->area[3-j][i] = rotation[i][j];
	}
	else if (tempX > 0)
	{
		currentX = tempX;
	}
}
