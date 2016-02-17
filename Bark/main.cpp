#include "main.h"
#include <stdio.h>
#include "PlatformFunctions.h"
#include "UtilMesh.h"
#include "DDSLoader.h"
#include "OBJLoader.h"
#include "DXTK\ScreenGrab.h"

static int STR_SIGMA_COUNT = 9;
static float STR_SIGMA_INIT = 0.1f;
static float STR_SIGMA_STEP = 0.2f;
static int STR_SIGMA = 0;

static int BASE_FREQ_COUNT = 2;
static float BASE_FREQ_INIT = 0.02f;
static float BASE_FREQ_STEP = 0.1f;
static int BASE_FREQ = 0;

static int INTER_LAYER_COUNT = 5;
static float INTER_LAYER_INIT = 0;
static float INTER_LAYER_STEP = 0.4f;
static int INTER_LAYER = 0;

static int INTRA_LAYER_COUNT = 5;
static float INTRA_LAYER_INIT = 0;
static float INTRA_LAYER_STEP = 0.4f;
static int INTRA_LAYER = 0;

static int DAMP_COUNT = 5;
static float DAMP_INIT = 0.1f;
static float DAMP_STEP = 1.0f;
static int DAMP = 0;

static int LAYERS_COUNT = 5;
static int LAYERS_INIT = 1;
static int LAYERS = 0;

static bool BOUND = false;

#define CYLINDER
#define THICKNESS 3

void Win32WriteFile(wchar_t *fileName, void *data, unsigned int size)
{
	HANDLE fileHandle = 0;
	fileHandle = CreateFileW(fileName, GENERIC_WRITE, NULL, NULL, CREATE_NEW,
							 FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		DeleteFileW(fileName);
		fileHandle = CreateFileW(fileName, GENERIC_WRITE, NULL, NULL, CREATE_NEW,
								 FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD writtenBytes;
		WriteFile(fileHandle, data, size, &writtenBytes, NULL);
		if (writtenBytes <= 0)
		{
		}
		CloseHandle(fileHandle);
	}
	else
	{
	}
}

void Init(Context *context)
{
	srand(0);

	FileData vertexShaderFile = Win32ReadFile(L"VertexShader.cso");
	FileData pixelShaderFile = Win32ReadFile(L"PixelShader.cso");
	VertexInputLayout inputLayout = {};
	inputLayout.inputCount = 2;
	char *names[] = { "POSITION" , "NORMAL"};
	InputType types[] = { FLOAT3, FLOAT3};
	inputLayout.names = names;
	inputLayout.components = types;
	Graphics::InitPipeline(&context->renderer, &context->pipeline, inputLayout, &vertexShaderFile, &pixelShaderFile);
	Win32ReleaseFile(vertexShaderFile);
	Win32ReleaseFile(pixelShaderFile);

	FileData cubeModelFile = Win32ReadFile(L"cube.obj");
	uint32 vertexCount, indexCount;
	OBJLoader::GetSizes(&cubeModelFile, &vertexCount, &indexCount);
	float *vertices = new float[vertexCount * 6];
	uint16 *indices = new uint16[indexCount];
	OBJLoader::Load(&cubeModelFile, vertices, indices, VertexAttributes::POSITION_NORMAL, 6, 0);

	Graphics::InitModel(&context->renderer, &context->cubeModel, vertices, vertexCount, 6 * sizeof(float), 
						indices, indexCount);
	Win32ReleaseFile(cubeModelFile);


	context->camera.azimuth = 0.0f;
	context->camera.distance = 150.0f;
	context->camera.polar = PI / 2;

	//context->bark.dampStr = 1.0f;
	//context->bark.strengthSigma = 1.0f;
	//context->bark.strengthBase = 0.1f;
	//context->bark.interLayerStiffness = 1.0f;
	//context->bark.intraLayerStiffness = 1.0f;
	//context->bark.boundedLayer = true;
	//context->bark.baseFreq = .0625f;

#ifdef PARAM_SPACE_TEST
	context->bark.dampStr = DAMP_INIT;
	context->bark.strengthSigma = STR_SIGMA_INIT;
	context->bark.strengthBase = 0.1f;
	context->bark.interLayerStiffness = INTER_LAYER_INIT;
	context->bark.intraLayerStiffness = INTRA_LAYER_INIT;
	context->bark.boundedLayer = BOUND;
	context->bark.baseFreq = BASE_FREQ_INIT;
#else
	context->bark.dampStr = 1.0f;
	context->bark.strengthSigma = 1.0f;
	context->bark.strengthBase = 0.1f;
	context->bark.interLayerStiffness = 1.0f;
	context->bark.intraLayerStiffness = 2.0f;
	context->bark.boundedLayer = true;
	context->bark.baseFreq = .0625f;
#endif

	uint32 cellXCount = 100;
	uint32 cellYCount = 90;
	uint32 cellZCount = THICKNESS;
	uint32 cellCount = cellXCount * cellYCount * cellZCount;
	FLBark::Init(&context->bark, cellXCount, cellYCount, cellZCount);

	uint32 connectionCount = cellXCount * cellYCount * cellZCount * 12 * 2;
	context->connections = new Vector3[connectionCount];
	Graphics::InitModel(&context->renderer, &context->connectionsModel, (float *)context->connections, 
						connectionCount, 3 * sizeof(float), NULL, 0, DYNAMIC);

	context->instanceModels = new ModelData[800];

	context->background = new Vector3[2 * 12 * cellXCount];
	Graphics::InitModel(&context->renderer, &context->bgModel, (float *)context->background,
						12 * cellXCount, 6 * sizeof(float), NULL, 0, DYNAMIC);

	Graphics::InitConstantBuffer(&context->renderer, &context->sceneBuffer, sizeof(Matrix4x4) * 2);
	Graphics::InitConstantBuffer(&context->renderer, &context->modelBuffer, sizeof(ModelData));
	Graphics::InitConstantBuffer(&context->renderer, &context->instanceBuffer, sizeof(ModelData) * 800);
	Graphics::InitConstantBuffer(&context->renderer, &context->controlBuffer, sizeof(uint32));

	VertexInputLayout texLayout = {};
	char *texNames[] = { "POSITION", "TEXCOORD" };
	InputType texSizes[] = { FLOAT3, FLOAT2 };
	texLayout.names = texNames;
	texLayout.components = texSizes;
	texLayout.inputCount = 2;
	texLayout.instanceCount = 0;

	FileData Graphics2DVS = Win32ReadFile(L"Graphics2DVS.cso");
	FileData Graphics2DPS = Win32ReadFile(L"Graphics2DPS.cso");
	Graphics::InitPipeline(&context->renderer, &context->graphics2Dpass, texLayout, &Graphics2DVS, &Graphics2DPS);
	Win32ReleaseFile(Graphics2DVS);
	Win32ReleaseFile(Graphics2DPS);

	FileData textPS = Win32ReadFile(L"TextPS.cso");
	FileData textVS = Win32ReadFile(L"Graphics2DVS.cso");
	Graphics::InitPipeline(&context->renderer, &context->textPass, texLayout, &textVS, &textPS);
	Win32ReleaseFile(textVS);
	Win32ReleaseFile(textPS);

	Mesh screenQuadMesh = {};
	UtilMesh::MakeScreenQuad(&screenQuadMesh);
	Graphics::InitModel(&context->renderer, &context->screenQuad, screenQuadMesh.vertices, screenQuadMesh.vertexCount,
						screenQuadMesh.vertexStride, screenQuadMesh.indices, screenQuadMesh.indexCount);
	UtilMesh::Release(&screenQuadMesh);

	Graphics2D::Init(&context->g2Dcontext, &context->renderer, &context->screenQuad, &context->graphics2Dpass,
					 &context->textPass);

	FileData fontFile = Win32ReadFile(L"font.dds");
	TextureDesc fontDesc = DDSLoader::Load(&fontFile);
	Graphics::InitTexture(&context->renderer, &context->g2Dcontext.fontTexture[0], fontDesc);
	Win32ReleaseFile(fontFile);

	FileData fontFile2 = Win32ReadFile(L"lane.dds");
	TextureDesc fontDesc2 = DDSLoader::Load(&fontFile2);
	Graphics::InitTexture(&context->renderer, &context->g2Dcontext.fontTexture[1], fontDesc2);
	Win32ReleaseFile(fontFile2);

	UISettings settings;
	settings.activeFont = 0;
	settings.colors[BGColor] = Vector4(1,1,1,0.2f);
	settings.colors[TextColorMain] = Vector4(0.2,0.2,0.2,1);
	settings.colors[TextColorSecondary] = Vector4(0,0,0,1);

	UI::InitContext(&context->uiContext, &context->g2Dcontext, &settings);
}

const float width = 100.0f;
//#define CYLINDER
Vector3 WallPosition(Vector3 position)
{
#ifdef CYLINDER
	Vector3 result, normal;
	normal = Math::Normalize(position);
	result = position - 0.7f * normal;
#else
	Vector3 planePosition;
	float d = Math::Dot(Math::Normalize(Vector3(position.x, 0, position.z)), Vector3(0, 0, -1)) * 0.5f + 0.5f;
	planePosition.x = (1 - d) * width * (position.x < 0 ? -1 : 1);
	planePosition.y = position.y;
	planePosition.z = 0;
	Vector3 result = planePosition;
#endif
	return result;
}

Texture tex;
bool texShown = false;
float time = 0;
#include <string>
void Update(Context *context, Input *input, float dt)
{
	time += dt;
#ifdef PARAM_SPACE_TEST
	if (time > 10.0f)
#else
	if (0)//time > 10.0f)
#endif
	{
		time = 0.0f;
		float damp = DAMP_INIT + DAMP_STEP * DAMP;
		float sigma = STR_SIGMA_INIT + STR_SIGMA_STEP * STR_SIGMA;
		float inter = INTER_LAYER_INIT + INTER_LAYER_STEP * INTER_LAYER;
		float intra = INTRA_LAYER_INIT + INTRA_LAYER_STEP * INTRA_LAYER;
		int bound = BOUND ? 1 : 0;
		float baseFreq = BASE_FREQ_INIT + BASE_FREQ_STEP * BASE_FREQ;
		std::string name = "Screens/damp" + std::to_string(damp) + "sigma" + std::to_string(sigma) + "inter" + std::to_string(inter) + 
			"intra" + std::to_string(intra) + "bound" + std::to_string(bound) + "freq" + std::to_string(baseFreq) + ".dds";
		std::wstring a;
		a.assign(name.begin(), name.end());
		DirectX::SaveDDSTextureToFile(context->renderer.context, context->screenTarget.resource, a.c_str());

		DAMP++;
		if (DAMP > DAMP_COUNT)
		{
			DAMP = 0;
			STR_SIGMA++;
			if (STR_SIGMA > STR_SIGMA_COUNT)
			{
				STR_SIGMA = 0;
				INTER_LAYER++;
				if (INTER_LAYER > INTER_LAYER_COUNT)
				{
					INTER_LAYER = 0;
					INTRA_LAYER++;
					if (INTRA_LAYER > INTRA_LAYER_COUNT)
					{
						INTRA_LAYER = 0;
						BASE_FREQ++;
						if (BASE_FREQ > BASE_FREQ_COUNT)
						{
							BASE_FREQ = 0;
							BOUND = true;
						}
					}
				}
			}
		}

		context->bark.dampStr = DAMP_INIT + DAMP_STEP * DAMP;
		context->bark.strengthSigma = STR_SIGMA_INIT + STR_SIGMA_STEP * STR_SIGMA;
		context->bark.strengthBase = 0.1f;
		context->bark.interLayerStiffness = INTER_LAYER_INIT + INTER_LAYER_STEP * INTER_LAYER;
		context->bark.intraLayerStiffness = INTRA_LAYER_INIT + INTRA_LAYER_STEP * INTRA_LAYER;
		context->bark.boundedLayer = BOUND;
		context->bark.baseFreq = BASE_FREQ_INIT + BASE_FREQ_STEP * BASE_FREQ;
		FLBark::Reinit(&context->bark);
	}
	float deltaLevel = input->mouseScroll / 120.0f;
	Camera *camera = &context->camera;
	camera->distance -= deltaLevel;
	Vector2 mouseDelta = input->touchPosition - input->previousPosition;
	if (input->leftButtonDown)
	{
		camera->azimuth -= mouseDelta.x / 100.0f;
		camera->polar -= mouseDelta.y / 100.0f;
	}

	Bark *bark = &context->bark;
	if (input->Wpressed)
	{
		time = 0;
		FLBark::Reinit(bark);
	}

	if (input->Spressed)
	{
		texShown = false;
		DirectX::SaveDDSTextureToFile(context->renderer.context, context->screenTarget.resource, L"bark.dds");
	}

	for (int i = 0; i < 3; ++i)
	{
		FLBark::Update(bark, Math::Min(dt, 0.02f));
	}
	Graphics::BindTarget(&context->renderer, &context->screenTarget);
	Graphics::ClearTarget(&context->renderer, &context->screenTarget);
	Graphics::BindPipeline(&context->renderer, &context->pipeline);
	Matrix4x4 uiProj = Math::GetOrthographicsProjectionLH(0.0f, context->renderer.screenSize.x,
														  0, context->renderer.screenSize.y, -100.0f, 100.0f);
#ifdef CYLINDER
	Matrix4x4 projection = Math::GetPerspectiveProjectionDXLH(45.0f, 800.0f / 600.0f, 0.01f, 1000.0f);
	float cameraX = Math::Sin(camera->polar) * Math::Sin(camera->azimuth) * camera->distance;
	float cameraY = Math::Cos(camera->polar) * camera->distance;
	float cameraZ = Math::Sin(camera->polar) * Math::Cos(camera->azimuth) * (-camera->distance);
	Matrix4x4 view = Math::LookAtLH(Vector3(cameraX, cameraY + bark->countY / 2.0f, cameraZ), 
									Vector3(0,bark->countY / 2.0f,0), Vector3(0,1,0));
#else
	float projHeight = 90.0f;
	float projWidth = projHeight * 800.0f / 600.0f;
	Matrix4x4 projection = Math::GetOrthographicsProjectionLH(-projWidth / 2.0f, projWidth / 2.0f, 
															  0, projHeight, 
															  -50, 50);
	Matrix4x4 view = Math::GetIdentity();
#endif
	Matrix4x4 mats[] = { projection, view };
	Graphics::BindConstantBuffer(&context->renderer, &context->sceneBuffer, mats, SCENE_SLOT);
	
	ModelData *modelData = context->instanceModels;
	uint32 modelCount = 0;
	Vector3 *connections = context->connections;
	uint32 connectionCount = 0;

	Vector3 *background = context->background;
	Vector3 *backgroundWrite = background;
	for (uint32 z = 0; z < bark->countZ; ++z)
	{
		float c = (z + 1) * (1.0f / bark->countZ);//1.0f - (bark->countZ - 1 - z) * 0.1f;
		Vector4 color(c,c,c,1);
		for (uint32 y = 0; y < bark->countY; ++y)
		{
			for (uint32 x = 0; x < bark->countX; ++x)
			{
				Cell *cell = FLBark::GetCell(bark, x, y, z);
				Vector3 position = cell->position;
#ifndef CYLINDER
				Vector3 planePosition;
				Vector3 xzPlanePos = Vector3(position.x, 0, position.z);
				float d = Math::Dot(Math::Normalize(xzPlanePos), Vector3(0, 0, -1));
				d = acosf(d) / PI;
				planePosition.x = (1 - d) * width * (position.x < 0 ? - 1 : 1);
				planePosition.y = position.y;
				planePosition.z = - (int)z - 0.5f;
				position = planePosition;
#endif
				bool drawElement = false;

				for (uint32 i = 0; i < 3; ++i)
				{
					if (cell->connectionsBottom[i] || z == 0)
					{
						drawElement = true;
					}
				}

				if (drawElement)
				{
					Matrix4x4 modelMat = Math::GetTranslation(position.x, position.y, position.z) *
#ifdef CYLINDER
						Math::GetScale(0.1f, 0.1f, 0.1f);
#else
						Math::GetScale(1.0f, 1.0f, 0.1f);
#endif
					modelData[modelCount].model = modelMat;
					modelData[modelCount++].color = color;


#ifdef CYLINDER
					for (uint32 i = 0; i < 6; ++i)
					{
						if (cell->connections[i])
						{
							++connectionCount;
							connections[connectionCount * 2] = position;
							connections[connectionCount * 2 + 1] = cell->connections[i]->position;
						}
					}


					for (uint32 i = 0; i < 3; ++i)
					{
						if (cell->connectionsBottom[i])
						{
							++connectionCount;
							connections[connectionCount * 2] = position;
							connections[connectionCount * 2 + 1] = cell->connectionsBottom[i]->position;
						}
					}

					for (uint32 i = 0; i < 3; ++i)
					{
						if (cell->connectionsTop[i])
						{
							++connectionCount;
							connections[connectionCount * 2] = position;
							connections[connectionCount * 2 + 1] = cell->connectionsTop[i]->position;
						}
					}
#endif
				}


				if (modelCount == 800)
				{
					uint32 instanceControl = 1;
					Graphics::BindConstantBuffer(&context->renderer, &context->controlBuffer, &instanceControl, CONTROL_SLOT);
					Graphics::BindConstantBuffer(&context->renderer, &context->instanceBuffer, modelData, INSTANCE_SLOT);

					uint32 cubeCount = modelCount;
					Graphics::RenderModelInstanced(&context->renderer, &context->cubeModel, cubeCount);
					modelCount -= 800;
				}

				if (z == 0 && y == 0)
				{
					Vector3 p1 = WallPosition(cell->position);
					Vector3 p2 = WallPosition(FLBark::GetCell(bark, x + 1, y, z)->position);
					Vector3 p3 = WallPosition(FLBark::GetCell(bark, x, bark->countY - 1, z)->position);
					Vector3 p4 = WallPosition(FLBark::GetCell(bark, x + 1, bark->countY - 1, z)->position);
					Vector3 average = (p1 + p2 + p3 + p4) / 4.0f;
					average.y = 0;
					Vector3 normal = Math::Normalize(average);
					Vector3 newQuad[] =
					{
						p1, normal, p3, normal, p2, normal,
						p4, normal, p2, normal, p3, normal,

						p1, normal, p2, normal, p3, normal,
						p4, normal, p3, normal, p2, normal
					};
					memcpy(backgroundWrite, newQuad, sizeof(newQuad));
					backgroundWrite += 24;
				}
			}
		}

		uint32 instanceControl = 0;
		Graphics::BindConstantBuffer(&context->renderer, &context->controlBuffer, &instanceControl, CONTROL_SLOT);

		Matrix4x4 model = Math::GetIdentity();
		ModelData modelDataConnections = { model, color};
		Graphics::BindConstantBuffer(&context->renderer, &context->modelBuffer, &modelDataConnections, MODEL_SLOT);
		Graphics::UpdateModelData(&context->renderer, &context->connectionsModel, connections,
								  2 * sizeof(Vector3) * connectionCount, 2 * connectionCount);
		Graphics::RenderModel(&context->renderer, &context->connectionsModel, LINES);
		connectionCount = 0;

		if (z == 0)
		{
			ModelData modelDataConnections = { model, Vector4(0,0,0,1)};
			Graphics::BindConstantBuffer(&context->renderer, &context->modelBuffer, &modelDataConnections, MODEL_SLOT);
			Graphics::UpdateModelData(&context->renderer, &context->bgModel, background,
									  sizeof(Vector3) * 24 * bark->countX, bark->countX * 12);
			Graphics::RenderModel(&context->renderer, &context->bgModel);
		}
	}

	uint32 instanceControl = 1;
	Graphics::BindConstantBuffer(&context->renderer, &context->controlBuffer, &instanceControl, CONTROL_SLOT);
	Graphics::BindConstantBuffer(&context->renderer, &context->instanceBuffer, modelData, INSTANCE_SLOT);

	uint32 cubeCount = modelCount;
	Graphics::RenderModelInstanced(&context->renderer, &context->cubeModel, cubeCount);

	
	Graphics::BindTargetDepthless(&context->renderer, &context->screenTarget);
	Matrix4x4 matsUI[] = { uiProj, Math::GetIdentity() };
	Graphics::BindConstantBuffer(&context->renderer, &context->sceneBuffer, matsUI, SCENE_SLOT);

	if (input->Bpressed)
	{
#ifndef CYLINDER
		texShown = true;
		uint32 *screenBuffer = new uint32[800 * 600];
		for (uint32 y = 0; y < 600; ++y)
		{
			for (uint32 x = 0; x < 800; ++x)
			{
				screenBuffer[y * 800 + x] = 0xFF000000;
			}
		}
		for (uint32 z = 0; z < bark->countZ; ++z)
		{
			uint32 level = (uint32)(255 * (z / (float)bark->countZ));
			uint32 color =  (0xAA << 24) | (level << 16) | (level << 8) | level;
			for (uint32 y = 0; y < bark->countY; ++y)
			{
				for (uint32 x = 0; x < bark->countX; ++x)
				{
					Cell *cell = FLBark::GetCell(bark, x, y, z);
					Vector3 position = cell->position;
					Vector3 planePosition;
					Vector3 xzPlanePos = Vector3(position.x, 0, position.z);
					float d = Math::Dot(Math::Normalize(xzPlanePos), Vector3(0, 0, -1));
					d = acosf(d) / PI;
					planePosition.x = (1 - d) * width * (position.x < 0 ? -1 : 1);
					planePosition.y = position.y;
					position = planePosition;
					position.y = position.y * 600 / projHeight;
					position.x = position.x + projWidth / 2.0f;
					position.x = position.x * 800 / (projWidth);
					position.y = 600 - position.y;
					float resizedHeight = 600.0f + 10 * 600.0f / 90.0f * 2.0f;
					position.y = position.y * resizedHeight / 600.0f - 10 * 600.0f / 90.0f;
					int32 size = 600 / 90.0f;
					for (int32 h = -size / 2; h <= size / 2; ++h)
					{
						for (int32 w = -size / 2; w <= size / 2; ++w)
						{
							Vector2 pos = Vector2(position.x + w, position.y + h);
							int32 posX = pos.x;
							int32 posY = pos.y;
							if(posX < 0 || posX >= 800 || posY < 0 || posY >= 600)
								continue;
							screenBuffer[posY * 800 + posX] = color;
						}
					}
				}
			}
		}

		TextureDesc texDesc;
		texDesc.data = screenBuffer;
		texDesc.mipCount = 1;
		texDesc.height = 600;
		texDesc.width = 800;

		Graphics::InitTexture(&context->renderer, &tex, texDesc);

		Win32WriteFile(L"bark", screenBuffer, sizeof(uint32) * 800 * 600);
#endif
	}

	Graphics2D::ClearRender(&context->g2Dcontext);
	context->uiContext.input = input;
	Layout layout;
	UI::InitLayout(&layout, Vector2(20, 20));
	UI::DrawLabelInt(&context->uiContext, &layout, "STEP", 1);
	context->bark.strengthSigma = UI::DrawSlider(&context->uiContext, &layout, "STR SIGMA", 
												 context->bark.strengthSigma, 0, 0.0f, 5.0f);
	context->bark.strengthBase = UI::DrawSlider(&context->uiContext, &layout, "STR BASE",
												 context->bark.strengthBase, 4, 0.0f, 5.0f);
	context->bark.baseFreq = UI::DrawSlider(&context->uiContext, &layout, "BASE FREQ",
												context->bark.baseFreq, 5, 0.0f, .5f);
	context->bark.interLayerStiffness = UI::DrawSlider(&context->uiContext, &layout, "INTER LAYER",
												 context->bark.interLayerStiffness, 1, 0.0f, 5.0f);
	context->bark.intraLayerStiffness = UI::DrawSlider(&context->uiContext, &layout, "INTRA LAYER",
												 context->bark.intraLayerStiffness, 2, 0.0f, 5.0f);
	context->bark.dampStr = UI::DrawSlider(&context->uiContext, &layout, "DAMP",
													   context->bark.dampStr, 6, 0.0f, 5.0f);
	if (UI::DrawToggle(&context->uiContext, &layout, "BOUND", bark->boundedLayer, 3))
	{
		bark->boundedLayer = !bark->boundedLayer;
	}
	Graphics2D::CommitRender(&context->g2Dcontext);

	if (texShown)
	{
		Graphics::BindPipeline(&context->renderer, &context->graphics2Dpass);
		Graphics::BindTexture(&context->renderer, &tex);
		Matrix4x4 sceneData[] = {Math::GetIdentity(), Math::GetIdentity()};
		Graphics::BindConstantBuffer(&context->renderer, &context->sceneBuffer, sceneData, 0);
		PerModelData modelData;
		modelData.modelMatrix = Math::GetIdentity();
		modelData.instancingOn = false;
		modelData.instanceBaseOffset = 0;
		Graphics::BindConstantBuffer(&context->renderer, &context->g2Dcontext.perModelbuffer, &modelData, 1);
		Vector4 color = Vector4(1,1,1,1);
		Graphics::BindConstantBuffer(&context->renderer, &context->g2Dcontext.colorBuffer, &color, 12);
		Vector4 sourceRect = Vector4(0,0,1,1);
		Graphics::BindConstantBuffer(&context->renderer, &context->g2Dcontext.spriteBuffer, &sourceRect, 13);
		Graphics::RenderModel(&context->renderer, &context->screenQuad);
	}

}

void Release(Context *context)
{
	Graphics::Release(&context->connectionsModel);
	Graphics::Release(&context->cubeModel);
	Graphics::Release(&context->sceneBuffer);
	Graphics::Release(&context->modelBuffer);
	Graphics::Release(&context->controlBuffer);
	Graphics::Release(&context->instanceBuffer);
	Graphics::Release(&context->pipeline);
}