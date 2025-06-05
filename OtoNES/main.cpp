//Using SDL, SDL OpenGL, standard IO, and, strings
#include <SDL.h>
#include "SDL_syswm.h"
#include <SDL_ttf.h>
#if defined(WIN32) | defined(_WIN64)
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <shobjidl.h> 
#include <atlstr.h>
#include <shlwapi.h>
#endif

#include <string>
#include <iostream>
#include <sstream>

#include "NesMachine.h"
#include "RomLoader.h"


std::string loadcart_dialog2(SDL_Window* window)
{
	std::cout << "loadcart_dialog2()" << std::endl;
	std::string filePath; 
	
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(window, &info) == -1) cout << "Failed on GetWMInfo!\n";
	std::cout << "Coinit" << std::endl;
	HRESULT hr = CoInitialize(NULL);// , COINIT_APARTMENTTHREADED |
		//COINIT_DISABLE_OLE1DDE);
	std::stringstream streamlinedfile;
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;

		// Create the FileOpenDialog object.
		std::cout << "Cocreate..." << std::endl;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			std::cout << "Show" << std::endl;
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					LPWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					filePath = CW2A(pszFilePath);
					std::cout << "GETDISPLAY NAME: " << filePath << std::endl;

					// Display the file name to the user.
					if (SUCCEEDED(hr))
					{
						streamlinedfile << *pszFilePath;
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}


	return filePath;// streamlinedfile.str();
}
std::string loadcart_dialog(SDL_Window* window) {
	std::cout << "loadcart_dialog()" << std::endl;
	OPENFILENAME* ofp = new OPENFILENAME();
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(window, &info) == -1) cout << "Failed on GetWMInfo!\n";
	char filename[500] = { 0 };


	/*=== Initialization mess to use open file dialogbox ===*/
	ofp->lStructSize = sizeof(OPENFILENAME);
	ofp->hwndOwner = info.info.win.window;
	ofp->lpstrFilter = "*.nes";
	ofp->lpstrFile = filename;
	ofp->nMaxFile = sizeof(filename);

	ofp->lpstrInitialDir = NULL;
	ofp->lpstrTitle = "Open a rom";




	if (GetOpenFileNameA(ofp)) {
		delete ofp;
		return std::string(filename);
	}
	else {
		cout << "getopenfilename failed:\n";

		switch (CommDlgExtendedError()) {
		case CDERR_DIALOGFAILURE:
			cout << "Dialog failure.\n";
			break;
		case CDERR_FINDRESFAILURE:
			cout << "Find resource failure. \n";
			break;
		case CDERR_NOHINSTANCE:
			cout << "No Hinstance failure.\n";
			break;
		case CDERR_INITIALIZATION:
			cout << "Init failure.\n";
			break;
		case CDERR_NOHOOK:
			cout << "No hook!\n";
			break;
		case CDERR_NOTEMPLATE:
			cout << "No template\n";
			break;
		case CDERR_LOADRESFAILURE:
			cout << "load resource failure.\n";
			break;
		case CDERR_STRUCTSIZE:
			cout << "Struct size failure";
			break;
		default:
			cout << "Other error!\n";
			break;
		}
		delete ofp;
		return "";
	}

}


//@TODO: make these configurable.
//right now we just have 2x scaling.
const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 512;
int main(int argc, char** argv)
{
	//The window we'll be rendering to
	SDL_Window* window = NULL;

	//The surface contained by the window
	SDL_Surface* screenSurface = NULL;
	SDL_Texture* screenTexture = NULL;
	SDL_Renderer* renderer = NULL;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
	//	std::string errorString = "SDL_Init_Error: " + boost::lexical_cast<std::string>(SDL_GetError()) + "\n";
	//	OutputDebugStringA(errorString.c_str());
		
		return 1;
	}
	//Create window
	window = SDL_CreateWindow("OtoNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	if (TTF_Init() < 0)
	{
		std::cerr << "Failed to initialize SDL_TTF!" << std::endl;
		return 1;
	}

	TTF_Font* mainFont = TTF_OpenFont("emulogic.ttf", 5);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
	uint32_t format;
	int access;
	int w;
	int h;

	//SDL_QueryTexture(screenTexture, &format, &access, &w, &h);
//	std::cout << "Format: " << format << std::endl
	//	<< "access: " << access << std::endl
	//	<< "w: " << w << std::endl
	//	<< "h: " << h << std::endl;


	//@TODO: Need some kind of platform agnostic loader window here... Too bad SDL doesn't
	//have something like that. :(
	std::cout << "Calling cart load dialog..." << std::endl;
	std::string loadme = loadcart_dialog2(window);
	std::cout << "LOAD ME: " << loadme << std::endl;
	nes::RomLoader loader;
	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom(loadme);
 //   std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("SMARIO.nes");

//	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("Duck_tales.nes");
//	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("Black_bass.nes");
//	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("Megaman.nes");
//	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("1942.nes");
//	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("cart.nes");
//	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("Atomic.nes");
//	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("junkrom.nes");

	std::shared_ptr<nes::NesMachine> machine;
	if (!rom)
	{
		std::cout << "FAILED TO LOAD ROM." << std::endl;
	}
	else
	{
		machine.reset(new nes::NesMachine(rom));
		std::cout << "NES created." << std::endl;

	}
	if (machine)
	{
		machine->reset();
	}
	uint64_t fps = 0;
	bool quit = false;
	Uint32 nextTicks = SDL_GetTicks() + 1000;
	Uint32 nextFrame = SDL_GetTicks() + 17;
	Uint32 nextCPUSlice = SDL_GetTicks() + 8; 
	bool frameEnable = true;
	uint64_t last = SDL_GetPerformanceCounter();
	const double cpuFreq = 1789773.0; // NES NTSC CPU frequency
	while (!quit)
	{
		if (machine)
		{
			uint64_t now = SDL_GetPerformanceCounter();
			double deltaSec =
				((double)(now - last) )/ static_cast<double>(SDL_GetPerformanceFrequency());
			last = now;

			uint64_t cyclesToRun = deltaSec * cpuFreq;
			//Run 2 ms or around 30 scanlines... 
			if(cyclesToRun > 0)
			{
				
				machine->advanceCycles(cyclesToRun);
			}	
			
		}
		if (SDL_TICKS_PASSED(SDL_GetTicks(), nextFrame) && frameEnable)
		{
			
			std::shared_ptr<nes::PPU> ppu = machine->getPPU();
			std::shared_ptr<uint8_t[]> screen = ppu->getScreen();
			void* screenPix;
			int screenPitch;
			
			SDL_LockTexture(screenTexture, NULL, &screenPix, &screenPitch);
			uint8_t* pointerHATE = (uint8_t*)screenPix;
			//memcpy(screenPix, screen.get(), 256 * 3 * 256);
			//Asked for RGB888, it says it gave me RGB888, it behaves as if it's BGRA8888...
			//FML...
			for (size_t i = 0; i < 256 * 256; i++)
			{
				pointerHATE[i * 4] = screen[i * 3 + 2];
				pointerHATE[i * 4 + 1] = screen[i * 3 + 1];
				pointerHATE[i * 4 + 2] = screen[i * 3];
				pointerHATE[i * 4 + 3] = 255;
			}
			SDL_UnlockTexture(screenTexture);
			nextFrame = SDL_GetTicks() + 17;
		
			//Update the surface
			SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
			
			SDL_RenderPresent(renderer);
		}
		if (SDL_TICKS_PASSED(SDL_GetTicks(), nextTicks))
		{
			std::cout << "FPS: " << std::dec << fps << std::endl;
			nextTicks = SDL_GetTicks() + 1000;
			fps = 0;
		}

		
		SDL_Event e;
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{

			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym)
				{
				case SDLK_SPACE:
					if (machine)
					{
						//std::cout << "." << std::endl;
						//machine->advanceCycles(1);
					}
					break;
				case 'A':
				case 'a':
					if (machine)
					{
						machine->keyDown(NES_KEY_A);
					}
					break;
				case 'S':
				case 's':
					if (machine)
					{
						machine->keyDown(NES_KEY_B);
					}
					break;
				case 'Z':
				case 'z':
					if (machine)
					{
						machine->keyDown(NES_KEY_ST);
					}
					break;
				case 'x':
				case 'X':
					if (machine)
					{
						machine->keyDown(NES_KEY_SL);
					}
					break;

				case 'N':
				case 'n':
					if (machine)
					{
						machine->__debugTriggerNmi();
						machine->__debugFakeSprite0Hit();
					}
					break;
				case 'M':
				case 'm':
					if (machine)
					{
						machine->__debugFakeSprite0Hit();
					}
					break;
				case 'k':
				case 'K':
					if (machine)
					{
						machine->getPPU()->debugDumpNameTable("c:\\codestuffs\\ntdump");
					}
					break;
				case 'p':
				case 'P':
					if (machine)
					{
						machine->getPPU()->debugDumpPatternTable("c:\\codestuffs\\ptdump.raw");
					}
					break;
				case 'l':
				case 'L':
					if (machine)
					{
						machine->getPPU()->debugDumpRawScreen("c:\\codestuffs\\screendump.raw");
					}
					break;
				case 'f':
				case 'F':
					if (!frameEnable) frameEnable = true;
					else frameEnable = false; 
					
					std::cout << "FRAME ENABLE: " << frameEnable << std::endl;
					break;
				case 'r':
				case 'R':
					if (machine) machine->reset();
					break;
				case SDLK_UP:
					if (machine)
					{
						machine->keyDown(NES_KEYUP);
					}
					break;
				case SDLK_DOWN:
					if (machine)
					{
						machine->keyDown(NES_KEYDOWN);
					}
					break;
				case SDLK_RIGHT:
					if (machine)
					{
						machine->keyDown(NES_KEYRIGHT);
					}
					break;
				case SDLK_LEFT:
					if (machine)
					{
						machine->keyDown(NES_KEYLEFT);
					}
					break;

				default:
					break;
				}
			}
			else if (e.type == SDL_KEYUP)
			{
				switch (e.key.keysym.sym)
				{
				case 'A':
				case 'a':
					if (machine)
					{
						machine->keyUp(NES_KEY_A);
					}
					break;
				case 'S':
				case 's':
					if (machine)
					{
						machine->keyUp(NES_KEY_B);
					}
					break;
				case 'Z':
				case 'z':
					if (machine)
					{
						machine->keyUp(NES_KEY_ST);
					}
					break;
				case 'x':
				case 'X':
					if (machine)
					{
						machine->keyUp(NES_KEY_SL);
					}
					break;
				case SDLK_UP:
					if (machine)
					{
						machine->keyUp(NES_KEYUP);
					}
					break;
				case SDLK_DOWN:
					if (machine)
					{
						machine->keyUp(NES_KEYDOWN);
					}
					break;
				case SDLK_RIGHT:
					if (machine)
					{
						machine->keyUp(NES_KEYRIGHT);
					}
					break;
				case SDLK_LEFT:
					if (machine)
					{
						machine->keyUp(NES_KEYLEFT);
					}
					break;

				default:
					break;
				}
			}

		
		}
	
	}

	return 0;
}