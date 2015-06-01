#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "SDL2main.lib" )
#pragma comment( lib, "SDL2_image.lib" )

/******************************************************************************
*                                                                             *
*                              Included Header Files                          *
*                                                                             *
******************************************************************************/
#include "TensorSplat.h"
#include <string>
#include <iostream>
#include <fstream>
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <SDL\SDL_image.h>

// Initialize textureID to 0. 
GLuint TensorSplat::textureID = 0;

// Convenience function for flipping a double from big endian->little endian.
double flip(double byte)
{
	double a;
	unsigned char *dst = (unsigned char*)&a;
	unsigned char *src = (unsigned char*)&byte;

	dst[0] = src[7];
	dst[1] = src[6];
	dst[2] = src[5];
	dst[3] = src[4];
	dst[4] = src[3];
	dst[5] = src[2];
	dst[6] = src[1];
	dst[7] = src[0];

	return a;
}

/******************************************************************************
*                                                                             *
*                         TensorSplat::TensorSplat                            *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  pos                                                                        *
*           Position of the TensorSplat in world space.                       *
*  color                                                                      *
*           The r, g, b, a color of the TensorSplat.                          *
*  mat                                                                        *
*           The 3 x 3 matrix representing the tensor.                         *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Public constructor for the TensorSplat object.                             *
*                                                                             *
*******************************************************************************/
TensorSplat::TensorSplat(const glm::vec4& position, const glm::vec4& color, 
	const glm::mat3 matrix)
{
	init_tensorsplat(position, color, matrix);
}
TensorSplat::TensorSplat()
{
	init_tensorsplat(DEFAULT_POSITION, DEFAULT_COLOR, DEFAULT_MATRIX);
}
TensorSplat::TensorSplat(const TensorSplat& other)
{
	init_tensorsplat(other.position, other.color, other.matrix);
	c[SPHERICAL] = other.c[SPHERICAL];
	c[LINEAR] = other.c[LINEAR];
	c[PLANAR] = other.c[PLANAR];
}
TensorSplat& TensorSplat::operator=(const TensorSplat& other)
{
	if (this != &other)
	{
		init_tensorsplat(other.position, other.color, other.matrix);
		c[SPHERICAL] = other.c[SPHERICAL];
		c[LINEAR] = other.c[LINEAR];
		c[PLANAR] = other.c[PLANAR];
	}
	return *this;
}
void TensorSplat::init_tensorsplat(const glm::vec4& position, 
	const glm::vec4& color, const glm::mat3 matrix)
{
	// Initialize all members.
	this->position = position;
	this->color = color;
	this->matrix = matrix;
	this->c[SPHERICAL] = 0;
	this->c[LINEAR] = 0;
	this->c[PLANAR] = 0;

	// Indices will always be constant.
	GLuint localIndices[] = { 0, 1, 2, 3, };

	// Generate the buffer space.
	bufferIDs = new GLuint[2];
	glGenBuffers(2, bufferIDs);

	// Create vertex buffer.
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TensorSplat_Vertex) * SPLAT_NUM_VERTICES,
		NULL, GL_DYNAMIC_DRAW);

	// Create index buffer.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[ELEMENT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(localIndices),
		localIndices, GL_STATIC_DRAW);

	// Generate Vertex Array Object.
	glGenVertexArrays(1, &vertexArrayID);

	GLenum err = glGetError();
	if (err == GL_OUT_OF_MEMORY)
		std::cout << "Out of memory!" << std::endl;

	//std::cout << "Array #: " << vertexArrayID << std::endl;

	// Bind this vertex array ID.
	glBindVertexArray(vertexArrayID);


	// Bind the vertex buffer.
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);

	// Enable the vertex attributes.
	glEnableVertexAttribArray(A_0_ATTRIB);
	glEnableVertexAttribArray(A_1_ATTRIB);
	glEnableVertexAttribArray(A_2_ATTRIB);
	glEnableVertexAttribArray(A_3_ATTRIB);

	// Vertex position attribute.
	glVertexAttribPointer(A_0_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(TensorSplat_Vertex),
		(void*)A_0_OFFSET);

	// Vertex interpolator attribute.
	glVertexAttribPointer(A_1_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(TensorSplat_Vertex),
		(void*)A_1_OFFSET);

	// Vertex color attribute.
	glVertexAttribPointer(A_2_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(TensorSplat_Vertex),
		(void*)A_2_OFFSET);

	// Vertex color attribute.
	glVertexAttribPointer(A_3_ATTRIB, 3, GL_FLOAT, GL_FALSE, sizeof(TensorSplat_Vertex),
		(void*)A_3_OFFSET);

	// Set the draw mode.
	drawMode = GL_QUADS;

}

GLfloat TensorSplat::recalculate(glm::vec3 e, glm::vec3 up)
{
	// Grab variables from TensorSplat.
	glm::vec3 c     = glm::vec3(position);
	glm::mat3 T     = matrix;
	glm::mat3 T_inv = glm::inverse(T);

	// Calculate parameter-space variables.
	glm::vec3 e_tilda    = T_inv * (e - c);
	glm::vec3 up_tilda   = T_inv * up;
	glm::vec3 z_hat      = -glm::normalize(e_tilda);
	glm::vec3 y_hat      =  glm::normalize(up_tilda);
	glm::vec3 x_hat      =  glm::normalize(glm::cross(z_hat, y_hat));
	GLfloat   mu         = 1.0f / glm::length(e_tilda);
	GLfloat   mu_squared = std::pow(mu, 2);
	glm::vec3 m_tilda    = mu_squared * e_tilda;
	GLfloat   r_tilda    = std::sqrt(1 - mu_squared);

	// Calculate world-space variables.
	glm::vec3 m = (T * m_tilda) + c;
	glm::vec3 x = T * x_hat;
	glm::vec3 y = T * y_hat;

	GLfloat scale = 2.0f;

	// Define local vertices.
	glm::vec3 V[] =
	{
		{ m + (r_tilda * ( x + y) * scale) },
		{ m + (r_tilda * (x - y) * scale) },
		{ m + (r_tilda * (-x - y) * scale) },
		{ m + (r_tilda * (-x + y) * scale) },
	};

	glm::vec3 loc_A_0[] =
	{
		V[0] - e,
		V[1] - e,
		V[2] - e,
		V[3] - e,
	};

	glm::vec3 loc_A_1[] =
	{
		{+1.0f, +1.0f, mu},
		{+1.0f, -1.0f, mu},
		{-1.0f, -1.0f, mu},
		{-1.0f, +1.0f, mu},
	};

	glm::mat3 in_sq = T_inv * T_inv;

	glm::vec3 loc_A_2[] =
	{
		{ in_sq * (e - c) },
		{ in_sq * (e - c) },
		{ in_sq * (e - c) },
		{ in_sq * (e - c) },
	};

	glm::vec3 loc_A_3[] =
	{
		{ in_sq * loc_A_0[0] },
		{ in_sq * loc_A_0[1] },
		{ in_sq * loc_A_0[2] },
		{ in_sq * loc_A_0[3] },
	};

	TensorSplat_Vertex local_verts[] =
	{
		{ loc_A_0[0], loc_A_1[0], loc_A_2[0], loc_A_3[0] },
		{ loc_A_0[1], loc_A_1[1], loc_A_2[1], loc_A_3[1] },
		{ loc_A_0[2], loc_A_1[2], loc_A_2[2], loc_A_3[2] },
		{ loc_A_0[3], loc_A_1[3], loc_A_2[3], loc_A_3[3] },
	};
	
	// Send the data down.
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[VERTEX]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(local_verts), local_verts);

	return r_tilda;
}

/******************************************************************************
*                                                                             *
*                         TensorSplat::initTexture                            *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  filename                                                                   *
*           Path to the texture file to load as the splat texture.            *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Initializes the single texture used by all tensor splats.                  *
*                                                                             *
*******************************************************************************/
void TensorSplat::init_texture(const char* filename)
{
	/* Enable Texture 2D. */
	glEnable(GL_TEXTURE_2D);

	/* If the filename is null, do nothing. */
	if (filename != NULL)
	{
		/* Load the SDL_Surface from the file. */
		SDL_Surface* textureSurface = IMG_Load(filename);

		/* If the image was not loaded correctly, do nothing. */
		if (textureSurface != NULL)
		{
			/* The default color scheme is RGB. */
			GLenum colorScheme = GL_RGB;

			/* If the file is a bitmap, change the color scheme to BGR. */
			std::string file(filename);
			std::string ext = file.substr(file.find('.'), file.length() - 1);
			if (ext == ".bmp")
				colorScheme = GL_BGR;

			/* Generate the texture buffer. */
			glGenTextures(1, &textureID);

			/* Bind the texture ID to the appropriate binding point. */
			glBindTexture(GL_TEXTURE_2D, textureID);

			/* Send the image data down to the graphics card. */
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSurface->w,
				textureSurface->h, 0, colorScheme, GL_UNSIGNED_BYTE,
				textureSurface->pixels);

			/* Set the desred texture parameters. */
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glBindTexture(GL_TEXTURE_2D, TensorSplat::textureID);

			glActiveTexture(GL_TEXTURE0);

		}
	}
}

/******************************************************************************
*                                                                             *
*                         TensorSplat::delete_texture                         *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Deletes the texture from the graphics hardware.                            *
*                                                                             *
*******************************************************************************/
void TensorSplat::delete_texture()
{
	glDeleteTextures(1, &textureID);
}

/******************************************************************************
*                                                                             *
*                              TensorSplat::cleanUp                           *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Deletes the buffers from the graphics hardware.                            *
*                                                                             *
*******************************************************************************/
void TensorSplat::cleanUp()
{
	glDeleteBuffers(2, bufferIDs);
	glDeleteBuffers(1, &vertexArrayID);
}

/******************************************************************************
*                                                                             *
*                         TensorField::TensorField                            *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  x                                                                          *
*           Length of the x-axis of the Tensor Field.                         *
*  y                                                                          *
*           Length of the y-axis of the Tensor Field.                         *
*  z                                                                          *
*           Length of the z-axis of the Tensor Field.                         *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Public constructor for the TensorField object.                             *
*                                                                             *
*******************************************************************************/
TensorField::TensorField(GLuint x, GLuint y, GLuint z) :
x_size(x), y_size(y), z_size(z)
{
	// Allocate x-axis.
	field = new TensorSplat***[x];
	for (GLuint i  = 0; i < x; i++)
	{
		// Allocate y-axis.
		field[i] = new TensorSplat**[y];

		for (GLuint j = 0; j < y; j++)
		{
			// Allocate z-axis.
			field[i][j] = new TensorSplat*[z];

			for (GLuint k = 0; k < z; k++)
			{
				// Initialize to null.
				field[i][j][k] = NULL;
			}
		}
	}
}

/******************************************************************************
*                                                                             *
*                              TensorField::cleanUp                           *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Deletes the previously allocated memory.                                   *
*                                                                             *
*******************************************************************************/
void TensorField::cleanUp()
{
	GLuint i, j, k;

	for (i = 0; i < x_size; i++)
	{
		std::cout << "Deleting " << i << std::endl;
		for (j = 0; j < y_size; j++)
		{
			for (k = 0; k < z_size; k++)
			{
				if (field[i][j][k] != NULL)
				{
					field[i][j][k]->cleanUp();
					delete field[i][j][k];
				}
			}
			delete[] field[i][j];
		}
		delete[] field[i];
	}
	delete[] field;

}

/******************************************************************************
*                                                                             *
*                       TensorField::read_eig_file                            *
*                                                                             *
*******************************************************************************
* PARAMETERS                                                                  *
*  nifti_file_path                                                            *
*           Path to file containing the relevant header information.          *
*  eig_file_path                                                              *
*           Path to the file containing the eigenvector/eigenvalue data.      *
*                                                                             *
*******************************************************************************
* DESCRIPTION                                                                 *
*  Static method which reads an eigenvector/eigenvalue file into a Tensor     *
*  Field object.                                                              *
*                                                                             *
*******************************************************************************/
TensorField* TensorField::read_eig_file(const std::string nifti_file_path, 
	const std::string eig_file_path)
{
	nifti_1_header hdr;
	FILE *fp;
	GLint ret;
	GLdouble* data_double = NULL;
	GLfloat* data_float = NULL;

	// Open and read header file.
	fp = fopen(nifti_file_path.c_str(), "rb");
	if (fp == NULL) {
		fprintf(stderr, "\nError opening header file %s\n", nifti_file_path.c_str());
		return NULL;
	}
	ret = fread(&hdr, NII_HEADER_SIZE, 1, fp);
	if (ret != 1) {
		fprintf(stderr, "\nError reading header file %s\n", nifti_file_path.c_str());
		return NULL;
	}

	// Print some header information.
	fprintf(stderr, "\n%s header information:", nifti_file_path.c_str());
	fprintf(stderr, "\nNumber of dimensions: %d", hdr.dim[0]);
	fprintf(stderr, "\nXYZT dimensions: %d %d %d %d %d", hdr.dim[1], hdr.dim[2], hdr.dim[3], hdr.dim[4], hdr.dim[5]);
	fprintf(stderr, "\nDatatype code and bits/pixel: %d %d", hdr.datatype, hdr.bitpix);
	fprintf(stderr, "\nScaling slope and intercept: %.6f %.6f", hdr.scl_slope, hdr.scl_inter);
	fprintf(stderr, "\nByte offset to data in datafile: %ld", (long)(hdr.vox_offset));
	fprintf(stderr, "\n");

	fclose(fp);

	// Grab the dimensions of the volume.
	GLuint X_DIM  = hdr.dim[1];
	GLuint Y_DIM  = hdr.dim[2];
	GLuint Z_DIM  = hdr.dim[3];
	GLuint STRIDE = 12;
	GLuint size   = X_DIM * Y_DIM * Z_DIM * STRIDE;

	// Open the eigenvector file.
	fp = fopen(eig_file_path.c_str(), "rb");
	if (fp == NULL) {
		fprintf(stderr, "\nError opening header file %s\n", eig_file_path.c_str());
		return NULL;
	}
	// Read the float values out of the file.
	data_float = (GLfloat*)malloc(sizeof(GLfloat) * size); 
	ret = fread(data_float, sizeof(GLfloat), size, fp);

	// Create new tensor field. 
	TensorField* tf = new TensorField(X_DIM, Y_DIM, Z_DIM);

	// Eigenvalues / Eigenvectors.
	GLfloat e_val_1, e_val_2, e_val_3;
	glm::vec3 e_vec_1, e_vec_2, e_vec_3;
	
	// Looping variables.
	TensorSplat* tensor = NULL;
	GLuint index;
	GLfloat scale = 1e9;

	// Parse the data and initialize the tensor field.
	for (GLuint k = 0; k < Z_DIM; k++)
	for (GLuint j = 0; j < Y_DIM; j++)
	for (GLuint i = 0; i < X_DIM; i++)
	{

		// Access the matrix at the index.
		index = (i + (j * X_DIM) + (k * X_DIM * Y_DIM)) * STRIDE;

		// Grab the eigenvalues and eigenvectors.
		e_val_1   = data_float[index +  0] * scale;
		e_vec_1.x = data_float[index +  1];
		e_vec_1.y = data_float[index +  2];
		e_vec_1.z = data_float[index +  3];
		e_val_2   = data_float[index +  4] * scale;
		e_vec_2.x = data_float[index +  5];
		e_vec_2.y = data_float[index +  6];
		e_vec_2.z = data_float[index +  7];
		e_val_3   = data_float[index +  8] * scale;
		e_vec_3.x = data_float[index +  9];
		e_vec_3.y = data_float[index + 10];
		e_vec_3.z = data_float[index + 11];

		// Determine if the tensor is significant or not.
		if (e_val_1 != 0 || e_val_2 != 0 || e_val_3 != 0)
		{
			tensor = tf->field[i][j][k] = new TensorSplat();

			glm::mat3 e_vec_matrix{ e_vec_1, e_vec_2, e_vec_3 };
			glm::mat3 e_val_matrix{ e_val_1, 0, 0, 0, e_val_2, 0, 0, 0, e_val_3 };
			glm::mat3 tensor_matrix = glm::mat3{ e_vec_matrix * e_val_matrix * glm::inverse(e_vec_matrix) };
			GLfloat det = glm::determinant(tensor_matrix);

			//std::cout << det << std::endl;

			// Calculate the barycentric parameters.
			GLfloat sum = e_val_1 + e_val_2 + e_val_3;

			GLfloat max = (e_val_1 > e_val_2) ? e_val_1 : e_val_2;
			        max = (max > e_val_3) ? max : e_val_3;
			GLfloat min = (e_val_1 < e_val_2) ? e_val_1 : e_val_2;
			        min = (min < e_val_3) ? min : e_val_3;
			GLfloat med = sum - (max + min);

			GLfloat c_linear = (max - med) / sum;
			GLfloat c_planar = (2 * (med - min)) / sum;
			GLfloat c_spherical = (3 * min) / sum;
			sum = c_linear + c_planar;
			GLfloat c_f = (sum == 0) ? 0 : c_linear / sum;

			GLfloat alpha = std::exp(-2 * c_spherical );

			// Set the tensor color.
			if (det <= 10 && c_spherical < 0.95)
			{
				glm::vec4 color{ 1.0 - c_f, c_f, 0.0, alpha };
				glm::vec4 position;
				// Set the tensor position.
				position.x = ((hdr.srow_x[0] * i) + (hdr.srow_x[1] * j) + (hdr.srow_x[2] * k)
					+ hdr.srow_x[3]);
				position.y = ((hdr.srow_y[0] * i) + (hdr.srow_y[1] * j) + (hdr.srow_y[2] * k)
					+ hdr.srow_y[3]);
				position.z = ((hdr.srow_z[0] * i) + (hdr.srow_z[1] * j) + (hdr.srow_z[2] * k)
					+ hdr.srow_z[3]);

				tensor = tf->field[i][j][k] = new TensorSplat(position, color, tensor_matrix);
				tensor->c[SPHERICAL] = c_spherical;
				tensor->c[LINEAR] = c_linear;
				tensor->c[PLANAR] = c_planar;
			}
		}
	}

	// Free the float buffer.
	free(data_float);
	fclose(fp);

	// Return the tensor field.
	return tf;
}

void TensorField::get_slices(SliceList& splats, GLuint view_plane, GLfloat threshold)
{
	splats.clear();
	switch (view_plane)
	{
	case AXIAL:
		for (GLuint k = 0; k < z_size; k++)
		{
			splats.push_back(std::vector<TensorSplat*>());
			for (GLuint j = 0; j < y_size; j++)
			for (GLuint i = 0; i < x_size; i++)
				if (field[i][j][k] != NULL)
					splats[k].push_back(field[i][j][k]);
		}
		return;
	case CORONAL:
		for (GLuint j = 0; j < y_size; j++)
		{
			splats.push_back(std::vector<TensorSplat*>());
			for (GLuint i = 0; i < x_size; i++)
			for (GLuint k = 0; k < z_size; k++)
				if (field[i][j][k] != NULL)
					splats[j].push_back(field[i][j][k]);
		}
		return;
	case SAGITTAL:
		for (GLuint i = 0; i < x_size; i++)
		{
			splats.push_back(std::vector<TensorSplat*>());
			for (GLuint k = 0; k < z_size; k++)
			for (GLuint j = 0; j < y_size; j++)
				if (field[i][j][k] != NULL)
					splats[i].push_back(field[i][j][k]);
		}
		return;
	case ALL_LINEAR:
		for (GLuint k = 0; k < z_size; k++)
		{
			splats.push_back(std::vector<TensorSplat*>());
			for (GLuint j = 0; j < y_size; j++)
				for (GLuint i = 0; i < x_size; i++)
					if (field[i][j][k] != NULL)
						if (field[i][j][k]->c[LINEAR] >= threshold)
							splats[0].push_back(field[i][j][k]);
		}
		return;
	case ALL_PLANAR:
		for (GLuint k = 0; k < z_size; k++)
		{
			splats.push_back(std::vector<TensorSplat*>());
			for (GLuint j = 0; j < y_size; j++)
			for (GLuint i = 0; i < x_size; i++)
			if (field[i][j][k] != NULL)
				if (field[i][j][k]->c[PLANAR] >= threshold)
					splats[0].push_back(field[i][j][k]);
		}
		return;
		return;
	case ALL:		
		for (GLuint k = 0; k < z_size; k++)
		{
			splats.push_back(std::vector<TensorSplat*>());
			for (GLuint j = 0; j < y_size; j++)
			for (GLuint i = 0; i < x_size; i++)
				if (field[i][j][k] != NULL)
					splats[0].push_back(field[i][j][k]);
		}
		return;
	}
}