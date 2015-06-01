#version 130

precision highp float;

// Transformation data.
uniform   mat4  model_to_projection;
uniform   mat4  model_to_world;
uniform   vec3  C_0;
uniform vec4  eye_position;

varying   vec4  out_position;
varying   vec4  out_eye_position;
varying   vec3  A_0_inter;
varying   vec3  A_1_inter;
varying   vec3  A_2_inter;
varying   vec3  A_3_inter;
varying   vec2  tex_coord;

attribute vec3  A_0;
attribute vec3  A_1;
attribute vec3  A_2;
attribute vec3  A_3;

void main()
{
	out_position = model_to_world * vec4(A_0 + C_0, 1.0);
	out_eye_position = model_to_world * eye_position;

	A_0_inter = A_0;
	A_1_inter = A_1;
	A_2_inter = A_2;
	A_3_inter = A_3;

	tex_coord = 0.5 * vec2(A_1.x + 1.0, A_1.y + 1.0);

	gl_Position = model_to_projection * vec4(A_0 + C_0, 1.0);
}
