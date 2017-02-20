#pragma once
#include "ofMain.h"
#include "Kinect.h"



class Bird {
    
public:
    Bird();
    void init(int amt);
	ofVec3f mapIndexToOF(int index);
    void draw();
    ofVec3f forces(int index);
    ofVec3f cohesion(int index);
    ofVec3f alignment(int index);
    ofVec3f avoidWalls(int index);
    ofVec3f avoider(ofVec3f target, int index);
	ofVec3f BodySwarm(int index);
    
    ofMaterial material;
    ofColor color;
    ofMesh mesh;
    of3dPrimitive primitive;
    int amount;
    float boid_dist;
    std::vector<ofVec3f> orth;
    std::vector<ofVec3f> positions;
    std::vector<ofVec3f> velocities;
    
    ofLight pointLight;
    ofLight pointLight2;

	IKinectSensor* sensor = nullptr;
	IDepthFrameReader* dfr = nullptr;
	IBodyIndexFrameReader* bifr = nullptr;
	ICoordinateMapper* cm = nullptr;

	uint16_t* depthbuffer = nullptr;
	byte* bibuffer = nullptr;
	CameraSpacePoint* csp = nullptr;
	int depthWidth = 0, depthHeight = 0;

	std::vector<ofVec3f> people_points;
};
