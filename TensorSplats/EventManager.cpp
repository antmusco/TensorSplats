#include "EventManager.h"
#include "glm\glm.hpp"
#include "GL\glew.h"
#include <iostream>

/******************************************************************************
*                                                                             *
*                     EventManager::EventManager (Constructor)                *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Constructor for the EventManager class. This constructor initializes the   *
*  camera used by this event manager.                                         *
*                                                                             *
*******************************************************************************/
EventManager::EventManager() :
camera(0), display(0), speed(0), slice(0), play(0), mode(0),
field(0), slice_list(0) { /* Empty. */ }

/******************************************************************************
*                                                                             *
*                         EventManager::handleSDLEvent()                      *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param event                                                               *
*           The SDL event to be handled.                                      *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Primary event handling function for the application. All SDL events are    *
*  sent here, which then routes the events to specific subroutines to handle  *
*  the specific action required.                                              *
*                                                                             *
*******************************************************************************/
void EventManager::handleSDLEvent(SDL_Event* event) 
{
	if (event->type == SDL_MOUSEMOTION)
	{
		handleMouseMotion(event);
	}
	else if (event->type == SDL_KEYDOWN)
	{
		handleKeyPress(event->key.keysym.scancode);
	}
	else if (event->type == SDL_MOUSEBUTTONDOWN)
	{
		handleButtonClick(event->button);
	}
	else if (event->type == SDL_MOUSEBUTTONUP)
	{
		handleButtonRelease(event->button);
	}
	last_event = *event;
}

void EventManager::handleMouseMotion(SDL_Event* event)
{
	if (state.left_button_down)
	{
		if (camera != NULL)
			camera->updatePosition({ event->motion.x, event->motion.y });
	}
	else if (state.right_button_down)
	{
			camera->updateLookAt({ event->motion.x, event->motion.y });
	}
}

/******************************************************************************
*                                                                             *
*                      EventManager::handleButtonClick                        *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param button                                                              *
*           The SDL_MouseButtonEvent indicating the button pressed.           *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Handles a button event, typically indicating camera motion.                *
*                                                                             *
*******************************************************************************/
void EventManager::handleButtonClick(SDL_MouseButtonEvent button)
{
	switch (button.button)
	{
	case SDL_BUTTON_LEFT:
		if (last_event.type != SDL_MOUSEBUTTONDOWN &&
			last_event.button.button != SDL_BUTTON_LEFT)
		{
			state.left_button_down = true;
		}
		break;
	case SDL_BUTTON_RIGHT:
		if (last_event.type != SDL_MOUSEBUTTONDOWN &&
			last_event.button.button != SDL_BUTTON_RIGHT)
		{
			state.right_button_down = true;
		}
		break;
	}
}

/******************************************************************************
*                                                                             *
*                      EventManager::handleButtonRelease                      *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param button                                                              *
*           The SDL_MouseButtonEvent indicating the button released.          *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Handles a button release, indicating mouse disable.                        *
*                                                                             *
*******************************************************************************/
void EventManager::handleButtonRelease(SDL_MouseButtonEvent button)
{
	switch (button.button)
	{
	case SDL_BUTTON_LEFT:
		state.left_button_down = false;
		break;
	case SDL_BUTTON_RIGHT:
		state.right_button_down = false;
		break;
	}
}

/******************************************************************************
*                                                                             *
*                         EventManager::handleKeyPress()                      *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param key                                                                 *
*           The SDL_Scancode indicating the key which was pressed.            *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Handles a key press event, typically indicating camera motion.             *
*                                                                             *
*******************************************************************************/
void EventManager::handleKeyPress(SDL_Scancode key) 
{	
	glm::vec3 move;
	float scale = 0.25f;

	switch (key) 
	{
	/* Speed Up. */
	case SDL_SCANCODE_T:
		if(*speed < 4.0f)
			*speed += 0.5f;
		break;
	
	/* Slow Down. */
	case SDL_SCANCODE_Y:
		if(*speed < 0.0f)
			*speed -= 0.5f;
		break;

	/* Strafe Right. */
	case SDL_SCANCODE_D:
	case SDL_SCANCODE_RIGHT:
		move = glm::cross(*(camera->getViewDirection()), *(camera->getUpDirection()));
		move = move / glm::length(move);
		*( camera->getPosition() ) += move * *speed;
		break;

	/* Strafe Left. */
	case SDL_SCANCODE_A:
	case SDL_SCANCODE_LEFT:
		move = glm::cross(*( camera->getUpDirection() ), *( camera->getViewDirection() ));
		move = move / glm::length(move);
		*(camera->getPosition()) += move * *speed;
		break;

	/* Step Forward. */
	case SDL_SCANCODE_W:
	case SDL_SCANCODE_UP:
		move = *( camera->getViewDirection() );
		move /= glm::length(move);
		*(camera->getPosition()) += move * *speed;
		break;

	/* Step Backward. */
	case SDL_SCANCODE_S:
	case SDL_SCANCODE_DOWN:
		move = *(camera->getViewDirection());
		move = -1.0f * ( move / glm::length(move) );
		*(camera->getPosition()) += move * *speed;
		break;

	/* Step Down. */
	case SDL_SCANCODE_Z:
		move = *(camera->getUpDirection());
		move /= glm::length(move);
		*(camera->getPosition()) -= move * *speed;
		break;

	/* Step Up. */
	case SDL_SCANCODE_X:
		move = *( camera->getUpDirection() );
		move /= glm::length(move);
		*(camera->getPosition()) += move * *speed;
		break;
	// Forward Slice.
	case SDL_SCANCODE_F:
		if(!(*play)) (*slice)++;
		break;
	// Reverse Slice
	case SDL_SCANCODE_R:
		if (!(*play)) (*slice)--;
		break;

	case SDL_SCANCODE_P:
		*play = !(*play);
		break;
	case SDL_SCANCODE_V:
		*mode = (*mode + 1) % 5;
		field->get_slices(*slice_list, *mode, *threshold);
		*slice = 0;
		break;
	case SDL_SCANCODE_EQUALS:
		*threshold -= THRESHOLD_INCREMENT;
		if (*threshold <= MIN_THRESHOLD)
			*threshold = MIN_THRESHOLD;
		field->get_slices(*slice_list, *mode, *threshold);
		break;
	case SDL_SCANCODE_MINUS:
		*threshold += THRESHOLD_INCREMENT;
		if (*threshold >= MAX_THRESHOLD)
			*threshold = MAX_THRESHOLD;
		field->get_slices(*slice_list, *mode, *threshold);
		break;

	case  SDL_SCANCODE_ESCAPE:
		exit(0);
	}
}