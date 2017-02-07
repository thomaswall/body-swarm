#pragma once
#include "ofMain.h"

class Bird {
    
public:
    Bird();
    void init(int amt);
    void draw();
    ofVec3f forces(int index);
    ofVec3f cohesion(int index);
    ofVec3f alignment(int index);
    ofVec3f avoidWalls(int index);
    ofVec3f avoider(ofVec3f target, int index);
    
    ofMaterial material;
    ofColor color;
    ofMesh mesh;
    of3dPrimitive primitive;
    int amount;
    float boid_dist;
    std::vector<ofVec3f> speeds;
    std::vector<ofVec3f> positions;
    std::vector<ofVec3f> velocities;
    
};
