#include "ofApp.h"

void ofApp::setup(){
    ofBackground(20);
	//ofSetWindowShape(3840, 2160);
    
    ofEnableLighting();
    ofEnableDepthTest();
    ofSetSmoothLighting(true);
    //ofSetVerticalSync(false);

	GetDefaultKinectSensor(&kinect);

	//camera = new ofVRCamera(kinect);
	bird = new Bird(kinect);

    bird->init(5000);
    
    pointLight.setPosition(500, 500, -995);
    pointLight.setPointLight();

}

//--------------------------------------------------------------
void ofApp::update(){
    //vp.update();
	bird->update();
	//camera->update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest();

	//camera->begin();
    bird->draw();

    //ofTexture mesh = vp.getTexture();
    ofDisableLighting();
   // mesh.draw(-800,-700, -1000, 2700, 2200);
    ofEnableLighting();
	//camera->end();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
