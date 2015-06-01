#pragma once

/******************************************************************************
 *                                                                            *
 *                              Included Header Files                         *
 *                                                                            *
 ******************************************************************************/
#include <SDL\SDL.h>
#include <GL\GL.h>
#include <glm\glm.hpp>
#include <string>
#include <vector>
#include "Camera.h"
#include "TensorSplat.h"
#include "Shader.h"

/******************************************************************************
 *                                                                            *
 *                        Defined Constants and Macros                        *
 *                                                                            *
 ******************************************************************************/

/* Field of View parameter. */
#define  DEFAULT_FOV              (60.0f * M_PI / 180.0)
/* Near clipping plane parameter. */
#define  DEFAULT_NEAR_PLANE       1.0f
/* Far clipping plane parameter. */
#define  DEFAULT_FAR_PLANE        1000.0f
#define  DEFAULT_CLEAR_COLOR      0.1f, 0.1f, 0.1f, 1.0f
/* Default vertex and fragment shader source files. */
#define  SPLAT_VERTEX_SHADER      "res/shaders/splat.vs"
#define  SPLAT_FRAGMENT_SHADER    "res/shaders/splat.fs"

/******************************************************************************
 *																			  *
 *	                           Dimension Enum                                 *
 *																			  *
 ******************************************************************************
 *  WIDTH                                                                     *
 *       Represents the width of an object.                                   *
 *  HEIGHT                                                                    *
 *       Represents the height of an object.                                  *
 *  DEPTH                                                                     *
 *       Represents the depht of an object.                                   *
 *                                                                            *
 ******************************************************************************
 * DESCRIPTION                                                                *
 *  Enumeration specifying valid dimensions for function calls.               *
 *                                                                            *
 ******************************************************************************/
enum class Dimension
{
	WIDTH,
	HEIGHT,
	DEPTH,
};

/******************************************************************************
 *																			  *
 *	                             Display Class                                *
 *																			  *
 ******************************************************************************
 *  aspectRatio                                                               *
 *          Ratio describing the width/height for the viewport.               *
 *  window                                                                    *
 *          Pointer to the SDL_Window instance for this display.              *
 *  context                                                                   *
 *          Pointer to the OpenGL context created for this window.            *
 *          drawn.                                                            *
 *  camera                                                                    *
 *          Camera instance whose perspective this display shows.             *
 *  modelToProjectionMatrix                                                   *
 *          4-D matrix representing the total transformation to the display.  *
 *  viewToProjectionMatrix                                                    *
 *          4-D matrix representing the transformation from the view to the   *
 *          projection (camera view).                                         *
 *  modelToProjectionUniformLocation                                          *
 *          ID  of the location for the modelToProjectionMatrix in the shader *
 *          program.                                                          *
 *  textureUniformLocation                                                    *
 *          ID  of the location for the texture sampler in the shader program *
 *                                                                            *
 ******************************************************************************
 * DESCRIPTION                                                                *
 *  Class representing the window in which the OpenGL context may render.     *
 *                                                                            *
 ******************************************************************************/
class Display
{
/* Public Members. */
public:

	/* Constructor. */
	         Display(std::string title, 
	                 GLushort    width, 
	                 GLushort    height);

	/* Calculate the width and height of the screen dimensions. */
	GLushort getScreenDimension(Dimension d);
	GLushort getWindowDimension(Dimension d);

	/* Update the size of the viewport on window resize. */
	void     updateViewport();

	/* Maximnize the display on the current screen. */
	void     maximize();

	/* Repaint the graphics. */
	void     repaint(std::vector<TensorSplat*>& splats);
	void     getCenterPos(GLuint* x, GLuint* y, GLuint width, GLuint height);

	/* Getters. */
	Camera*  getCamera()               {  return &camera;            }

	/* Setters. */     
	void    setShader(Shader* shader);
	void    createShaders();
	void    setClearColor(GLclampf r, 
                          GLclampf b,
                          GLclampf g, 
                          GLclampf a) {  glClearColor(r, b, g, a);  } 

	/* Destructor. */
	               ~Display();

/* Private Members.*/
private:

	/* Current aspect ratio of the window. */
	GLfloat        aspectRatio;
	/* Pointer to the SDL Window. */
	SDL_Window*    window;
	/* Pointer to the GL Context. */
	SDL_GLContext  context;
	/* Camera for looking at the world. */
	Camera         camera;
	/* Model to Projection (complete) matrix. */
	glm::mat4      modelToProjectionMatrix;
	/* View to Projection matrix. */
	glm::mat4      viewToProjectionMatrix;
	/* Uniform location for the full transformation. */
	GLuint         model_to_projection_UL_a;
	/* Uniform location for the model to world transformation.*/
	GLuint         model_to_world_UL_a;
	/* Uniform location for the full transformation. */
	GLuint         model_to_projection_UL_b;
	/* Uniform location for the model to world transformation.*/
	GLuint         model_to_world_UL_b;
	/* Uniform location for the light source. */
	GLuint         light_position_UL;
	/* Uniform location for the ambient light. */
	GLuint         ambient_color_UL;
	/* Uniform location for the diffuse light. */
	GLuint         diffuse_color_UL;
	/* Uniform location for the specular light. */
	GLuint         specular_color_UL;
	/* Uniform location for the phong shininess parameter. */
	GLuint         shininess_UL;
	GLuint         eye_pos_UL;
	GLuint         color_UL;
	GLuint         texture_UL;

	glm::vec4 ambient_color;
	glm::vec4 diffuse_color;
	glm::vec4 specular_color;
	glm::vec4 light_position;
	GLfloat   shininess;
	GLfloat   t;

	GLuint         a_2_UL;
	GLuint         c_0_UL;
	GLuint         u_UL;

	Shader*        mesh_shader;
	Shader*        splat_shader;
	bool           once;
};
