#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;

uniform sampler2DRect velData;   // previous velocity texture
uniform sampler2DRect posData;      // position texture
uniform int resolution;
uniform int width;
uniform int height;
uniform vec3 people_points[1000];
uniform int people_size;

in vec2 vTexCoord;

out vec4 vFragColor;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 avoider(vec3 target, vec3 pos) {
    vec3 steer = pos;
    steer -= target;
    steer *= 3;
    steer *= (1 / (distance(pos, target)*distance(pos, target)));
    return steer;
    
}

vec3 avoidWalls(vec3 pos) {
    vec3 vec = vec3(0,0,0);
    vec3 sum = vec3(0,0,0);
    
    vec = vec3(0, pos.y, pos.z);
    sum += avoider(vec, pos);
    
    vec = vec3(width, pos.y, pos.z);
    sum += avoider(vec, pos);
    
    vec = vec3(pos.x, 0, pos.z);
    sum += avoider(vec, pos);
    
    vec = vec3(pos.x, height, pos.z);
    sum += avoider(vec, pos);
    
    vec = vec3(pos.x, pos.y, 300);
    sum += avoider(vec, pos);
    
    vec = vec3(pos.x, pos.y, -500);
    sum += avoider(vec, pos);
    
    return sum;
}

vec3 bodySwarm(vec3 pos, vec3 vel) {
	float min_neighbor_dist = 300;
	vec3 c_sum = vec3(0, 0, 0);
	int c_count = 0;
	float max_dist = 100000000;

	vec3 steer = vec3(0, 0, 0);
	vec3 add = vec3(0, 0, 0);

	if (rand(normalize(pos.xy)) > 0.7) {
		for (int i = 0; i < people_size; i++) {
			float d = distance(pos, people_points[i]);

			if (d > 0 && d < min_neighbor_dist && d < max_dist) {
				max_dist = d;
				c_sum = people_points[i];
				c_count = 1;
			}
		}
	}

	if (c_count > 0) {
		c_sum /= c_count;
		vec3 seek = c_sum - pos;
		seek = normalize(seek);
		seek *= 3;
		add = seek - vel;
		if(length(add) > 1.0) {
			add = normalize(add);
		}
		steer += add;
	}
	return steer;
}

void main() {

	float min_neighbor_dist = 50;
	float desired_separation = 20*2;
	vec3 steer = vec3(0,0,0);
	vec3 add = vec3(0,0,0);
	vec3 acc_sum = vec3(0,0,0);
	vec3 c_sum = vec3(0,0,0);
	vec3 s_sum = vec3(0,0,0);
	int acc_count = 0;
	int s_count = 0;

	vec3 pos = texture( posData, vTexCoord).xyz;
	vec3 vel = texture( velData, vTexCoord).xyz;
	vec3 birdPosition;
	vec3 birdVelocity;
	float d;

	for(int x=0; x <resolution ;x++) {
		for(int y=0; y< resolution; y++) {
			vec2 ref = vec2( float(x) + 0.5, float(y) + 0.5);
			birdPosition = texture( posData, ref ).xyz;
			birdVelocity = texture( velData, ref ).xyz;

			d = distance(birdPosition, pos);

			if(d > 0.0 && d < min_neighbor_dist) {
				acc_sum += birdVelocity;
				c_sum += birdPosition;
				acc_count++;
			}

			if(d > 0.0 && d < desired_separation) {
				vec3 diff = pos - birdPosition;
				s_sum += (normalize(diff) / d);
				s_count++;
			}
		}
	}

	if(s_count > 0) {
		s_sum /= s_count;
		s_sum = normalize(s_sum);
		s_sum *= 3;
		add = s_sum - vel;
		if(length(add) > 0.1) {
			add = normalize(add) * 0.1;
		}
		if(rand(normalize(add.xy)) > 0.6) {
			steer += add * 1.1;
		}
	}
	steer += add;

	add = vec3(0,0,0);

	if(acc_count > 0) {
		c_sum /= acc_count;
		acc_sum /= acc_count;

		vec3 seek = c_sum - pos;
		seek = normalize(seek);
		seek *= 3;
		add = seek - vel;
		if(length(add) > 0.1) {
			add = normalize(add) * 0.1;
		}
		if(rand(normalize(add.xy)) > 0.6) {
			steer += add * 0.8;
		} 

		acc_sum = normalize(acc_sum);
		acc_sum *= 3;
		add = acc_sum - vel; //same
		if(length(add) > 0.1) {
			add = normalize(add) * 0.1;
		}
		if(rand(normalize(add.xy)) > 0.6) {
			steer += add * 1.4;
		} 
	}
	steer += avoidWalls(pos);
	steer += bodySwarm(pos, vel) * 1.8;

	vel += steer;

	float mag = distance(vel, vec3(0,0,0));
	if(mag > 5) {
		vel /= (mag / 5);
	}

	vFragColor = vec4(vel,1.0);
}