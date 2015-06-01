#pragma once

/******************************************************************************
*                                                                             *
*                              Included Header Files                          *
*                                                                             *
******************************************************************************/
#include <GL\glew.h>
#include <SDL\SDL.h>
#include <glm\glm.hpp>
#include <nifticlib\nifti1.h>
#include <vector>

/******************************************************************************
*                                                                             *
*                           Defined Constants / Macros                        *
*                                                                             *
******************************************************************************/
#define SPLAT_NUM_VERTICES      4
#define SPLAT_NUM_INDICES       8
#define NII_HEADER_SIZE         352
#define A_0_OFFSET              (sizeof(GLfloat) * 0)
#define A_1_OFFSET              (sizeof(GLfloat) * 3)
#define A_2_OFFSET              (sizeof(GLfloat) * 6)
#define A_3_OFFSET              (sizeof(GLfloat) * 9)
#define VERTEX                  0
#define ELEMENT                 1
#define SPHERICAL               0
#define LINEAR                  1
#define PLANAR                  2
#define A_0_ATTRIB              0
#define A_1_ATTRIB              1
#define A_2_ATTRIB	            2
#define A_3_ATTRIB              3
#define AXIAL                   0
#define SAGITTAL                1
#define CORONAL                 2
#define ALL_LINEAR              3
#define ALL_PLANAR              4
#define ALL                     5
#define DEFAULT_THRESHOLD       0.5f
#define THRESHOLD_INCREMENT     0.05f
#define MAX_THRESHOLD           0.9f
#define MIN_THRESHOLD           0.3f
#define ARRAY_SIZE(a)           sizeof(a) / sizeof(*a)
#define PRINT_VEC3(a)			"{ " << a.x << ", " << a.y << ", " << a.z << "}"
#define DEFAULT_POSITION        glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}
#define DEFAULT_COLOR           glm::vec4{1.0f, 1.0f, 1.0f, 0.0f}
#define DEFAULT_MATRIX          glm::mat3()


/******************************************************************************
*                                                                             *
*                             Splat_Vertex     (struct)                       *
*                                                                             *
******************************************************************************/
struct TensorSplat_Vertex
{

	glm::vec3      A_0;
	glm::vec3      A_1;
	glm::vec3      A_2;
	glm::vec3      A_3;

};

/******************************************************************************
*                                                                             *
*                                  TensorSplat      (class)                   *
*                                                                             *
*******************************************************************************
* MEMBERS                                                                     *
*  position                                                                   *
*           The x, y, z, w position of the TensorSplat in world space.        *
*  color                                                                      *
*           The r, g, b, a color of the TensorSplat.                          *
*  matrix                                                                     *
*           The 3 x 3 matrix representing the tensor.                         *
*  inverse                                                                    *
*           The 3 x 3 inverse tensor.                                         *
*  c                                                                          *
*           Array of barycentric values representing the linear, planar, and  *
*           spherical components of the tensor.                               *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Class representing a tensor splat in 3-D space.                            *
*                                                                             *
*******************************************************************************/
class TensorSplat
{

public:

	// Single texture for all TensorSplat objects.
	static GLuint textureID;

	// Initialize TensorSplat texture.
	static void init_texture(const char* filename);
	static void delete_texture();

	// Graphics buffer variables.
	GLuint*        bufferIDs;
	GLuint         vertexArrayID;
	GLenum         drawMode;

	// Attributes of the TensorSplat.
	glm::vec4      position;
	glm::vec4      color;
	glm::mat3      matrix;
	GLfloat        c[3];

	// Constructors.
	TensorSplat();
	TensorSplat(const TensorSplat& other);
	TensorSplat(const glm::vec4& pos, const glm::vec4& color, const glm::mat3 mat);
	TensorSplat& operator=(const TensorSplat& other);
	GLfloat& operator()(GLuint i, GLuint j) { return matrix[i][j]; }
	GLfloat operator()(GLuint i, GLuint j) const { return matrix[i][j]; }

	// Re-position bounding box.
	GLfloat recalculate(glm::vec3 e, glm::vec3 up);

	// Free up stuff on graphics card.
	void cleanUp();

private:

	// Initialization function.
	void init_tensorsplat(const glm::vec4& position, const glm::vec4& color, 
		const glm::mat3 matrix);

};

typedef std::vector<std::vector<TensorSplat*>> SliceList;

/******************************************************************************
*                                                                             *
*                                  TensorField      (class)                   *
*                                                                             *
*******************************************************************************
* MEMBERS                                                                     *
*  x_size                                                                     *
*           The number of tensors aligned on the x-axis.                      *
*  y_size                                                                     *
*           The number of tensors aligned on the y-axis.                      *
*  z_size                                                                     *
*           The number of tensors aligned on the z-axis.                      *
*  field                                                                      *
*           The 3-D array of tensors.                                         *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Class representing a field of tensor splats.                               *
*                                                                             *
*******************************************************************************/
class TensorField {

public:

	// Size of the field.
	GLuint x_size;
	GLuint y_size;
	GLuint z_size;

	// 3-D array of tensors.
	TensorSplat**** field;

	// Constructors.
	TensorField(GLuint x, GLuint y, GLuint z);
	TensorField(const TensorField& other);
	TensorField& operator=(const TensorField& rhs);

	// Deallocate memory.
	void cleanUp();

	void TensorField::get_slices(SliceList& splats, GLuint view_plane, GLfloat threshold);
	static TensorField* read_nifti_file(const std::string nifti_file_path);
	static TensorField* TensorField::read_eig_file(const std::string nifti_file_path,
		std::string eig_file_path);
};

