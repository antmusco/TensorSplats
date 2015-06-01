#version 130

precision highp float;

uniform mat4 modelToProjectionMatrix;
uniform mat4 modelToWorldMatrix;

attribute vec4 model_position;
attribute vec4 model_color;

varying vec4 out_position;
varying vec4 out_color;

void main()
{
	out_position = modelToWorldMatrix * model_position;
	out_color = model_color;
	gl_Position = modelToProjectionMatrix * model_position;
}
