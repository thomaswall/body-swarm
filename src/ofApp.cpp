#include "ofApp.h"

void ofApp::setup(){
    ofBackground(20);
    bird.init(3000);
    
    //ofEnableLighting();
    //ofEnableDepthTest();
    //ofSetSmoothLighting(true);
    //ofSetVerticalSync(true);
    
    vp.load("/Users/thomaswall/Desktop/movie1.mov");
    vp.play();
    
    pointLight.setPosition(500, 500, -995);
    pointLight.setPointLight();

}

//--------------------------------------------------------------
void ofApp::update(){
    vp.update();
	bird.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest();
    bird.draw();

    ofTexture mesh = vp.getTexture();
    ofDisableLighting();
   // mesh.draw(-800,-700, -1000, 2700, 2200);
    ofEnableLighting();
    
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
