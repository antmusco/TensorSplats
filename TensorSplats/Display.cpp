/******************************************************************************
*                                                                             *
*                              Included Header Files                          *
*                                                                             *
******************************************************************************/
#include <gl\glew.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>
#include <SDL\SDL_video.h>
#include <iostream>
#include "Display.h"
#include "TensorSplat.h"

/******************************************************************************
*                                                                             *
*                          Display::Display (Constructor)                     *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param title                                                               *
*           The title of the window to be displayed in the toolbar.           *
*  @param width                                                               *
*           The width of the window in pixels.                                *
*  @param height                                                              *
*           The height of the window in pixels.                               *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Constructor for the Display class. This constructor creates the window and *
*  the GL context using SDL (Simple Directmedia Layer) and initializes GLEW.  *
*  GLEW (GL Extension Wrangler Library) "binds" all of OpenGL's functions     *
*  to the hardware-specific implementation (OpenGL acts as an Adapter Class)  *
*                                                                             *
*******************************************************************************/
Display::Display(std::string title, GLushort width, GLushort height) :
mesh_shader(nullptr), splat_shader(nullptr)
{
	GLuint x, y;
	getCenterPos(&x, &y, width, height);

	/* Create the SDL window. */
	window = SDL_CreateWindow(title.c_str(), x, y, width, height, 
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	/* Create the SDL GL context. */
	context = SDL_GL_CreateContext(window);

	/* Set the clear color to default. */
	glClearColor(DEFAULT_CLEAR_COLOR);

	// Enable alpha blending.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Initialize GLEW (binds all of OpenGL's functions to the hardware). */
	GLenum status = glewInit();

	/* Validate GLEW initialized correctly. */
	if (status != GLEW_OK)
	{
		/* If the functions were not bound, output error. */
		std::cerr << "Glew failed to initialize!" << std::endl;
		std::cerr << "Status: " << glewGetErrorString(status) << std::endl;
	}

	/* Show the version of GLEW currently being used. */
	fprintf(stdout, "Stats: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	
	/* Update the viewport. */
	updateViewport();

	createShaders();

	t = 0;
	ambient_color  = glm::vec4{ 0.05, 0.05, 0.05, 1.0 };
	diffuse_color  = glm::vec4{ 1.0, 1.0, 1.0, 1.0 };
	specular_color = glm::vec4{ 1.0, 1.0, 1.0, 1.0 };
	shininess = 10;

}


void Display::createShaders()
{

	splat_shader = new Shader(SPLAT_VERTEX_SHADER, SPLAT_FRAGMENT_SHADER);
	splat_shader->use();

	model_to_projection_UL_b = glGetUniformLocation(
		splat_shader->getProgram(), "model_to_projection");
	model_to_world_UL_b = glGetUniformLocation(
		splat_shader->getProgram(), "model_to_world");
	light_position_UL = glGetUniformLocation(
		splat_shader->getProgram(), "light_position");
	ambient_color_UL = glGetUniformLocation(
		splat_shader->getProgram(), "ambient_color");
	diffuse_color_UL = glGetUniformLocation(
		splat_shader->getProgram(), "diffuse_color");
	specular_color_UL = glGetUniformLocation(
		splat_shader->getProgram(), "specular_color");
	shininess_UL = glGetUniformLocation(
		splat_shader->getProgram(), "shininess");
	c_0_UL = glGetUniformLocation(
		splat_shader->getProgram(), "C_0");
	eye_pos_UL = glGetUniformLocation(
		splat_shader->getProgram(), "eye_position");
	color_UL = glGetUniformLocation(
		splat_shader->getProgram(), "color");
	texture_UL = glGetUniformLocation(
		splat_shader->getProgram(), "texture");
}

/******************************************************************************
*                                                                             *
*                        Display::getScreenDimension                          *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param d                                                                   *
*        Dimension to get (height or width).                                  *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  The value of the number of pixels wide the specified dimension is for the  *
*  specified display.                                                         *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Function which returns the width or height of the display.                 *
*                                                                             *
*******************************************************************************/
GLushort Display::getScreenDimension(Dimension d) 
{
	/* Place the dimensions of the screen in an SDL_Rect struct. */
	SDL_Rect display;
	SDL_GetDisplayBounds(0, &display);
	
	/* Return the indicated Dimension. */
	if(d == Dimension::HEIGHT)
		return display.h;
	else if(d == Dimension::WIDTH)
		return display.w;
	else
		return -1;
}

/******************************************************************************
*                                                                             *
*                        Display::getWindownDimension                         *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param d                                                                   *
*        Dimension to get (height or width).                                  *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  The value of the number of pixels wide the specified dimension is for the  *
*  specified window.                                                         *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Function which returns the width or height of the window.                  *
*                                                                             *
*******************************************************************************/
GLushort Display::getWindowDimension(Dimension d)
{
	/* Place the dimensions of the screen in an SDL_Rect struct. */
	GLint h, w;
	SDL_GL_GetDrawableSize(window, &w, &h);

	/* Return the indicated Dimension. */
	if (d == Dimension::HEIGHT)
		return (GLushort) h;
	else if (d == Dimension::WIDTH)
		return (GLushort) w;
	else
		return -1;
}


/******************************************************************************
*                                                                             *
*                              Display::maximize                              *
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
*  Maximizes the SDL Window to the current screen.                            *
*                                                                             *
*******************************************************************************/
void Display::maximize()
{
	/* Maximize the window. */
	SDL_MaximizeWindow(window);
}

/******************************************************************************
*                                                                             *
*                           Display::updateViweport                           *
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
*  Function which gets the width and height of the window and updates the     *
*  aspect ratio, the glViewwport, and the viewToPorojectionMatrix for the     *
*  display.                                                                   *
*                                                                             *
*******************************************************************************/
void Display::updateViewport()
{
	/* Get the width and height of the window and calculate aspect ratio. */
	GLint width, height;
	SDL_GetWindowSize(window, &width, &height);
	aspectRatio = (GLfloat)width / height;

	/* Update the GLviewport. */
	glViewport(0, 0, width, height);

	/* Calculate the View-To-Projection matrix. */
	viewToProjectionMatrix = glm::perspectiveFov((GLfloat) DEFAULT_FOV, 
		(GLfloat) width,  (GLfloat) height, DEFAULT_NEAR_PLANE,
		DEFAULT_FAR_PLANE);

}

/******************************************************************************
*                                                                             *
*                             Display::repaint                                *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param meshes                                                              *
*           Vector of Mesh objects to be displayed to the screen.             *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Function which clears the window by changing all of the pixels to the      *
*  specified color and opacity.                                               *
*                                                                             *
*******************************************************************************/
void Display::repaint(std::vector<TensorSplat*>& splats)
{
	/* Tell OpenGL to clear the color buffer and depth buffer. */
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);	

	/* Get the window dimensions and update the viewport. */
	updateViewport();

	modelToProjectionMatrix = viewToProjectionMatrix * camera.getWorldToViewMatrix();

	t += 0.001f;

	GLfloat x_radius = 40.0f, y_radius = 40.0f;
	light_position = glm::vec4{ cosf(t)  * x_radius, 5.0f, sinf(t) * y_radius, 1.0f };
	
	splat_shader->use();
	
	glUniform4fv(light_position_UL, 1, &(light_position.x));
	glUniform4fv(ambient_color_UL, 1, &(ambient_color.x));
	glUniform4fv(diffuse_color_UL, 1, &(diffuse_color.x));
	glUniform4fv(specular_color_UL, 1, &(specular_color.x));
	glUniform1f(shininess_UL, shininess);
	glUniform4fv(eye_pos_UL, 1, &(camera.getPosition()->x));
	glUniform1i(texture_UL, 0);

	glm::vec3 cam_view = *camera.getViewDirection();
	glm::vec3 cam_right_side = glm::cross(cam_view, *camera.getUpDirection());
	glm::vec3 cam_up = glm::normalize(glm::cross(cam_right_side, cam_view));

	glm::mat4 id;
	for (TensorSplat* splat : splats)
	{	
		splat->recalculate(*camera.getPosition(), cam_up);
	
		/* Bind the appropriate Vertex Array. */
		glBindVertexArray(splat->vertexArrayID);
	
		/* Bind the appropriate Index Array. */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, splat->bufferIDs[1]);

		glm::vec3 eye_position = *(camera.getPosition());
		glUniformMatrix4fv(model_to_world_UL_b, 1, GL_FALSE,
			&(id[0][0]));
		glUniformMatrix4fv(model_to_projection_UL_b, 1, GL_FALSE,
			&modelToProjectionMatrix[0][0]);
		glUniform3fv(c_0_UL, 1, &(eye_position.x));
		glUniform4fv(color_UL, 1, &(splat->color.x));
	
		glDrawElements(splat->drawMode, 4, GL_UNSIGNED_INT, 0);
	}

	/* Swap the double buffer. */
	SDL_GL_SwapWindow(window);

}

/******************************************************************************
*                                                                             *
*                             Display::setShader                              *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  @param shader                                                              *
*        The shader object to be used for rendering the geometry.             *
*                                                                             *
*******************************************************************************
* RETURNS                                                                     *
*  void                                                                       *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Tells OpenGL to use the indicated shader for rendering, and gets the       *
*  location IDs of the uniform variables inside the shader program.           *
*                                                                             *
*******************************************************************************/
void Display::setShader(Shader* shader)
{
	/* Tell OpenGL to use this shader. */
	shader->use();

	/* Bind the indicated data attributes to the variables. */
	glBindAttribLocation(shader->getProgram(), A_0_ATTRIB, "A_0");
	glBindAttribLocation(shader->getProgram(), A_1_ATTRIB, "A_1");
	glBindAttribLocation(shader->getProgram(), A_2_ATTRIB, "A_2");
	glBindAttribLocation(shader->getProgram(), A_3_ATTRIB, "A_3");
}

void Display::getCenterPos(GLuint* x, GLuint* y, GLuint width, GLuint height)
{
	*x = (getScreenDimension(Dimension::WIDTH) - width) / 2;
	*y = (getScreenDimension(Dimension::HEIGHT) - width) / 2;
}

/******************************************************************************
*                                                                             *
*                           Display::~Display (Destructor)                    *
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
*  Constructor for the Display class. This constructor creates the window and *
*  the GL context using SDL (Simple Directmedia Layer) and initializes GLEW.  *
*  GLEW (GL Extension Wrangler Library) "binds" all of OpenGL's functions     *
*  to the hardware-specific implementation (OpenGL acts as an Adapter Class)  *
*                                                                             *
*******************************************************************************/
Display::~Display()
{
	/* Delete shaders. */
	delete mesh_shader;
	delete splat_shader;

	/* Delete the GL context. */
	SDL_GL_DeleteContext(context);

	/* Destroy the window. */
	SDL_DestroyWindow(window);
}
