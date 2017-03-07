#include "Bird.h"
#include <math.h>

Bird::Bird(IKinectSensor* _sensor) {
	this->sensor = _sensor;
	camera = new ofVRCamera(sensor);

	background.load("background.jpg");
	//vp.setPixelFormat(OF_PIXELS_NATIVE);
	//vp.load("movie2.avi");
	//vp.play();

	plane1.set(1000, 2200);
	plane1.setPosition(0, 500, -500);

	box.set(2700, 2200, 7000);
	box.setResolution(100, 100, 100);
	box.setPosition(500, 500, -1000);
}

void Bird::init(int amt) {
	resolution = 20;

	GLfloat lmodel_ambient[] = { 1.0, 0.2, 0.2, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	HRESULT hr;
	
	/*hr = GetDefaultKinectSensor(&sensor);
	if (FAILED(hr))
		exit;

	sensor->Open();
	*/

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
    
    //pointLight.setPosition(0, 500, -200);
    //pointLight.setPointLight();
    //pointLight2.setPosition(1000, 500, -200);
    //pointLight2.setPointLight();
	pointLight.enable();
    
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

	mesh.setupIndicesAuto();

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

	camera->update();
	//vp.update();

	renderFBO.begin();
	camera->begin();
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
	//ofDisableBlendMode();
	glEnd();

	updateRender.end();

	//vp.writeToTexture(vp_mesh);
	//vp_mesh.draw(-800, -700, -1000, 2700, 2200);
	//vp.draw(0, 0, 10, 20);

	/*ofPushMatrix();
	ofRotateY(90);
	plane1.drawWireframe();
	ofPopMatrix();*/
	box.drawWireframe();

	camera->end();
	background.draw(-900, -800, -4000, 3000, 2500);
	renderFBO.end();
	ofPopStyle();
}

ofVec3f Bird::mapIndexToOF(int index) {
	return ofVec3f(-csp[index].X*400 + 500, -csp[index].Y*400 + 400, -csp[index].Z*100);
}

void Bird::draw() {
	ofSetColor(255, 255, 255);
	renderFBO.draw(0, 0);
}