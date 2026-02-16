// Compilation-time configuration of the voxel engine.


//TODO: think of a better way to store configs
//An idea would be to store things like the rendering method
//as a compilation variable in Cmake
// and all the things that can be evaluated at runtime 
// on the voxelengineconfig struct.


//TODO: maybe move this in the cmake configuration
#define RENDERING_OPENGL
//#define RENDERING_N3DS
//#define RENDERING_NDS


/* Chunk related constants */
//TODO: maybe move those into the runtime config struct
#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 16
#define CHUNK_DEPTH 16
#define CHUNK_SIZE (CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH)

//TODO: maybe move those into the runtime config struct
//or directly into the camera class
#define CAMERA_MOV_SPEED 5.0f
#define CAMERA_DEFAULT_ZOOM 70.0f
#define DEFAULT_CAM_YAW -90.0f
#define DEFAULT_CAM_PITCH 0.0f

#define MOUSE_SENSITIVITY 0.1f
