#pragma once
#include "ofMain.h"

struct LatticeCube {
public:
    ofVec3f location;
    ofVec3f velocity;
};

class Bird {
    
public:
    Bird();
    void init(int amt);
    void draw();
    ofVec3f forces(int index, std::vector<LatticeCube> neighbors);
    ofVec3f cohesion(int index, std::vector<LatticeCube> neighbors);
    ofVec3f alignment(int index, std::vector<LatticeCube> neighbors);
    ofVec3f avoidWalls(int index);
    ofVec3f avoider(ofVec3f target, int index);
    
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
    
    std::vector<std::vector<std::vector<std::vector<LatticeCube>>>> lattice;
};
