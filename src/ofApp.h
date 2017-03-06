#pragma once

#include "ofMain.h"
#include "Bird.h"
#include "ofVRCamera.h"
#include "Kinect.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        ofMaterial material;
        ofColor color;
        ofMesh mesh;
        of3dPrimitive primitive;
        Bird* bird = nullptr;

		IKinectSensor* kinect = nullptr;
		//ofVRCamera* camera;
    
        ofVideoPlayer vp;
        ofLight pointLight;

		ofShader shader;
		
};
