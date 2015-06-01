#pragma once
#include  "Camera.h"
#include  "SDL\SDL.h"
#include  <GL\glew.h>
#include  "TensorSplat.h"
#include  "Display.h"

struct EventState
{
	bool left_button_down;
	bool right_button_down;
	glm::vec2 last_click_position;
};

typedef struct EventState EventState;

/******************************************************************************
 *																			  *
 *                              EventManager Class                            *
 *																			  *
 ******************************************************************************
*  aspectRatio                                                                *
*          Ratio describing the width/height for the viewport.                *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Class which acts as an event manager for the application. All input and    *
*  output will be handled by an event manager object.                         *
*                                                                             *
*******************************************************************************/
class EventManager
{
/* Public Members. */
public:

	/* Constructor. */
	EventManager();

	/* Handle an SDL Event. */
	void           handleSDLEvent(SDL_Event* event);
	/* Handle a key press event. */
	void           handleKeyPress(SDL_Scancode key);
	// Handle a button press event.
	void           handleButtonClick(SDL_MouseButtonEvent button);
	// Handle a button release event.
	void           handleButtonRelease(SDL_MouseButtonEvent button);
	// Handle a mouse motion event.
	void           handleMouseMotion(SDL_Event* event);

	/* Setters. */
	void           setCamera(Camera* c)          {  camera = c;           }
	void           setDisplay(Display* d)        {  display = d;          }
 	void           setSlice(GLint* s)            {  slice = s;            }
	void           setSpeed(GLfloat* s)          {  speed = s;            }
	void           setThreshold(GLfloat* t)      {  threshold = t;        }
	void           setPlay(bool* p)              {  play = p;             }
	void           setMode(GLuint* m)            {  mode = m;             }
	void           setField(TensorField* t)      {  field = t;            }
	void           setSliceList(SliceList* s)    {  slice_list = s;       }
	/* Destructor. */
	~EventManager()                              {  /* Empty */           }

/* Private Members. */
private:
	/* Camera for the application. */
	Camera*         camera;
	Display*        display;
	GLfloat*        speed;
	GLfloat*        threshold;
	GLint*          slice;
	bool*           play;
	GLuint*         mode;
	TensorField*    field;
	SliceList*      slice_list;
	SDL_Event       last_event;
	EventState      state;
};

