/*******************************************************************************
 *                                                                             *
 *                              Included Header Files                          *
 *                                                                             *
 ******************************************************************************/
#include <gl\glew.h>
#include <SDL\SDL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include "Display.h"
#include "Shader.h"
#include "TensorSplat.h"
#include "Camera.h"
#include "EventManager.h"

/*******************************************************************************
 *                                                                             *
 *                        Defined Constants and Macros                         *
 *                                                                             *
 ******************************************************************************/
#define  FULLSCREEN_ENABLED   true
#define  DEFAULT_HEIGHT		  800
#define  DEFAULT_WIDTH        1000
#define  FORWARD              0
#define  BACKWARD             1
#define  MILLIS_PER_SECOND    1000
#define  FRAMES_PER_SECOND    40
#define  MAX_ROT              2 * M_PI
#define  PROJECT_TITLE        "Tensor Splatting Technique"
#define  SPLAT_FILE           "res/textures/gaussian_mask.png"
#define  TENSOR_FIELD_FILE    "res/data/mri_data.Lfloat"
#define  PRINT(a)             std::cout << a << std::endl;

/*******************************************************************************
 *                                                                             *
 *                                     main                                    *
 *                                                                             *
 *******************************************************************************
 * PARAMETERS                                                                  *
 *  argc                                                                       *
 *        The number of command line strings.                                  *
 *  argv                                                                       *
 *        The array of command line stirngs.                                   *
 *                                                                             *
 *******************************************************************************
 * RETURNS                                                                     *
 *  0 on success, any non-zero value on failure.                               *
 *                                                                             *
 *******************************************************************************
 * DESCRIPTION                                                                 *
 *  Beginning point of the program.                                            *
 *                                                                             *
 *******************************************************************************/
int main(int argc, char* argv[])
{
	// Initialize SDL with all subsystems.
	SDL_Init(SDL_INIT_EVERYTHING);

	// Initialize local parameters.
	GLfloat speed = 5;
	GLint slice = 0;
	bool play = true;
	GLfloat threshold = DEFAULT_THRESHOLD;
	GLuint direction = FORWARD;
	GLuint mode = ALL_PLANAR;
	SliceList slice_list;

	// Create the display, shader, camera, and event manager.
	Display      display(PROJECT_TITLE, DEFAULT_WIDTH, DEFAULT_HEIGHT);
	Camera*      camera = display.getCamera();
	EventManager eventManager;

	// Apply the shaders and maximize the display.
	//display.maximize();

	// Construct the tensor field.
	TensorSplat::init_texture(SPLAT_FILE);
	TensorField* field = TensorField::read_eig_file("res/data/nifti_dt.nii", 
		TENSOR_FIELD_FILE);

	// Set the controls of the event manager.
	eventManager.setDisplay(&display);
	eventManager.setCamera(camera);
	eventManager.setSpeed(&speed);
	eventManager.setSlice(&slice);
	eventManager.setThreshold(&threshold);
	eventManager.setPlay(&play);
	eventManager.setMode(&mode);
	eventManager.setField(field);
	eventManager.setSliceList(&slice_list);

	// Get the slices from the field.
	field->get_slices(slice_list, mode, threshold);


	// Instantiate the event reference.
	SDL_Event event;
	SDL_PollEvent(&event);

	// Begin the milliseconds counter.
	GLuint startMillis = 0, tempMillis = 0, currentMillis = 0,
		millisPerFrame = (GLuint)((1.0 / FRAMES_PER_SECOND) * MILLIS_PER_SECOND);
	startMillis = currentMillis = SDL_GetTicks();

	// Main loop.
	while (event.type != SDL_QUIT)
	{
		// Handle the new event.
		eventManager.handleSDLEvent(&event);

		// Get the new number of milliseconds.
		currentMillis = SDL_GetTicks();
		
		// If a new frame is to be drawn, update the display.
		if ((currentMillis - startMillis) >= millisPerFrame)
		{

			if (mode != ALL && mode != ALL_PLANAR && mode != ALL_LINEAR)
			{
				if (play && mode != ALL)
					slice = (direction == FORWARD) ? slice + 1 : slice - 1;

				if (slice < 0)
				{
					slice = 0;
					direction = FORWARD;
				}
				else if (slice >= (GLint)slice_list.size())
				{
					slice = slice_list.size() - 1;
					direction = BACKWARD;
				}
			}
			else
			{
				slice = 0;
			}
			display.repaint(slice_list[slice]);

			startMillis = currentMillis;

		}
		
		// Get the next event.
		SDL_PollEvent(&event);
	}

	field->cleanUp();
	TensorSplat::delete_texture();

	// Quit using SDL.
	SDL_Quit();

	// Exit Success.
	return 0;
}
