#version 130

precision highp float;

varying vec4 out_position;
varying vec4 out_color;

void main()
{
    gl_FragColor = out_color;
}
