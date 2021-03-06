/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

/*
 * Produced by:
 *
 * 				Derivative Inc
 *				401 Richmond Street West, Unit 386
 *				Toronto, Ontario
 *				Canada   M5V 3A8
 *				416-591-3555
 *
 * NAME:				TOP_CPlusPlusBase.h 
 *
 */

/*******
	Do not edit this file directly!
	Make a subclass of TOP_CPlusPlusBase instead, and add your own data/function

	Derivative Developers:: Make sure the virtual function order
	stays the same, otherwise changes won't be backwards compatible
********/


#ifndef __TOP_CPlusPlusBase__
#define __TOP_CPlusPlusBase__

#define NOMINMAX
#include <windows.h>
#include <gl/gl.h>
#include <cstdio>

#include "CPlusPlus_Common.h"

#define DLLEXPORT __declspec (dllexport)

#define TOP_CPLUSPLUS_API_VERSION	4

class TOP_CPlusPlusBase;


// These are the definitions for the C-functions that are used to
// load the library and create instances of the object you define
typedef int (__cdecl *GETTOPAPIVERSION)(void);
typedef TOP_CPlusPlusBase* (__cdecl *CREATETOPINSTANCE)(const OP_NodeInfo*);
typedef void (__cdecl *DESTROYTOPINSTANCE)(TOP_CPlusPlusBase*);

// These classes are used to pass data to/from the functions you will define

enum ExecuteType 
{ 
	// Rendering is done using OpenGL into a FBO/RenderBuffers
	// that is provided for you
	OPENGL_FBO = 0,

	// CPU memory is filled with data directly. No OpenGL calls can be
	// made when using this mode. Doing so will likely result in
	// rendering issues within TD.
	CPU_MEM_WRITEONLY = 1, 
	CPU_MEM_READWRITE = 2 
};

enum CPUMemPixelType 
{ 
	// 8-bit per color, BGRA pixels
	BGRA8Fixed = 0, 
	// 8-bit per color, RGBA pixels
	RGBA8Fixed = 1, 
	// 32-bit float per color, RGBA pixels
	RGBA32Float = 2 
};



// Touch will select the best pixel format based on the options you give
// Not all possible combinations of channels/bit depth are possible,
// so you get the best choice supported by your card

class TOP_OutputFormat
{
public:
	int				width;
	int				height;


	// The aspect ratio of the TOP's output

	float			aspectX;
	float			aspectY;


	// The anti-alias level.
	// 1 means no anti-alaising
	// 2 means '2x', etc., up to 32 right now
	// Only used when executeMode == OPENGL_FBO

	int				antiAlias;


	// Set true if you want this channel, false otherwise
	// The channel may still be present if the combination you select
	// isn't supported by the card (blue only for example)

	bool			redChannel;
	bool			greenChannel;
	bool			blueChannel;
	bool			alphaChannel;


	// The number of bits per channel. 
	// Touch will select the closest supported number of bits based on
	// your cards capabilities

	unsigned		bitsPerChannel;

	// Set to true if you want a floating point format.
	// Some bit precisions don't support floating point (8-bit for example)
	// while others require it (32-bit)

	bool			floatPrecision;


	// If you want to use multiple render targets, you can set this
	// greater than one
	// Only used when executeMode == OPENGL_FBO

	int				numColorBuffers;


	// The number of bits in the depth buffer.
	// 0 for no depth buffer
	// Only used when executeMode == OPENGL_FBO

	int				depthBits;


	// The number of bits in the stencil buffer
	// 0 for no stencil buffer, if this is > 0 then
	// it will also cause a depth buffer to be created
	// even if you have depthBits == 0
	// Only used when executeMode == OPENGL_FBO

	int				stencilBits;

private:
	int				reserved[20];
};


// This class will tell you the actual output format
// that was chosen.
class TOP_OutputFormatSpecs
{
public:
	int				width;
	int				height;
	float			aspectX;
	float			aspectY;

	int				antiAlias;

    int				redBits;
    int				blueBits;
    int				greenBits;
    int				alphaBits;
    bool			floatPrecision;

    /*** BEGIN: OPENGL_FBO executeMode specific ***/
	int				numColorBuffers;

	int				depthBits;
	int				stencilBits;


	// This is the index of the FBO that TouchDesigner has bound for you
	// You can use this to re-bind the FBO incase you are using some of
	// your own for intermediate results.

	GLuint			FBOIndex;
    /*** END: OPENGL_FBO executeMode specific ***/




    /*** BEGIN: CPU_MEM_* executeMode specific ***/

    // if the 'executeMode' is set to CPU_MEM_*
    // then cpuPixelData will point to three blocks of memory of size 
    // width * height * bytesPerPixel
    // and one may be uploaded as a texture after the execute call.
	// All of these pointers will stay valid until the next execute() call
	// unless you set newCPUPixelDataLocation to 0, 1 or 2. In that case
	// the location you specified will become invalid as soon as execute()
	// returns. The pointer for the locations you don't specify stays 
	// valid though.
	// This means you can hold onto these pointers by default and use them
	// after execute() returns, such as filling them in another thread.
    void*           cpuPixelData[3];

    // setting this to 0 will upload memory from cpuPixelData[0],
    // setting this to 1 will upload memory from cpuPixelData[1]
    // setting this to 2 will upload memory from cpuPixelData[2]
    // uploading from a memory location will invalidate it and a new memory location will be provided next execute call.
    // setting this to -1 will not upload any memory and retain previously uploaded texture
    // setting this to any other value will result in an error being displayed in the CPlusPlus TOP.
    // defaults to -1
    mutable int     newCPUPixelDataLocation;

    /*** END: CPU_MEM_* executeMode specific ***/



	/*** BEGIN: New OPENGL_FBO execudeMode specific data ***/
	
	// The first color can either be a GL_TEXTURE_2D or a GL_RENDERBUFFER
	// depending on the settings. This will be set to either
	// GL_TEXTURE_2D or GL_RENDERBUFFER accordingly
	GLenum			colorBuffer0Type;

	// The indices for the renderBuffers for the color buffers that are attached to the FBO, except for possibly index 0 (see colorBuffer0Type)
	// these are all GL_RENDERBUFFER GL objects, or 0 if not present
	GLuint			colorBufferRB[32];
	
	// The renderBuffer for the depth buffer that is attached to the FBO
	// This is always a GL_RENDERBUFFER GL object
	GLuint 			depthBufferRB;

    /*** END: OPENGL_FBO executeMode specific ***/

private:
	int				reserved[10];
};

class TOP_GeneralInfo
{
public:
	// Set this to true if you want the TOP to cook every frame, even
	// if none of it's inputs/parameters are changing

	bool			cookEveryFrame;


	// Touch will clear the color/depth buffers before calling
	// execute(), as an optimization you can disable this, if you know
	// you'll be overwriting all the data or calling clear yourself

	bool			clearBuffers;


	// Set this to true if you want Touch to create mipmaps for all the
	// TOPs that are passed into execute() function

	bool			mipmapAllTOPs;

	// Set this to true if you want the CHOP to cook every frame, if asked
	// (someone uses it's output)
	// This is different from 'cookEveryFrame', which causes the node to cook
	// every frame no matter what

	bool			cookEveryFrameIfAsked;

	// When setting the output texture size using the node's common page
	// if using 'Input' or 'Half' options for example, it uses the first input
	// by default. You can use a different input by assigning a value 
	// to inputSizeIndex.

	int				inputSizeIndex;

    // executeType determines how you will update the texture
    // "OPENGL_FBO" - you will draw directly to the FBO using OpenGL calls.

	// *NOTE* - Do not use OpenGL calls when using a CPU_* executeMode.

    // "CPU_MEM_WRITEONLY" - cpuPixelData* will be provided that you fill in with pixel data. This will automatically be uploaded to the GPU as a texture for you. Reading from the memory will result in very poor performance.
    // "CPU_MEM_READWRITE" - same as CPU_MEM_WRITEONLY but reading from the memory won't result in a large performance pentalty. The initial contents of the memory is undefined still.
    // cpuPixelData[0] and cpupixelData[1] are width by height array of pixels. 
    // to access pixel (x,y) you would need to offset the memory location by bytesperpixel * ( y * width + x).
    // all pixels should be set, pixels that was not set will have an undefined value.
	ExecuteType			executeMode;

    // determines the datatype of each pixel in CPU memory. This will determin
	// the size of the CPU memory buffers that are given to you
	// in TOP_OutputFormatSpecs
    // "BGRA8Fixed" - each pixel will hold 4 fixed-point values of size 8 bits (use 'unsigned char' in the code). They will be ordered BGRA. This is the preferred ordering for better performance.
    // "RGBA8Fixed" - each pixel will hold 4 fixed-point values of size 8 bits (use 'unsigned char' in the code). They will be ordered RGBA
    // "RGBA32Float" - each pixel will hold 4 floating-point values of size 32 bits (use 'float' in the code). They will be ordered RGBA 
	CPUMemPixelType		memPixelType;

private:
	int				reserved[18];
};


/***** FUNCTION CALL ORDER DURING INITIALIZATION ******/
/*
	When the TOP loads the dll the functions will be called in this order

	setupParameters(OP_ParameterManager* m);

*/


/***** FUNCTION CALL ORDER DURING A COOK ******/
/*
	When the TOP cooks the functions will be called in this order

	getGeneralInfo()
	getOutputFormat()

	execute()
	getNumInfoCHOPChans()
	for the number of chans returned getNumInfoCHOPChans()
	{
		getInfoCHOPChan()
	}
	getInfoDATSize()
	for the number of rows/cols returned by getInfoDATSize()
	{
		getInfoDATEntries()
	}
	getWarningString()
	getErrorString()
	getInfoPopupString()

*/


/*** DO NOT EDIT THIS CLASS, MAKE A SUBCLASS OF IT INSTEAD ***/
class TOP_CPlusPlusBase
{
protected:
	TOP_CPlusPlusBase()
	{
	}


public:

	virtual ~TOP_CPlusPlusBase()
	{
	}

	// BEGIN PUBLIC INTERFACE

	// Some general settings can be assigned here by setting memebers of
	// the TOP_GeneralInfo class that is passed in
	virtual void		getGeneralInfo(TOP_GeneralInfo*)
						{
						}


	// This function is called so the class can tell the TOP what
	// kind of buffer it wants to output into.
	// Touch will try to find the best match based on the specifications
	// given.
	// Return true if you specify the output here
	// Return false if you want the output to be set by the TOP's parameters
	// The TOP_OutputFormat class is pre-filled with what the TOP would
	// output if you return false, so you can just tweak a few settings
	// and return true if you want

	virtual bool		getOutputFormat(TOP_OutputFormat*)
						{
							return false;
						}

	// In this function you do whatever you want to fill the framebuffer
	// 
	// See the OP_Inputs class definition for more details on it's
	// contents

	virtual void		execute(const TOP_OutputFormatSpecs*,
								OP_Inputs* ,
								void* reserved) = 0;


	// Override these methods if you want to output values to the Info CHOP/DAT
	// returning 0 means you dont plan to output any Info CHOP channels

	virtual int			getNumInfoCHOPChans()
						{
							return 0;
						}

	// Specify the name and value for CHOP 'index',
	// by assigning something to 'name' and 'value' members of the
	// OP_InfoCHOPChan class pointer that is passed (it points
	// to a valid instance of the class already.
	// the 'name' pointer will initially point to NULL
	// you must allocate memory or assign a constant string
	// to it.

	virtual void		getInfoCHOPChan(int index,
										OP_InfoCHOPChan* chan)
						{
						}


	// Return false if you arn't returning data for an Info DAT
	// Return true if you are.
	// Fill in members of the OP_InfoDATSize class to specify the size

	virtual bool		getInfoDATSize(OP_InfoDATSize* infoSize)
						{
							return false;
						}

	// You are asked to assign values to the Info DAT 1 row or column at a time
	// The 'byColumn' variable in 'getInfoDATSize' is how you specify
	// if it is by column or by row.
	// 'index' is the row/column index
	// 'nEntries' is the number of entries in the row/column

	virtual void		getInfoDATEntries(int index,
											int nEntries,
											OP_InfoDATEntries* entries)
						{
						}

	// You can use this function to put the node into a warning state
	// with the returned string as the message.
	// Return NULL if you don't want it to be in a warning state.
	virtual const char* getWarningString()
						{
							return NULL;
						}

	// You can use this function to put the node into a error state
	// with the returned string as the message.
	// Return NULL if you don't want it to be in a error state.
	virtual const char* getErrorString()
						{
							return NULL;
						}

	// Use this function to return some text that will show up in the
	// info popup (when you middle click on a node)
	// Return NULL if you don't want to return anything.
	virtual const char* getInfoPopupString()
						{
							return NULL;
						}



	// Override these methods if you want to define specfic parameters
	virtual void		setupParameters(OP_ParameterManager* manager)
						{
						}


	// This is called whenever a pulse parameter is pressed
	virtual void		pulsePressed(const char* name)
						{
						}


	// END PUBLIC INTERFACE
				

private:

	// Reserved for future features
	virtual int		reservedFunc6() { return 0; }
	virtual int		reservedFunc7() { return 0; }
	virtual int		reservedFunc8() { return 0; }
	virtual int		reservedFunc9() { return 0; }
	virtual int		reservedFunc10() { return 0; }
	virtual int		reservedFunc11() { return 0; }
	virtual int		reservedFunc12() { return 0; }
	virtual int		reservedFunc13() { return 0; }
	virtual int		reservedFunc14() { return 0; }
	virtual int		reservedFunc15() { return 0; }
	virtual int		reservedFunc16() { return 0; }
	virtual int		reservedFunc17() { return 0; }
	virtual int		reservedFunc18() { return 0; }
	virtual int		reservedFunc19() { return 0; }
	virtual int		reservedFunc20() { return 0; }

	int				reserved[400];

};

#endif
