#include "Bird.h"
#include <math.h>

Bird::Bird() {}

void Bird::init(int amt) {
	resolution = 20;

	HRESULT hr;
	hr = GetDefaultKinectSensor(&sensor);
	if (FAILED(hr))
		exit;

	sensor->Open();

	IDepthFrameSource* dfs;
	IBodyIndexFrameSource* bifs;
	sensor->get_DepthFrameSource(&dfs);
	sensor->get_BodyIndexFrameSource(&bifs);
	sensor->get_CoordinateMapper(&cm);
	bifs->OpenReader(&bifr);
	dfs->OpenReader(&dfr);

	IFrameDescription *fd;
	dfs->get_FrameDescription(&fd);
	fd->get_Width(&depthWidth);
	fd->get_Height(&depthHeight);

	//dfr->SubscribeFrameArrived(DFrameArrived);
    
    amount = amt;
    boid_dist = 20;
    
    for(int i=0; i < amount; i++) {
        ofVec3f origin = ofVec3f(
            ofRandom(0, 1000),
            ofRandom(0, 800),
            ofRandom(-300, 300)
        );
        
        velocities.push_back(ofVec3f(ofRandom(-1, 1),ofRandom(-1, 1),ofRandom(-1, 1)));
        positions.push_back(origin);
		last_positions.push_back(origin);
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

	depthbuffer = new uint16_t[depthWidth * depthHeight];
	bibuffer = new byte[depthWidth * depthHeight];
	csp = new CameraSpacePoint[depthWidth * depthHeight];
    
    for(int i =0; i<resolution; i++) {
        lattice.push_back(std::vector<std::vector<std::vector<LatticeCube>>>());
        for(int j=0; j<resolution; j++) {
            lattice[i].push_back(std::vector<std::vector<LatticeCube>>());
            for(int k=0; k<resolution; k++) {
                lattice[i][j].push_back(std::vector<LatticeCube>());
            }
        }
    }
    
	//new starts here

	updatePos.load("nothing.vert", "posUpdate.frag");
	updateVel.load("nothing.vert", "velUpdate.frag");

	updateRender.setGeometryInputType(GL_POINTS);
	updateRender.setGeometryOutputType(GL_TRIANGLE_STRIP);
	updateRender.setGeometryOutputCount(6);
	updateRender.load("render.vert", "render.frag", "render.geom");

	textureRes = (int)sqrt((float)amount);
	amount = textureRes * textureRes;

	vector<float> pos(amount*3);
	for (int x = 0; x < textureRes; x++) {
		for (int y = 0; y < textureRes; y++) {
			int i = textureRes * y + x;

			pos[i * 3 + 0] = ofRandom(1000.0); //x*offset;
			pos[i * 3 + 1] = ofRandom(800.0); //y*offset;
			pos[i * 3 + 2] = ofRandom(-300.0, 300.0);
		}
	}

	// Load this information in to the FBO's texture
	posPingPong.allocate(textureRes, textureRes, GL_RGB32F);
	posPingPong.src->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);
	posPingPong.dst->getTexture().loadData(pos.data(), textureRes, textureRes, GL_RGB);

	vector<float> vel(amount * 3);
	for (int i = 0; i < amount; i++) {
		vel[i * 3 + 0] = ofRandom(-1.0, 1.0);
		vel[i * 3 + 1] = ofRandom(-1.0, 1.0);
		vel[i * 3 + 2] = ofRandom(-1.0, 1.0);
	}
	// Load this information in to the FBO's texture
	velPingPong.allocate(textureRes, textureRes, GL_RGB32F);
	velPingPong.src->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);
	velPingPong.dst->getTexture().loadData(vel.data(), textureRes, textureRes, GL_RGB);

	width = ofGetWindowWidth();
	height = ofGetWindowHeight();

	renderFBO.allocate(width, height, GL_RGB32F);
	renderFBO.begin();
	ofClear(0, 0, 0, 255);
	renderFBO.end();

	mesh.setMode(OF_PRIMITIVE_POINTS);
	for (int x = 0; x < textureRes; x++) {
		for (int y = 0; y < textureRes; y++) {
			mesh.addVertex(ofVec3f(x, y));
			mesh.addTexCoord(ofVec2f(x, y));
		}
	}

	texImage.load("spark.png");
	imageWidth = texImage.getWidth();
	imageHeight = texImage.getHeight();

}

void Bird::update() {
	people_size = 0;

	HRESULT hr;
	IDepthFrame* df;
	hr = dfr->AcquireLatestFrame(&df);
	if (SUCCEEDED(hr)) {
		df->CopyFrameDataToArray(depthWidth * depthHeight, depthbuffer);
		cm->MapDepthFrameToCameraSpace(depthWidth * depthHeight, depthbuffer, depthWidth*depthHeight, csp);
		df->Release();
		df = nullptr;
	}

	IBodyIndexFrame* bif;
	hr = bifr->AcquireLatestFrame(&bif);
	if (SUCCEEDED(hr)) {
		bif->CopyFrameDataToArray(depthWidth * depthHeight, bibuffer);
		bif->Release();
		bif = nullptr;

		people_points.clear();
		int count = 0;
		int mask_counter = 0;
		for (int i = 0; i < depthWidth * depthHeight; i++) {
			int mask_count = 0;

			if (depthWidth*depthHeight > i + depthWidth + 1 && i > depthWidth + 1)
			{
				if (bibuffer[i] != 0xff)
					mask_count += 1;
				if (bibuffer[i + 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i - 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i + depthWidth] != 0xff)
					mask_count += 1;
				if (bibuffer[i + depthWidth + 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i + depthWidth - 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i - depthWidth] != 0xff)
					mask_count += 1;
				if (bibuffer[i - depthWidth + 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i - depthWidth - 1] != 0xff)
					mask_count += 1;
			}
			if (bibuffer[i] != 0xff) {
				count += 1;
				if (mask_count > 0 && mask_count < 9)
					mask_counter += 1;
				if ((mask_count > 0 && mask_count < 9 && mask_counter%int(10 + i / (float(depthWidth)*float(depthHeight)) * 40) == 0) || count % int(100 + i / (float(depthWidth)*float(depthHeight)) * 400) == 0) {
					ofVec3f loc = mapIndexToOF(i);
					people_uniform[people_size*3 + 0] = loc.x;
					people_uniform[people_size*3 + 1] = loc.y;
					people_uniform[people_size*3 + 2] = loc.z;
					people_size += 1;
				}
			}
		}
	}


	velPingPong.dst->begin();
	ofClear(0);
	updateVel.begin();
	updateVel.setUniformTexture("velData", velPingPong.src->getTexture(), 0);   // passing the previus velocity information
	updateVel.setUniformTexture("posData", posPingPong.src->getTexture(), 1);  // passing the position information
	updateVel.setUniform1i("resolution", (int)textureRes);
	updateVel.setUniform3fv("people_points", people_uniform, 1000);
	updateVel.setUniform1i("people_size", people_size);

	// draw the source velocity texture to be updated
	velPingPong.src->draw(0, 0);

	updateVel.end();
	velPingPong.dst->end();

	velPingPong.swap();




	posPingPong.dst->begin();
	ofClear(0);
	updatePos.begin();
	updatePos.setUniformTexture("prevPosData", posPingPong.src->getTexture(), 0); // Previus position
	updatePos.setUniformTexture("velData", velPingPong.src->getTexture(), 1);  // Velocity

	// draw the source position texture to be updated
	posPingPong.src->draw(0, 0);

	updatePos.end();
	posPingPong.dst->end();

	posPingPong.swap();




	renderFBO.begin();
	ofClear(0, 0, 0, 0);
	updateRender.begin();
	updateRender.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0);
	updateRender.setUniformTexture("velTex", velPingPong.dst->getTexture(), 1);
    updateRender.setUniformTexture("sparkTex", texImage , 2);
    updateRender.setUniform1i("resolution", (float)textureRes); 
    updateRender.setUniform3f("screen", (float)width, (float)height, (float)400);
    updateRender.setUniform1f("size", (float)10); 
    updateRender.setUniform1f("imgWidth", (float)imageWidth);
    updateRender.setUniform1f("imgHeight", (float)imageHeight);
	updateRender.setUniform1f("time", (float)ofGetElapsedTimef());

	ofPushStyle();
	//ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofSetColor(255);

	mesh.draw();

	ofDisableBlendMode();
	glEnd();

	updateRender.end();
	renderFBO.end();
	ofPopStyle();
}

ofVec3f Bird::mapIndexToOF(int index) {
	return ofVec3f(-csp[index].X*400 + 500, -csp[index].Y*400 + 400, -csp[index].Z*100);
}

void Bird::draw() {
	ofSetColor(100, 255, 255);
	renderFBO.draw(0, 0);


	HRESULT hr;
	IDepthFrame* df;
	hr = dfr->AcquireLatestFrame(&df);
	if (SUCCEEDED(hr)) {
		df->CopyFrameDataToArray(depthWidth * depthHeight, depthbuffer);
		cm->MapDepthFrameToCameraSpace(depthWidth * depthHeight, depthbuffer, depthWidth*depthHeight, csp);
		df->Release();
		df = nullptr;
	}

	IBodyIndexFrame* bif;
	hr = bifr->AcquireLatestFrame(&bif);
	if (SUCCEEDED(hr)) {
		bif->CopyFrameDataToArray(depthWidth * depthHeight, bibuffer);
		bif->Release();
		bif = nullptr;

		people_points.clear();
		int count = 0;
		int mask_counter = 0;
		for (int i = 0; i < depthWidth * depthHeight; i++) {
			int mask_count = 0;

			if (depthWidth*depthHeight > i + depthWidth + 1 && i > depthWidth + 1)
			{
				if (bibuffer[i] != 0xff)
					mask_count += 1;
				if (bibuffer[i + 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i - 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i + depthWidth] != 0xff)
					mask_count += 1;
				if (bibuffer[i + depthWidth + 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i + depthWidth - 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i - depthWidth] != 0xff)
					mask_count += 1;
				if (bibuffer[i - depthWidth + 1] != 0xff)
					mask_count += 1;
				if (bibuffer[i - depthWidth - 1] != 0xff)
					mask_count += 1;
			}
			if (bibuffer[i] != 0xff) {
				count += 1;
				if (mask_count > 0 && mask_count < 9)
					mask_counter += 1;
				if ((mask_count > 0 && mask_count < 9 && mask_counter%int(10 + i / (float(depthWidth)*float(depthHeight)) * 40) == 0) || count % int(100 + i/(float(depthWidth)*float(depthHeight))*400) == 0) {
					people_points.push_back(mapIndexToOF(i));
				}
			}
		}
	}
    
    for(int i =0; i<resolution; i++) {
        for(int j=0; j<resolution; j++) {
            for(int k=0; k<resolution; k++) {
                lattice[i][j][k].clear();
            }
        }
    }
    
    LatticeCube lc = *new LatticeCube();
    lc.location = ofVec3f(0,0,0);
    lc.velocity = ofVec3f(0,0,0);

    for(int i = 0; i < amount; i++) {
        int x = positions[i].x / 1000 * resolution;
        int y = positions[i].y / 1000 * resolution;
        int z = -1*(positions[i].z - 300) / 600 * resolution;
        
        lc.location = positions[i];
        lc.velocity = velocities[i];
        if(x < resolution && y < resolution && z < resolution)
			lattice[x][y][z].push_back(lc);
    }

    for(int i = 0; i < amount;i++) {
		//positions[i] = primitive.getMesh().getVertex(i*6+3);
		velocities[i] = positions[i] - last_positions[i];
		last_positions[i] = positions[i];
  //      ofVec3f v1 = velocities[i];
  //      v1.normalize();
  //      
  //      
  //         
  //      if(ofRandom(0, 10) > 5) {
  //          int x = positions[i].x / 1000 * resolution;
  //          int y = positions[i].y / 1000 * resolution;
  //          int z = -1*(positions[i].z - 300) / 600 * resolution;
  //          
  //          std::vector<LatticeCube> neighbors;
  //          for(int j=x-1;j<=x+1;j+=2) {
  //              for(int k=y-1;k<=y+1;k+=2) {
  //                  for(int l=x-1;l<=x+1;l+=2) {
  //                      if(j > 0 && k > 0 && l > 0 && j < resolution-1 && k < resolution-1 && l < resolution-1) {
  //                          neighbors.insert(neighbors.end(), lattice[j][k][l].begin(), lattice[j][k][l].end());
  //                      }
  //                  }
  //              }
  //          }
  //          
  //          if(x > 0 && y > 0 && z > 0 && x < resolution-1 && y < resolution-1 && z < resolution-1)
  //              neighbors.insert(neighbors.end(), lattice[x][y][z].begin(), lattice[x][y][z].end());
  //          
  //          velocities[i] += forces(i, neighbors) * 1.1;
  //          velocities[i] += alignment(i, neighbors) * 1.4;
  //          velocities[i] += cohesion(i, neighbors) * 0.8;
  //      }
  //      
  //      velocities[i] += avoidWalls(i);
		//velocities[i] += BodySwarm(i) * 1.8;

  //      float mag = velocities[i].distance(ofVec3f(0,0,0));
  //      if(mag > 5) {
  //          velocities[i] /= (mag / 5);
  //      }
  //      
  //      ofVec3f v2 = velocities[i];
  //      v2.normalize();
  //      
  //      positions[i] += velocities[i];
  //      
  //      
  //      orth[i] = velocities[i].getPerpendicular(ofVec3f(0, positions[i].y, 0));
  //      orth[i].normalize();
  //      float angle = - ofDegToRad(ofVec3f(v1.x, 0, v1.y).angle(ofVec3f(v2.x, 0, v2.y)));

  //      
  //      /*ofMatrix3x3 mat = ofMatrix3x3(
  //              (1-cos(angle))*v2.x*v2.x + cos(angle), (1-cos(angle))*v2.x*v2.y - sin(angle)*v2.z, (1-cos(angle))*v2.x*v2.z + sin(angle)*v1.y,
  //              (1-cos(angle))*v2.x*v2.y + sin(angle)*v2.z, (1-cos(angle))*v2.y*v2.y + cos(angle), (1-cos(angle))*v2.y*v2.z - sin(angle)*v2.x,
  //              (1-cos(angle))*v2.x*v2.z - sin(angle)*v1.y, (1-cos(angle))*v2.y*v2.z + sin(angle)*v2.x, (1-cos(angle))*v2.z*v2.z + cos(angle));
  //      
  //      ofVec4f temp = ofMatrix4x4(
  //               mat.a, mat.b, mat.c, 0,
  //               mat.d, mat.e, mat.f, 0,
  //               mat.g, mat.h, mat.i, 0,
  //               0, 0, 0, 1).postMult(ofVec4f(orth[i].x, orth[i].y, orth[i].z, 0));
  //      
  //      
  //      orth[i] = ofVec3f(temp.x, temp.y, temp.z);*/
  //      
  //      
  //      
  //      ofVec3f t_vel = velocities[i];
  //      t_vel.normalize();
  //      
  //      ofVec3f wingd = velocities[i].getPerpendicular(orth[i]);
  //      wingd.normalize();
  //      
  //      primitive.getMesh().setVertex(i*6, positions[i] - orth[i]*10 + wingd*sin((ofGetElapsedTimef() + i/2)*6)*7);
  //      primitive.getMesh().setVertex(i*6 + 1, positions[i]);
  //      primitive.getMesh().setVertex(i*6 + 2, positions[i] + t_vel*10);
  //      primitive.getMesh().setVertex(i*6 + 3, positions[i]);
  //      primitive.getMesh().setVertex(i*6 + 4, positions[i] + t_vel*10);
  //      primitive.getMesh().setVertex(i*6 + 5, positions[i] + orth[i]*10 + wingd*sin((ofGetElapsedTimef() + i/2)*6)*7);
        
		allverts.positions[i] = positions[i];
		allverts2.positions[i] = velocities[i];
    }

 //   //material.begin();
 //   pointLight.enable();
 //   pointLight2.enable();
	//shader.begin();
	//shader.setUniformBuffer("AllVertices", allverts);
	//shader.setUniformBuffer("AllVertices2", allverts2);
 //   primitive.draw();
	//ofLog(OF_LOG_NOTICE) << primitive.getMesh().getVertex(0 * 6 + 3);
	//for(int i = 0; i < amount; i++)
	//	positions[i] = primitive.getMesh().getVertex(i*6+3);

	//shader.end();
 //   pointLight.disable();
 //   pointLight2.disable();
    //material.end();
}

ofVec3f Bird::BodySwarm(int index) {
	float min_neighbor_dist = 300;
	ofVec3f c_sum = ofVec3f(0, 0, 0);
	int c_count = 0;
	int max_dist = 100000000;
	if (ofRandom(0, 10) > 9) {
		for (int i = 0; i < people_points.size(); i++) {
			if (i > people_points.size() - 1)
				break;
			float d = positions[index].distance(people_points[i]);

			if (d > 0 && d < min_neighbor_dist && d < max_dist) {
				//c_sum += people_points[i];
				//c_count++;
				max_dist = d;
				c_sum = people_points[i];
				c_count = 1;
			}
		}
	}

	ofVec3f steer = ofVec3f(0, 0, 0);
	ofVec3f add = ofVec3f(0, 0, 0);

	if (c_count > 0) {
		c_sum /= c_count;
		ofVec3f seek = c_sum - positions[index];
		seek.normalize();
		seek *= 3;
		add = seek - velocities[index];
		add.limit(1.0);
		steer += add;
	}

	return steer;
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
    
    vec.set(positions[index].x, positions[index].y, -300);
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
    float min_neighbor_dist = 1024 / resolution;
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
    float min_neighbor_dist = 1024 / resolution;

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
    float min_neighbor_dist = 1024 / resolution - 10;
    
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