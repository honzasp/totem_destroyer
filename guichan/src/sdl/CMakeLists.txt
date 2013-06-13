INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})

ADD_LIBRARY(guichan_sdl STATIC 
	sdl.cpp			
	sdlgraphics.cpp		
	sdlinput.cpp 
	sdlimage.cpp
	sdlimageloader.cpp
	sdltruetypefont.cpp
)
