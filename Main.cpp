#include <windows.h>
#include "Game.h"

Game g_Game;

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
					HINSTANCE	hPrevInstance,		// Previous Instance
					LPSTR		lpCmdLine,			// Command Line Parameters
					int			nCmdShow)			// Window Show State
{

	bool fullscreen = false;
	
	// Create Our OpenGL Window
	if (!g_App->CreateGLWindow("Tetris game!",640,480,16,fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	g_App->Run();


	return (0);
}
