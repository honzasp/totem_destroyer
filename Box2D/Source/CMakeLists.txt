FILE(GLOB_RECURSE BOX2D_SRCS Common/*.cpp Collision/*.cpp Dynamics/*.cpp)

ADD_LIBRARY(box2d STATIC ${BOX2D_SRCS})

# v Box2D jsou obcas nepouzite parametry, tak at nerusi
SET_TARGET_PROPERTIES(box2d PROPERTIES
	COMPILE_FLAGS -Wno-unused
)

