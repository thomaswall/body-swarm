#include "Bird.h"

Bird::Bird() { }

void Bird::init(int amt) {
    
    amount = amt;
    boid_dist = 20;
    
    for(int i=0; i < amount; i++) {
        ofVec3f origin = ofVec3f(
            ofRandom(0, 1000),
            ofRandom(0, 800),
            ofRandom(-1000, 300)
        );
        
        velocities.push_back(ofVec3f(ofRandom(-1, 1),ofRandom(-1, 1),ofRandom(-1, 1)));
        positions.push_back(origin);
        //speeds[amount] = {ofVec3f(1.0,1.0,1.0)};
        //positions[i] = origin;

        primitive.getMesh().addVertex(ofVec3f(origin.x - 10,origin.y,origin.z));
        primitive.getMesh().addVertex(ofVec3f(origin.x,origin.y,origin.z));
        primitive.getMesh().addVertex(ofVec3f(origin.x,origin.y + 10,origin.z));
        primitive.getMesh().addVertex(ofVec3f(origin.x,origin.y,origin.z));
        primitive.getMesh().addVertex(ofVec3f(origin.x,origin.y + 10,origin.z));
        primitive.getMesh().addVertex(ofVec3f(origin.x + 10,origin.y,origin.z));
    }
    primitive.getMesh().setMode(OF_PRIMITIVE_TRIANGLES);
}

void Bird::draw() {
    for(int i = 0; i < amount;i++) {
        if(ofRandom(0, 10) > 5) {
            velocities[i] += forces(i) * 1.1;
            velocities[i] += alignment(i) * 1.4;
            velocities[i] += cohesion(i) * 0.8;
        }
        
        velocities[i] += avoidWalls(i);

        float mag = velocities[i].distance(ofVec3f(0,0,0));
        if(mag > 5) {
            velocities[i] /= (mag / 5);
        }
        
        positions[i] += velocities[i];
        
        primitive.getMesh().setVertex(i*6, ofVec3f(positions[i].x - 10,positions[i].y,positions[i].z));
        primitive.getMesh().setVertex(i*6 + 1, ofVec3f(positions[i].x,positions[i].y,positions[i].z));
        primitive.getMesh().setVertex(i*6 + 2, ofVec3f(positions[i].x,positions[i].y + 10,positions[i].z));
        primitive.getMesh().setVertex(i*6 + 3, ofVec3f(positions[i].x,positions[i].y,positions[i].z));
        primitive.getMesh().setVertex(i*6 + 4, ofVec3f(positions[i].x,positions[i].y + 10,positions[i].z));
        primitive.getMesh().setVertex(i*6 + 5, ofVec3f(positions[i].x + 10,positions[i].y,positions[i].z));
        
    }
    primitive.draw();
}

ofVec3f Bird::avoidWalls(int index) {
    ofVec3f vec = ofVec3f();
    ofVec3f sum = ofVec3f(0,0,0);
    
    vec.set(0, positions[index].y, positions[index].z);
    sum += avoider(vec, index);
    
    vec.set(1000, positions[index].y, positions[index].z);
    sum += avoider(vec, index);
    
    vec.set(positions[index].x, 0, positions[index].z);
    sum += avoider(vec, index);
    
    vec.set(positions[index].x, 800, positions[index].z);
    sum += avoider(vec, index);
    
    vec.set(positions[index].x, positions[index].y, 300);
    sum += avoider(vec, index);
    
    vec.set(positions[index].x, positions[index].y, -1000);
    sum += avoider(vec, index);
    
    return sum;
}

ofVec3f Bird::avoider(ofVec3f target, int index) {
    ofVec3f steer = ofVec3f(positions[index]);
    steer -= target;
    steer *= 3;
    steer *= (1 / positions[index].squareDistance(target));
    return steer;
    
}

ofVec3f Bird::alignment(int index) {
    float min_neighbor_dist = 50;
    ofVec3f acc_sum = ofVec3f(0,0,0);
    int acc_count = 0;
    
    for(int i = 0; i < amount; i++) {
        float d = positions[index].distance(positions[i]);
        
        if(d > 0 && d < min_neighbor_dist) {
            //alignment
            acc_sum += velocities[i];
            acc_count++;
        }
    }
    
    ofVec3f steer = ofVec3f(0,0,0);
    ofVec3f add = ofVec3f(0,0,0);
    
    if(acc_count > 0) {
        acc_sum /= acc_count;
        acc_sum.normalize();
        acc_sum *= 3;
        add = acc_sum - velocities[index];
        add.limit(0.1);
        if(ofRandom(0, 10) > 6)
            steer += add;
    }
    
    return steer;
}

ofVec3f Bird::cohesion(int index) {
    float desired_separation = 10*2;
    float min_neighbor_dist = 50;

    ofVec3f c_sum = ofVec3f(0,0,0);
    
    int s_count = 0;
    int acc_count = 0;
    int c_count = 0;
    
    for(int i = 0; i < amount; i++) {
        float d = positions[index].distance(positions[i]);
        
        if(d > 0 && d < min_neighbor_dist) {
            
            //cohesion
            c_sum += positions[i];
            c_count++;
        }
    }
    
    ofVec3f steer = ofVec3f(0,0,0);
    ofVec3f add = ofVec3f(0,0,0);
    
    if(c_count > 0) {
        c_sum /= c_count;
        ofVec3f seek = c_sum - positions[index];
        seek.normalize();
        seek *= 3;
        add = seek - velocities[index];
        add.limit(0.1);
        if(ofRandom(0, 10) > 6)
            steer += add;
    }
    
    return steer;
}

ofVec3f Bird::forces(int index) {
    float desired_separation = 20*2;
    float min_neighbor_dist = 40;
    
    ofVec3f s_sum = ofVec3f(0,0,0);
    ofVec3f acc_sum = ofVec3f(0,0,0);
    ofVec3f c_sum = ofVec3f(0,0,0);
    
    int s_count = 0;
    int acc_count = 0;
    int c_count = 0;
    
    for(int i = 0; i < amount; i++) {
        float d = positions[index].distance(positions[i]);
        
        //separation
        if(d > 0 && d < desired_separation) {
            ofVec3f diff = positions[index] - positions[i];
            diff.normalize();
            s_sum = s_sum + (diff / d);
            s_count++;
        }
    }
    
    ofVec3f steer = ofVec3f(0,0,0);
    ofVec3f add = ofVec3f(0,0,0);
    
    if (s_count > 0) {
        s_sum /= s_count;
        s_sum.normalize();
        s_sum *= 3;
        add = s_sum - velocities[index];
        add.limit(0.1);
        if(ofRandom(0, 10) > 6)
            steer += add;
    }
   steer += add * 1.0;
    
    return steer;
}