#pragma once
#include "ofMain.h"
#include "ofxUbo.h"
#include "Kinect.h"
#include "ofVRCamera.h"


struct AllVertices
{
	ofVec3f positions[10000];
};

struct pingPongBuffer {
public:
	void allocate(int _width, int _height, int _internalformat = GL_RGBA) {
		// Allocate
		for (int i = 0; i < 2; i++) {
			FBOs[i].allocate(_width, _height, _internalformat);
			FBOs[i].getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
		}

		// Clean
		clear();
	}

	void swap() {
		std::swap(src, dst);
	}

	void clear() {
		for (int i = 0; i < 2; i++) {
			FBOs[i].begin();
			ofClear(0, 255);
			FBOs[i].end();
		}
	}

	ofFbo& operator[](int n) { return FBOs[n]; }
	ofFbo   *src = &FBOs[0];       // Source       ->  Ping
	ofFbo   *dst = &FBOs[1];       // Destination  ->  Pong

private:
	ofFbo   FBOs[2];    // Real addresses of ping/pong FBO«s
};


struct LatticeCube {
public:
    ofVec3f location;
    ofVec3f velocity;
};

class Bird {
    
public:
    Bird(IKinectSensor* _sensor);
    void init(int amt);
	ofVec3f mapIndexToOF(int index);
    void draw();
	void update();
    ofVec3f forces(int index, std::vector<LatticeCube> neighbors);
    ofVec3f cohesion(int index, std::vector<LatticeCube> neighbors);
    ofVec3f alignment(int index, std::vector<LatticeCube> neighbors);
    ofVec3f avoidWalls(int index);
    ofVec3f avoider(ofVec3f target, int index);
	ofVec3f BodySwarm(int index);
    
    ofMaterial material;
    ofColor color;
    ofMesh mesh;
    of3dPrimitive primitive;
    int amount;
	int resolution;
    float boid_dist;
    std::vector<ofVec3f> orth;
    std::vector<ofVec3f> positions;
    std::vector<ofVec3f> velocities;
	std::vector<ofVec3f> last_positions;
    
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
	float people_uniform[3000];
	int people_size;
    
    std::vector<std::vector<std::vector<std::vector<LatticeCube>>>> lattice;
	AllVertices allverts;
	AllVertices allverts2;

	ofShader updatePos;
	ofShader updateVel;
	ofShader updateRender;

	pingPongBuffer posPingPong;
	pingPongBuffer velPingPong;

	ofFbo renderFBO;

	int textureRes;

	int width;
	int height;

	ofImage texImage;
	int imageWidth;
	int imageHeight;

	ofVRCamera* camera;
};