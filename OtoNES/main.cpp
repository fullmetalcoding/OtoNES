//Using SDL, SDL OpenGL, standard IO, and, strings
#include <SDL.h>
#include <SDL_ttf.h>
#if defined(WIN32) | defined(_WIN64)
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#include <windows.h>
#endif

#include <string>
#include <iostream>

#include "NesMachine.h"
#include "RomLoader.h"


//@TODO: make these configurable.
//Write now we just have 2x scaling.
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

	SDL_QueryTexture(screenTexture, &format, &access, &w, &h);
	std::cout << "Format: " << format << std::endl
		<< "access: " << access << std::endl
		<< "w: " << w << std::endl
		<< "h: " << h << std::endl;

	nes::RomLoader loader;
	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("super_mario_bros.nes");
	//std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("cart.nes");
//	std::shared_ptr<nes::mappers::IMapper> rom = loader.loadRom("Atomic.nes");

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
	while (!quit)
	{
		if (machine)
		{
			machine->advanceCycles(1);
			fps++;
			
		}
		if (SDL_TICKS_PASSED(SDL_GetTicks(), nextFrame))
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
			std::cout << "FPS: " << fps << std::endl;
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

				default:
					break;
				}
			}
			

		
		}
	
	}

	return 0;
}