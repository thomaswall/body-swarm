#version 150

// This fill the billboard made on the Geometry Shader with a texture

in vec2 vTexCoord;
in vec3 fColor;
out vec4 vFragColor;
uniform float imgWidth, imgHeight;

void main() {
    vFragColor = vec4(fColor, 1.0);
}