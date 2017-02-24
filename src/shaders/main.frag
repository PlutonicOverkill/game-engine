#version 450 core

readonly restrict uniform layout(rgba8) image2D image;
layout(location=0) out vec4 color;

void main(void)
{
	color = imageLoad(image, ivec4(gl_FragCoord).xy);
}
