#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(20);
    bird = *new Bird();
    bird.init(1500);
    
    ofEnableLighting();
    ofEnableDepthTest();
    ofSetSmoothLighting(true);
    
}

//--------------------------------------------------------------
void ofApp::update(){
//    primitive.move(ofVec3f(cos(ofGetElapsedTimef()*5),sin(ofGetElapsedTimef()*5),0));
//    ofVec3f v1 = primitive.getMesh().getVertex(0);
//    ofVec3f v2 = primitive.getMesh().getVertex(5);
//    primitive.getMesh().setVertex(0, ofVec3f(v1.x, v1.y, v1.z + 2*sin(ofGetElapsedTimef()*10)));
//    primitive.getMesh().setVertex(5, ofVec3f(v2.x, v2.y, v2.z + 2*sin(ofGetElapsedTimef()*10)));
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest();
    
    ofLog(OF_LOG_NOTICE, ofToString(ofGetSeconds()));
    
    color.r = ofRandom(255.0f);
    color.g = ofRandom(255.0f);
    color.b = 150.0f;
    color.a = 255.0f;
    
    bird.draw();
    
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
