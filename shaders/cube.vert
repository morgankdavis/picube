#version 110

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec3 vPos;
attribute vec3 vNorm;
attribute vec3 vColor;

varying vec3 color;
varying vec3 norm;

void main()
{
	vec3 vPosition_eye = vec3(view * model * vec4(vPos, 1.0));

	gl_Position = projection * vec4(vPosition_eye, 1.0);

    color = vColor;
	norm = vNorm; // not used, but compiler was optimizing it out
}
