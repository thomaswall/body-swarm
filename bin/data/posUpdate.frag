#version 150

uniform sampler2DRect prevPosData;  // previous position texture
uniform sampler2DRect velData;      // velocity texture

in vec2 vTexCoord;

out vec4 vFragColor;

void main(void){
    // Get the position and velocity from the pixel color.
    vec3 pos = texture( prevPosData, vTexCoord ).xyz;
    vec3 vel = texture( velData, vTexCoord ).xyz;
    
    // Update the position.
    pos += vel * 0.5;
    
    // And finally store it on the position FBO.
    vFragColor = vec4(pos.x,pos.y,pos.z,1.0);
	//vFragColor = vec4(vel.x, vel.y, vel.z, 1.0);
}