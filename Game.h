#ifndef __Game
#define __Game

#include <windows.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <mmsystem.h>


enum Type { Square, L, Z, S, T, Line, BL };

class Piece
{
public:
	int area[4][4];

public:
	Piece();
	Piece(Type type);
	void Render();
	void CreatePiece(Type t=Square);
};


class Map
{
private:
	const static int width = 10;
	const static int height = 20;
	int map[width][height];
	int currentY, currentX;
	Piece *currentPiece;
	int lines;

	void CheckMapForRows();

public:
	Map();
	bool Collide(Piece p);
	void Drop(Piece p);
	void Render();
	void MovePiece(int x, int y);
	void CreateNewPiece(Type t);
	int TestCollision(int x, int y);
	void PlacePiece(int x, int y);
	void RotatePiece(int direction);
	void PopulateMap();
	bool HasPiece() {return (currentPiece != NULL);} 
	int GetLines() {return lines;}
};


class Game
{
public:

HDC			hDC;
HGLRC		hRC;
HWND		hWnd;
HINSTANCE	hInstance;

GLuint	base;

DWORD currentTime;

bool	keys[256];
bool	active;
bool	fullscreen;
bool	done;
bool	leftDown;
bool	rightDown;

Map map;

float currentY;
float currentX;

int fallSpeedMS;
int moveSpeedMS;

int InitGL(GLvoid);
int DrawGLScene(GLvoid);
GLvoid KillGLWindow(GLvoid);
GLvoid ReSizeGLScene(GLsizei width, GLsizei height);
GLvoid BuildFont(GLvoid);
GLvoid Game::glPrint(const char *fmt, ...);

void Update(DWORD deltaMS);

public:
	Game();
	~Game();

	void Destroy();
	static LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag);
	int Run();
};

extern Game *g_App;

#endif