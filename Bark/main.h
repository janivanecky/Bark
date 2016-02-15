#pragma once
#include "Bark.h"
#include "Graphics.h"
#include "Core\Math.h"
#include "Graphics2D.h"
#include "UI.h"

struct Input
{
	float mouseScroll;
	bool leftButtonDown;
	bool leftButtonPressed;
	bool rightButtonDown;
	bool Wpressed;
	bool Spressed;
	bool Bpressed;
	Vector2 touchPosition;
	Vector2 previousPosition;
};

struct Camera
{
	float azimuth;
	float polar;
	float distance;
};

struct ModelData
{
	Matrix4x4 model;
	Vector4 color;
};

struct Context
{
	Model screenQuad;
	Pipeline graphics2Dpass;
	Pipeline textPass;

	Graphics2DContext g2Dcontext;
	UIContext uiContext;

	Renderer renderer;
	RenderTarget screenTarget;
	Pipeline pipeline;
	Model cubeModel;

	Vector3 *connections;
	Vector3 *background;

	ModelData *instanceModels;

	Model connectionsModel;
	Model bgModel;

	ConstantBuffer controlBuffer;
	ConstantBuffer instanceBuffer;
	ConstantBuffer sceneBuffer;
	ConstantBuffer modelBuffer;

	Camera camera;
	Bark bark;
};


void Init(Context *context);
void Update(Context *context, Input *input, float dt);
void Release(Context *context);