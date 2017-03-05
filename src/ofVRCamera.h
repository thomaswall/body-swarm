#pragma once

#include "Kinect.h"
#include "Kinect.Face.h"
#include "ofEasyCam.h"

class ofVRCamera
{
public:
	ofVRCamera(IKinectSensor* kinect);

	void begin();
	void update();
	void end();
	void zeroOutCamera();

	float scale = 500;

	~ofVRCamera();
private:
	IKinectSensor* kinect;

	ofEasyCam camera;

	IBodyFrameReader* bodyFrameReader;
	IBody* bodies;

	IHighDefinitionFaceFrameReader* faceFrameReader;

	IHighDefinitionFaceFrameSource* faceFrameSources[BODY_COUNT];
	IHighDefinitionFaceFrameReader* faceFrameReaders[BODY_COUNT];

	CameraSpacePoint* InitialNose = nullptr;
	CameraSpacePoint diff;

};

