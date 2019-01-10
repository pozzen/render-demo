#include "main.h"
#include "camera.h"
#include "texture.h"
#include "shader.h"
#include "model.h"
#include "text.h"

// Constants
const char *title = "Render Demo";
const int width = 1920;
const int height = 1080;
const bool fullScreen = true;

const int MAX_LIGHT_COUNT = 4096;
const int GROUP_X = 16;
const int GROUP_Y = 16;

const float CAMERA_Z_NEAR = 0.01;
const float CAMERA_Z_FAR = 50.0;

const vec3 CLEAR_COLOR = vec3(0.11, 0.36, 0.67);
const int LIGHT_SEED = 1;

// Lists
enum OutputMode
{
	OUTPUT_RENDERED = 0,
	OUTPUT_DEPTHMAP,
	OUTPUT_LIGHT_HEATMAP,
	OUTPUT_GBUFFER,

	OUTPUT_MODE_MAX
};

static const char *OutputModeStr[] = {
	"Rendered Scene",
	"Depth Map",
	"Light Heatmap",
	"GBuffer"
};

enum Technique
{
	TECHNIQUE_DEFERRED_TILED = 0,
	TECHNIQUE_FORWARD_PLUS,
	TECHNIQUE_DEFERRED,
	TECHNIQUE_FORWARD,

	TECHNIQUE_MAX
};

static const char *TechniqueStr[] = {
	"Tiled Deferred",
	"Forward+",
	"Deferred",
	"Forward"
};

static const char *ModelStr[] = {
	"Models/sibenik/sibenik.obj",
	"Models/dabrovic-sponza/sponza.obj"
};

// Types
struct Light
{
	vec4 positionRadius;
	vec4 colorSpec;
};

// Variables
OutputMode outputMode = OUTPUT_RENDERED;
Technique technique = TECHNIQUE_DEFERRED_TILED;
int lightCount = 256;
int curModel = 0;
bool showHelp = false;
bool showHUD = true;
bool moveLights = true;
bool lightSpheres = false;

// Internal variables
SDL_Window *window = nullptr;
SDL_GLContext glContext;

double curTime = 0.0;
double deltaTime = 0.0;
bool quitting = false;
bool mouseDown = false;
float framerate = 0.0;

Camera camera;

Shader colorShader;
Shader depthShader;
Shader lightCullShader;
Shader forwardShader;
Shader forwardPlusShader;
Shader deferredGBufferShader;
Shader deferredShader;
Shader deferredTiledShader;
Shader screenTextureShader;
Shader screenDepthShader;
Shader screenLightHeatmapShader;
Shader screenAbsTextureShader;

vector<Model> models;
Mesh screenQuad;
Model model;
Model sphere;

GLuint lightBuffer = 0;
GLuint visibleLightBuffer = 0;

GLuint depthFBO;
GLuint depthTexture;

GLuint gFBO;
GLuint gRBO;
GLuint gPositionTex;
GLuint gNormalTex;
GLuint gColSpecTex;

Light lights[MAX_LIGHT_COUNT];
vec3 lightTargets[MAX_LIGHT_COUNT];

float getRand(float min, float max)
{
	float range = max-min;
	return (rand()/(float)INT_MAX * range + min);
}

void updateLights()
{
	if (!moveLights && visibleLightBuffer) return;

	// Update light positions
	for (unsigned i = 0; i < lightCount; i++)
	{
		lights[i].positionRadius += vec4(normalize(lightTargets[i] - vec3(lights[i].positionRadius))
									* float(deltaTime) * float(i%1024) / 100.0f, 0.0);

		if (glm::distance2(vec3(lights[i].positionRadius), lightTargets[i]) < 0.1)
		{
			lightTargets[i] = vec3(getRand(model.bb.min.x, model.bb.max.x),
								   getRand(model.bb.min.y, model.bb.max.y),
								   getRand(model.bb.min.z, model.bb.max.z));
		}
	}

	if (!visibleLightBuffer)
	{
		// Light and light index buffers, used in lightCullShader and lightShader
		glGenBuffers(1, &lightBuffer);
		glGenBuffers(1, &visibleLightBuffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, ((width+15)/16) * ((width+15)/16) * 1024 * sizeof(GLuint), 0, GL_DYNAMIC_COPY);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, visibleLightBuffer);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHT_COUNT * sizeof(Light), &lights[0], GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void generateLights()
{
	srand(LIGHT_SEED);

	// Generate the maximum number of allowed lights,
	// even if we don't render all of them
	vec3 dims = model.bb.max - model.bb.min;
	float maxDim = glm::max(glm::max(dims.x, dims.y), dims.z);

	for (int i = 0; i < MAX_LIGHT_COUNT; i++)
	{
		lights[i].positionRadius = vec4(getRand(model.bb.min.x, model.bb.max.x),
										getRand(model.bb.min.y, model.bb.max.y),
										getRand(model.bb.min.z, model.bb.max.z),
										getRand(maxDim/50.0f, maxDim/5.0f));
		lights[i].colorSpec = vec4(getRand(0, 1),
								   getRand(0, 1),
								   getRand(0, 1),
								   getRand(0, 1));
		lightTargets[i] = vec3(getRand(model.bb.min.x, model.bb.max.x),
							   getRand(model.bb.min.y, model.bb.max.y),
							   getRand(model.bb.min.z, model.bb.max.z));
	}
}

void initialize()
{
	camera = Camera(60, width/(float)height, CAMERA_Z_NEAR, CAMERA_Z_FAR);
	initFont(width, height);


	// Create a fullscreen quad
	screenQuad = createMesh(
		{{vec3(-1.0,  1.0, 0.0), vec2(0.0, 1.0)},
		 {vec3(-1.0, -1.0, 0.0), vec2(0.0, 0.0)},
		 {vec3( 1.0, -1.0, 0.0), vec2(1.0, 0.0)},
		 {vec3( 1.0,  1.0, 0.0), vec2(1.0, 1.0)}},
		{0, 1, 2, 0, 2, 3});


	// Load models
	for (unsigned int i = 0, l = sizeof(ModelStr) / sizeof(ModelStr[0]); i < l; i++)
	{
		models.push_back(loadModel(ModelStr[i]));
	}
	model = models[0];
	sphere = loadModel("Models/Sphere.nff");


	// Generate lights
	generateLights();
	updateLights();


	// Load shaders
	// colorShader: Renders a model in a single color
	colorShader = getShader("Shaders/color");

	// depthShader: Renders scene to depth buffer
	depthShader = getShader("Shaders/depth");

	lightCullShader = getShader("Shaders/lightCull");
	glUseProgram(lightCullShader.program);
	lightCullShader.setUniform("projection", camera.projection);
	lightCullShader.setUniform("screenSize", vec2(width, height));
	lightCullShader.setUniform("zNear", CAMERA_Z_NEAR);
	lightCullShader.setUniform("zFar", CAMERA_Z_FAR);
	lightCullShader.setUniform("depthMap", 0);

	forwardShader = getShader("Shaders/forward");

	forwardPlusShader = getShader("Shaders/forwardPlus");
	glUseProgram(forwardPlusShader.program);
	forwardPlusShader.setUniform("tilesX", ((width+15)/16));
	forwardPlusShader.setUniform("screenSize", vec2(width, height));

	deferredGBufferShader = getShader("Shaders/deferredGBuffer");

	deferredShader = getShader("Shaders/deferred");
	glUseProgram(deferredShader.program);
	deferredShader.setUniform("gPosition", 0);
	deferredShader.setUniform("gNormal", 1);
	deferredShader.setUniform("gAlbedoSpec", 2);
	deferredShader.setUniform("depthMap", 3);

	deferredTiledShader = getShader("Shaders/deferredTiled");
	glUseProgram(deferredTiledShader.program);
	deferredTiledShader.setUniform("gPosition", 0);
	deferredTiledShader.setUniform("gNormal", 1);
	deferredTiledShader.setUniform("gAlbedoSpec", 2);
	deferredTiledShader.setUniform("depthMap", 3);
	deferredTiledShader.setUniform("tilesX", ((width+15)/16));
	deferredTiledShader.setUniform("screenSize", vec2(width, height));

	// screenTextureShader: Renders a texture to screen
	screenTextureShader = getShader("Shaders/screenTexture");
	glUseProgram(screenTextureShader.program);
	screenTextureShader.setUniform("screenTexture", 0);
	glUseProgram(0);

	// screenDepthShader: Renders depthmap to screen
	screenDepthShader = getShader("Shaders/screenDepthmap");
	glUseProgram(screenDepthShader.program);
	screenDepthShader.setUniform("depthMap", 0);
	screenDepthShader.setUniform("zNear", CAMERA_Z_NEAR);
	screenDepthShader.setUniform("zFar", CAMERA_Z_FAR);

	screenLightHeatmapShader = getShader("Shaders/screenLightHeatmap");
	glUseProgram(screenLightHeatmapShader.program);
	screenLightHeatmapShader.setUniform("tilesX", ((width+15)/16));

	// screenAbsTextureShader: Renders scaled, absolute pixel
	// colors from a texture to screen
	screenAbsTextureShader = getShader("Shaders/screenAbsTexture");
	glUseProgram(screenAbsTextureShader.program);
	screenAbsTextureShader.setUniform("screenTexture", 0);
	glUseProgram(0);


	// Framebuffer for depth map
	glGenFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

	glGenTextures(1, &depthTexture);

	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cerr<<"Error creating GBuffer"<<endl;
	}


	// GBuffer for deferred shading
	glGenFramebuffers(1, &gFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gFBO);

	//	Position buffer
	glGenTextures(1, &gPositionTex);
	glBindTexture(GL_TEXTURE_2D, gPositionTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionTex, 0);

	//	Normal buffer
	glGenTextures(1, &gNormalTex);
	glBindTexture(GL_TEXTURE_2D, gNormalTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTex, 0);

	//	Color + Specular buffer
	glGenTextures(1, &gColSpecTex);
	glBindTexture(GL_TEXTURE_2D, gColSpecTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColSpecTex, 0);

	// Depth buffer
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cerr<<"Error creating GBuffer"<<endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void deinitialize()
{
	glDeleteBuffers(1, &lightBuffer);
	glDeleteBuffers(1, &visibleLightBuffer);

	glDeleteFramebuffers(1, &depthFBO);
	glDeleteTextures(1, &depthTexture);

	glDeleteFramebuffers(1, &gFBO);
	glDeleteRenderbuffers(1, &gRBO);
	glDeleteTextures(1, &gPositionTex);
	glDeleteTextures(1, &gNormalTex);
	glDeleteTextures(1, &gColSpecTex);
}

void renderGeometry(Shader &shader)
{
	glUseProgram(shader.program);
	shader.setUniform("lightCount", lightCount);
	shader.setUniform("viewProjection", camera.getViewProjection());
	shader.setUniform("cameraPosition", camera.position);

	for (unsigned int i = 0; i < model.meshes.size(); i++)
	{
		bindMaterial(model.meshes[i].material, shader);

		glBindVertexArray(model.meshes[i].vao);
		shader.setUniform("model", model.meshes[i].transform);
		glDrawElements(GL_TRIANGLES, model.meshes[i].elements, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
}

void renderLightsDebug()
{
	glUseProgram(colorShader.program);
	colorShader.setUniform("viewProjection", camera.getViewProjection());

	for (unsigned int i = 0; i < lightCount; i++)
	{
		colorShader.setUniform("ambient", vec3(lights[i].colorSpec));
		colorShader.setUniform("model", glm::translate(vec3(lights[i].positionRadius))
									  * glm::scale(vec3(0.05 * lights[i].positionRadius.w)));

		glBindVertexArray(sphere.meshes[0].vao);
		glDrawElements(GL_TRIANGLES, sphere.meshes[0].elements, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
}

void renderHUD()
{
	if (!showHUD) return;

	glViewport(0, 0, width, height);

	beginRenderText();

	char status[1024];
	snprintf(status, 1023, "Framerate: %.2f - Light count: %d - Output mode: %s - Technique: %s",
			 framerate,
			 lightCount,
			 OutputModeStr[outputMode],
			 TechniqueStr[technique]);
	drawText(status, vec2(5, 5));

	if (!showHelp)
	{
		drawText("F1   Toggle help", vec2(5, height - 5), 1.0, ANCHOR_BOTTOM);
	}
	else
	{
		drawText("F1\n"
				 "F2\n"
				 "F3\n"
				 "F4\n"
				 "F5\n"
				 "F6\n"
				 "F7\n"
				 "+\n"
				 "-\n"
				 "W S A D / arrow keys\n"
				 "Left mouse button", vec2(5, height - 5), 1.0, ANCHOR_BOTTOM);

		drawText("Toggle help\n"
				 "Toggle HUD\n"
				 "Change output mode\n"
				 "Change rendering technique\n"
				 "Change model\n"
				 "Toggle moving lights\n"
				 "Toggle light spheres\n"
				 "Add 64 lights\n"
				 "Remove 64 lights\n"
				 "Navigate\n"
				 "Look", vec2(300, height - 5), 1.0, ANCHOR_BOTTOM);
	}

	endRenderText();
}

void renderScene()
{
	updateLights();

	bool needsGBuffer = (technique == TECHNIQUE_DEFERRED_TILED || technique == TECHNIQUE_DEFERRED);
	bool needsLightCulling = (technique == TECHNIQUE_FORWARD_PLUS || technique == TECHNIQUE_DEFERRED_TILED);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Update stuff
	camera.update(deltaTime);
	glViewport(0, 0, width, height);

	if (technique == TECHNIQUE_FORWARD_PLUS || outputMode == OUTPUT_DEPTHMAP)
	{
		// Depth step
		// This produces a depthmap which we can use to clip by
		// depth in the light culling step
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

		glClear(GL_DEPTH_BUFFER_BIT);
		renderGeometry(depthShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	else if (needsGBuffer)
	{
		// Render to GBuffer
		// This also renders to our depth texture
		glBindFramebuffer(GL_FRAMEBUFFER, gFBO);

		glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, 1.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		renderGeometry(deferredGBufferShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	if (outputMode == OUTPUT_DEPTHMAP)
	{
		// Render depth map to screen
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);

		glUseProgram(screenDepthShader.program);

		glBindVertexArray(screenQuad.vao);
		glDrawElements(GL_TRIANGLES, screenQuad.elements, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(0);
	}
	else if (technique == TECHNIQUE_FORWARD)
	{
		// Render scene with no light culling, ie Forward Rendering
		glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, 1.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		renderGeometry(forwardShader);
	}
	else
	{
		if (needsLightCulling)
		{
			// Light culling step
			// For every light in lightBuffer, this shader checks
			// whether it's visible in each 16x16 square of the
			// screen by dividing the camera frustum and testing
			// if the light is inside. Indices of lights which
			// pass this test are placed in visibleLightBuffer
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthTexture);

			glUseProgram(lightCullShader.program);
			lightCullShader.setUniform("lightCount", lightCount);
			lightCullShader.setUniform("view", camera.getView());
			glDispatchCompute(((width+15)/16), ((height+15)/16), 1);
			glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(0);
		}

		if (outputMode == OUTPUT_LIGHT_HEATMAP)
		{
			// Render light heat map to screen
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			glUseProgram(screenLightHeatmapShader.program);
			screenLightHeatmapShader.setUniform("lightCount", lightCount);

			glBindVertexArray(screenQuad.vao);
			glDrawElements(GL_TRIANGLES, screenQuad.elements, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(0);
		}
		else if (technique == TECHNIQUE_FORWARD_PLUS)
		{
			// Render scene with light culling
			// This shader uses the output of the culling shader
			// to determine which lights are visible on each
			// 16x16 square of the screen
			glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, 1.0);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			renderGeometry(forwardPlusShader);
		}
		else // Deferred
		{
			// Render to screen using data from the GBuffer
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			if (outputMode == OUTPUT_GBUFFER)
			{
				// Render the 3 GBuffer textures side by side,
				// along with the final result
				glUseProgram(screenAbsTextureShader.program);
				glBindVertexArray(screenQuad.vao);
				glActiveTexture(GL_TEXTURE0);

				// Position texture
				vec3 dims = model.bb.max - model.bb.min;
				screenAbsTextureShader.setUniform("scale", 1.0f / glm::min(glm::min(dims.x, dims.y), dims.z));
				glViewport(0, height/2, width/2, height/2);
				glBindTexture(GL_TEXTURE_2D, gPositionTex);
				glDrawElements(GL_TRIANGLES, screenQuad.elements, GL_UNSIGNED_INT, 0);

				// Normal texture
				screenAbsTextureShader.setUniform("scale", 1.0f);
				glViewport(width/2, height/2, width/2, height/2);
				glBindTexture(GL_TEXTURE_2D, gNormalTex);
				glDrawElements(GL_TRIANGLES, screenQuad.elements, GL_UNSIGNED_INT, 0);

				// Albedo texture
				glUseProgram(screenTextureShader.program);
				glViewport(0, 0, width/2, height/2);
				glBindTexture(GL_TEXTURE_2D, gColSpecTex);
				glDrawElements(GL_TRIANGLES, screenQuad.elements, GL_UNSIGNED_INT, 0);

				glViewport(width/2, 0, width/2, height/2);
			}

			if (technique == TECHNIQUE_DEFERRED_TILED)
			{
				// Render to screen using the deferred shader with light culling
				glUseProgram(deferredTiledShader.program);
				deferredTiledShader.setUniform("lightCount", lightCount);
				deferredTiledShader.setUniform("cameraPosition", camera.position);
			}
			else if (technique == TECHNIQUE_DEFERRED)
			{
				// Render to screen without light culling
				glUseProgram(deferredShader.program);
				deferredShader.setUniform("lightCount", lightCount);
				deferredShader.setUniform("cameraPosition", camera.position);
			}

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPositionTex);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormalTex);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gColSpecTex);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, depthTexture);

			if (outputMode != OUTPUT_GBUFFER) glBindVertexArray(screenQuad.vao);
			glDrawElements(GL_TRIANGLES, screenQuad.elements, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	if (lightSpheres)
	{
		if (technique != TECHNIQUE_FORWARD)
		{
			// Blit depth buffer to default framebuffer
			glBindFramebuffer(GL_READ_FRAMEBUFFER, depthFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			if (outputMode == OUTPUT_GBUFFER)
				glBlitFramebuffer(0,		0,	width,	height,
								  width/2,	0,	width,	height/2,
								  GL_DEPTH_BUFFER_BIT, GL_NEAREST);
			else
				glBlitFramebuffer(0, 0, width, height,
								  0, 0, width, height,
								  GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}
		renderLightsDebug();
	}

	// Clean up
	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Render HUD
	renderHUD();
}

int SDLCALL handleInput(void *userdata, SDL_Event* event)
{
	if (event->type == SDL_APP_WILLENTERBACKGROUND)
	{
		quitting = true;
		return 1;
	}
	else if (event->type == SDL_KEYDOWN)
	{
		switch (event->key.keysym.sym)
		{
		case SDLK_ESCAPE:
			quitting = true;
			break;

		case SDLK_w:
		case SDLK_UP:
			camera.moveForward = true;
			break;

		case SDLK_s:
		case SDLK_DOWN:
			camera.moveBackward = true;
			break;

		case SDLK_a:
		case SDLK_LEFT:
			camera.moveLeft = true;
			break;

		case SDLK_d:
		case SDLK_RIGHT:
			camera.moveRight = true;
			break;

		case SDLK_PLUS:
		case SDLK_EQUALS:
			lightCount = glm::min(lightCount + 64, MAX_LIGHT_COUNT);
			break;

		case SDLK_MINUS:
			lightCount = glm::max(lightCount - 64, 0);
			break;

		case SDLK_F1:
			showHelp = !showHelp;
			break;

		case SDLK_F2:
			showHUD = !showHUD;
			break;

		case SDLK_F3:
			outputMode = OutputMode((outputMode + 1) % OUTPUT_MODE_MAX);
			if (technique == TECHNIQUE_FORWARD_PLUS && outputMode == OUTPUT_GBUFFER)
			{
				outputMode = OutputMode((outputMode + 1) % OUTPUT_MODE_MAX);
			}
			if (technique == TECHNIQUE_FORWARD && outputMode == OUTPUT_DEPTHMAP)
			{
				outputMode = OUTPUT_RENDERED;
			}
			if (technique == TECHNIQUE_DEFERRED && outputMode == OUTPUT_DEPTHMAP)
			{
				outputMode = OutputMode((outputMode + 2) % OUTPUT_MODE_MAX);
			}
			break;

		case SDLK_F4:
			technique = Technique((technique + 1) % TECHNIQUE_MAX);
			outputMode = OUTPUT_RENDERED;
			break;

		case SDLK_F5:
			curModel = (curModel + 1) % models.size();
			model = models[curModel];
			generateLights();
			break;

		case SDLK_F6:
			moveLights = !moveLights;
			break;

		case SDLK_F7:
			lightSpheres = !lightSpheres;
			break;
		}
	}
	else if (event->type == SDL_KEYUP)
	{
		switch (event->key.keysym.sym)
		{
		case SDLK_w:
		case SDLK_UP:
			camera.moveForward = false;
			break;

		case SDLK_s:
		case SDLK_DOWN:
			camera.moveBackward = false;
			break;

		case SDLK_a:
		case SDLK_LEFT:
			camera.moveLeft = false;
			break;

		case SDLK_d:
		case SDLK_RIGHT:
			camera.moveRight = false;
			break;
		}
	}
	else if (event->type == SDL_MOUSEBUTTONDOWN)
	{
		if (event->button.button == SDL_BUTTON_LEFT)
		{
			SDL_SetRelativeMouseMode((SDL_bool)true);
			mouseDown = true;
		}
	}
	else if (event->type == SDL_MOUSEBUTTONUP)
	{
		if (event->button.button == SDL_BUTTON_LEFT)
		{
			SDL_SetRelativeMouseMode((SDL_bool)false);
			mouseDown = false;
		}
	}
	else if (event->type == SDL_MOUSEMOTION && mouseDown)
	{
		int dx = event->motion.xrel;
		int dy = event->motion.yrel;

		camera.pitch = glm::clamp(camera.pitch + dy * 0.01f, radians(-89.0f), radians(89.0f));
		camera.yaw -= dx * 0.01f;

		if (camera.yaw < 0.0f) camera.yaw += radians(360.0f);
		camera.yaw = glm::mod(camera.yaw, radians(360.0f));
	}
	return 0;
}

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0)
	{
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return 0;
	}

	window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

	glContext = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(0);

	SDL_SetWindowFullscreen(window, fullScreen? SDL_WINDOW_FULLSCREEN: 0);

	glewInit();

	SDL_AddEventWatch(handleInput, NULL);

	const GLubyte *version = glGetString(GL_VERSION),
				*vendor = glGetString(GL_VENDOR),
				*renderer = glGetString(GL_RENDERER);

	cout<<"OpenGL version: "<<version<<endl
		<<"Vendor: "<<vendor<<endl
		<<"Renderer: "<<renderer<<endl<<endl;

	initialize();

	while(!quitting)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quitting = true;
			}
		}

		Uint64 now = SDL_GetTicks();
		deltaTime = now/1000.0 - curTime;
		curTime = now/1000.0;

		static float second = 0;
		static int frames = 0;

		second += deltaTime;
		frames++;

		if (second >= 1.0)
		{
			framerate = frames/second;
			second -= 1;
			frames = 0;
		}

		renderScene();
		SDL_GL_SwapWindow(window);
		SDL_Delay(2);
	}

	deinitialize();
	clearMeshes();
	clearTextures();
	clearShaders();

	SDL_DelEventWatch(handleInput, NULL);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
