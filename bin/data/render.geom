#version 150

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 12) out;

uniform mat4 modelViewProjectionMatrix;

//uniform sampler2DRect posTex;
//uniform sampler2DRect velTex;

uniform float size;
uniform float imgWidth, imgHeight;

in vec3 vVelocity[];
in vec4 vPosition[];
in vec2 texcoord[];
in int vertexid[];

out vec2 vTexCoord;
out vec3 fColor;
uniform float time;

void main(void){

    // For each vertex moved to the right position on the vertex shader
    // it makes 6 more vertex that makes 2 GL_TRIANGLE_STRIP
    // that´s going to be the frame for the pixels of the sparkImg texture
	fColor = vec3(1.0, 0.0, 1.0);

    for(int i = 0; i < gl_in.length(); i++){
		vec3 orth = cross(vVelocity[i], vec3(0, vPosition[i].y, 0));
		orth = normalize(orth);
		vec3 wingd = cross(vVelocity[i], orth);
    	wingd = normalize(wingd);

		vec3 t_vel = normalize(vVelocity[i]);

		float angle = sin(acos(dot(vec3(0,1,0), t_vel)));
		float sunlight = 0.3 + angle / 4;

        gl_Position = modelViewProjectionMatrix * ( vPosition[i] + vec4((orth*size + wingd*sin((time + float(vertexid[i])/2.0)*6)*(size*0.7)), 0.0));
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
		fColor = vec3(0.0, 0.0, 0.0);
        EmitVertex();
        
        gl_Position = modelViewProjectionMatrix * (vPosition[i]);
        vTexCoord.x = imgWidth;
        vTexCoord.y = 0.0;
		fColor = vec3(0.0, 0.0, 0.0);
        EmitVertex();
        
        gl_Position = modelViewProjectionMatrix * (vPosition[i] + vec4(t_vel*size,0.0));
        vTexCoord.x = imgWidth;
        vTexCoord.y = imgHeight;
		fColor = vec3(0.0, 0.0, 0.0);
        EmitVertex();

        gl_Position = modelViewProjectionMatrix * (vPosition[i]);
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
		fColor = vec3(0.0, 0.0, 0.0);
        EmitVertex();
        
        gl_Position = modelViewProjectionMatrix * (vPosition[i] + vec4(t_vel*size,0.0));
        vTexCoord.x = 0.0;
        vTexCoord.y = imgHeight;
		fColor = vec3(0.0, 0.0, 0.0);
        EmitVertex();
        
        gl_Position = modelViewProjectionMatrix * (vPosition[i] - vec4(orth*size,0.0) + vec4(wingd*sin((time + float(vertexid[i])/2.0)*6)*(size*0.7), 0.0));
        vTexCoord.x = imgWidth;
        vTexCoord.y = imgHeight;
		fColor = vec3(0.0, 0.0, 0.0);
        EmitVertex();

		gl_Position = modelViewProjectionMatrix * (vPosition[i] - vec4(orth*size,0.0) + vec4(wingd*sin((time + float(vertexid[i])/2.0)*6)*(size*0.7), 0.0) - vec4(0,0.1,0,0));
        vTexCoord.x = imgWidth;
        vTexCoord.y = imgHeight;
		fColor = vec3(sunlight, sunlight, sunlight);
        EmitVertex();

		gl_Position = modelViewProjectionMatrix * (vPosition[i] + vec4(t_vel*size,0.0) - vec4(0,0.1,0,0));
        vTexCoord.x = 0.0;
        vTexCoord.y = imgHeight;
		fColor = vec3(sunlight, sunlight, sunlight);
        EmitVertex();

		gl_Position = modelViewProjectionMatrix * (vPosition[i] - vec4(0,0.1,0,0));
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
		fColor = vec3(sunlight, sunlight, sunlight);
        EmitVertex();

		gl_Position = modelViewProjectionMatrix * (vPosition[i] + vec4(t_vel*size,0.0) - vec4(0,0.1,0,0));
        vTexCoord.x = imgWidth;
        vTexCoord.y = imgHeight;
		fColor = vec3(sunlight, sunlight, sunlight);
        EmitVertex();

		gl_Position = modelViewProjectionMatrix * (vPosition[i] - vec4(0,0.1,0,0));
        vTexCoord.x = imgWidth;
        vTexCoord.y = 0.0;
		fColor = vec3(sunlight, sunlight, sunlight);
        EmitVertex();

		gl_Position = modelViewProjectionMatrix * ( vPosition[i] + vec4((orth*size + wingd*sin((time + float(vertexid[i])/2.0)*6)*(size*0.7)), 0.0) - vec4(0,0.1,0,0));
        vTexCoord.x = 0.0;
        vTexCoord.y = 0.0;
		fColor = vec3(sunlight, sunlight, sunlight);
        EmitVertex();


        EndPrimitive();
    }
}