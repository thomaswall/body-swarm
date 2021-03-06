#include "Bird.h"
#include <math.h>

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
        orth.push_back(ofVec3f(1,0,0));
        //speeds[amount] = {ofVec3f(1.0,1.0,1.0)};
        //positions[i] = origin;

        primitive.getMesh().addVertex(origin - orth[i].normalize()*10);
        primitive.getMesh().addVertex(origin);
        primitive.getMesh().addVertex(origin + velocities[i].normalize()*10);
        primitive.getMesh().addVertex(origin);
        primitive.getMesh().addVertex(origin + velocities[i].normalize()*10);
        primitive.getMesh().addVertex(origin + orth[i].normalize()*10);
    }
    primitive.getMesh().setMode(OF_PRIMITIVE_TRIANGLES);
    

    material.setEmissiveColor(ofColor(200, 0, 255, 255));
    material.setAmbientColor(ofColor(180, 0, 230, 255));
    material.setShininess(20);
    
    pointLight.setPosition(0, 500, -200);
    pointLight.setPointLight();
    pointLight2.setPosition(1000, 500, -200);
    pointLight2.setPointLight();
    
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    for(int i =0; i<20; i++) {
        lattice.push_back(std::vector<std::vector<std::vector<LatticeCube>>>());
        for(int j=0; j<20; j++) {
            lattice[i].push_back(std::vector<std::vector<LatticeCube>>());
            for(int k=0; k<20; k++) {
                lattice[i][j].push_back(std::vector<LatticeCube>());
            }
        }
    }
    
}

void Bird::draw() {
    
    for(int i =0; i<20; i++) {
        for(int j=0; j<20; j++) {
            for(int k=0; k<20; k++) {
                lattice[i][j][k].clear();
            }
        }
    }
    
    int resolution = 20;
    LatticeCube lc = *new LatticeCube();
    lc.location = ofVec3f(0,0,0);
    lc.velocity = ofVec3f(0,0,0);

    for(int i = 0; i < amount; i++) {
        int x = positions[i].x / 1000 * resolution;
        int y = positions[i].y / 1000 * resolution;
        int z = -1*(positions[i].z - 300) / 1300 * resolution;
        
        lc.location = positions[i];
        lc.velocity = velocities[i];
        
        
        lattice[x][y][z].push_back(lc);
    }
    
    
    for(int i = 0; i < amount;i++) {
        ofVec3f v1 = velocities[i];
        v1.normalize();
        
        
           
        if(ofRandom(0, 10) > 5) {
            int x = positions[i].x / 1000 * resolution;
            int y = positions[i].y / 1000 * resolution;
            int z = -1*(positions[i].z - 300) / 1300 * resolution;
            
            std::vector<LatticeCube> neighbors;
            for(int j=x-1;j<=x+1;j+=2) {
                for(int k=y-1;k<=y+1;k+=2) {
                    for(int l=x-1;l<=x+1;l+=2) {
                        if(j > 0 && k > 0 && l > 0 && j < 19 && k < 19 && l < 19) {
                            neighbors.insert(neighbors.end(), lattice[j][k][l].begin(), lattice[j][k][l].end());
                        }
                    }
                }
            }
            
            if(x > 0 && y > 0 && z > 0 && x < 19 && y < 19 && z < 19)
                neighbors.insert(neighbors.end(), lattice[x][y][z].begin(), lattice[x][y][z].end());
            
            velocities[i] += forces(i, neighbors) * 1.1;
            velocities[i] += alignment(i, neighbors) * 1.4;
            velocities[i] += cohesion(i, neighbors) * 0.8;
        }
        
        velocities[i] += avoidWalls(i);

        float mag = velocities[i].distance(ofVec3f(0,0,0));
        if(mag > 5) {
            velocities[i] /= (mag / 5);
        }
        
        ofVec3f v2 = velocities[i];
        v2.normalize();
        
        positions[i] += velocities[i];
        
        
        orth[i] = velocities[i].getPerpendicular(ofVec3f(0, positions[i].y, 0));
        orth[i].normalize();
        float angle = - ofDegToRad(ofVec3f(v1.x, 0, v1.y).angle(ofVec3f(v2.x, 0, v2.y)));

        
        ofMatrix3x3 mat = ofMatrix3x3(
                (1-cos(angle))*v2.x*v2.x + cos(angle), (1-cos(angle))*v2.x*v2.y - sin(angle)*v2.z, (1-cos(angle))*v2.x*v2.z + sin(angle)*v1.y,
                (1-cos(angle))*v2.x*v2.y + sin(angle)*v2.z, (1-cos(angle))*v2.y*v2.y + cos(angle), (1-cos(angle))*v2.y*v2.z - sin(angle)*v2.x,
                (1-cos(angle))*v2.x*v2.z - sin(angle)*v1.y, (1-cos(angle))*v2.y*v2.z + sin(angle)*v2.x, (1-cos(angle))*v2.z*v2.z + cos(angle));
        
        ofVec4f temp = ofMatrix4x4(
                 mat.a, mat.b, mat.c, 0,
                 mat.d, mat.e, mat.f, 0,
                 mat.g, mat.h, mat.i, 0,
                 0, 0, 0, 1).postMult(ofVec4f(orth[i].x, orth[i].y, orth[i].z, 0));
        
        
        orth[i] = ofVec3f(temp.x, temp.y, temp.z);
        
        
        
        ofVec3f t_vel = velocities[i];
        t_vel.normalize();
        
        ofVec3f wingd = velocities[i].getPerpendicular(orth[i]);
        wingd.normalize();
        
        primitive.getMesh().setVertex(i*6, positions[i] - orth[i]*10 + wingd*sin((ofGetElapsedTimef() + i/2)*6)*7);
        primitive.getMesh().setVertex(i*6 + 1, positions[i]);
        primitive.getMesh().setVertex(i*6 + 2, positions[i] + t_vel*10);
        primitive.getMesh().setVertex(i*6 + 3, positions[i]);
        primitive.getMesh().setVertex(i*6 + 4, positions[i] + t_vel*10);
        primitive.getMesh().setVertex(i*6 + 5, positions[i] + orth[i]*10 + wingd*sin((ofGetElapsedTimef() + i/2)*6)*7);
        
    }
    
    material.begin();
    pointLight.enable();
    pointLight2.enable();
    primitive.draw();
    pointLight.disable();
    pointLight2.disable();
    material.end();
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

ofVec3f Bird::alignment(int index, std::vector<LatticeCube> neighbors) {
    float min_neighbor_dist = 50;
    ofVec3f acc_sum = ofVec3f(0,0,0);
    int acc_count = 0;
    
    for(int i = 0; i < neighbors.size(); i++) {
        float d = positions[index].distance(neighbors[i].location);
        
        if(d > 0 && d < min_neighbor_dist) {
            //alignment
            acc_sum += neighbors[i].velocity;
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

ofVec3f Bird::cohesion(int index, std::vector<LatticeCube> neighbors) {
    float desired_separation = 10*2;
    float min_neighbor_dist = 50;

    ofVec3f c_sum = ofVec3f(0,0,0);
    
    int s_count = 0;
    int acc_count = 0;
    int c_count = 0;
    
    for(int i = 0; i < neighbors.size(); i++) {
        float d = positions[index].distance(neighbors[i].location);
        
        if(d > 0 && d < min_neighbor_dist) {
            
            //cohesion
            c_sum += neighbors[i].location;
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

ofVec3f Bird::forces(int index, std::vector<LatticeCube> neighbors) {
    float desired_separation = 20*2;
    float min_neighbor_dist = 40;
    
    ofVec3f s_sum = ofVec3f(0,0,0);
    ofVec3f acc_sum = ofVec3f(0,0,0);
    ofVec3f c_sum = ofVec3f(0,0,0);
    
    int s_count = 0;
    int acc_count = 0;
    int c_count = 0;
    
    for(int i = 0; i < neighbors.size(); i++) {
        float d = positions[index].distance(neighbors[i].location);
        
        //separation
        if(d > 0 && d < desired_separation) {
            ofVec3f diff = positions[index] - neighbors[i].location;
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