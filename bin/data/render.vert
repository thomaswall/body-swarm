#version 150

uniform mat4 modelViewMatrix;

in vec2 texcoord;
in vec4 color;
in int gl_VertexID;

uniform sampler2DRect posTex;
uniform sampler2DRect velTex;
uniform vec3 screen;

out vec3 vVelocity;
out vec4 vPosition;
out vec2 vTexCoord;
out vec4 vColor;
out int vertexid;

void main() {
    // Read position data from texture.
    vec4 pixVel = texture( velTex, texcoord );
	vec4 pixPos = texture( posTex, texcoord );
    
    // Map the position from the texture (from 0.0 to 1.0) to
    // the screen position (0 - screenWidth/screenHeight)
    //pixPos.x *= screen.x;
    //pixPos.y *= screen.y;
	//pixPos.z *= screen.z;
    
	vVelocity = pixVel.xyz;
    vPosition = pixPos;
    vTexCoord = texcoord;
    vColor = color;
	vertexid = gl_VertexID;
}