/*
Student Information
Student ID: 1155210428
Student Name: Janic Moser
*/

#include "./Dependencies/glew/glew.h"
#include "./Dependencies/GLFW/glfw3.h"
#include "./Dependencies/glm/glm.hpp"
#include "./Dependencies/glm/gtc/matrix_transform.hpp"
#include "Shader.h"
#include "Texture.h"
#include "Misc.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// screen setting
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

Shader s;

double cur_previous_x, cur_previous_y;

float scRotate = 180.0f, scScale = 0.003f, scMouseRotate = 0.0f;
glm::vec3 shipPos = glm::vec3(0.0f, 0.0f, 15.0f);
float plRotate = 0.0f, plScale = 1.2f;
glm::vec3 planetPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 planetTangent[];
glm::vec3 planetBitangent[];
float asR[200], asY[200], asAng[200], asSelfR[200], asScale[200];
float craftX[3], craftY[3], craftZ[3], craftUp[3], craftScale[3];
float sunX = 15.0f, sunY = 10.0f, sunZ = -30.0f, sunBrightness = 2.0f, sunScale = 0.01f;
float dirBrightness = 1.0f;

Model spacecraft;
Texture spacecraftTexture;
GLuint spacecraftVAO, spacecraftVBO, spacecraftEBO;

Model planet;
Texture planetTexture0, planetTexture1;
GLuint planetVAO, planetVBO, planetTangentVBO, planetEBO;

Model asteroid;
Texture asteroidTexture;
GLuint asteroidVAO, asteroidVBO, asteroidEBO;

Model craft;
Texture craftTexture0, craftTexture1;
GLuint craftVAO, craftVBO, craftEBO;

Model sun;
Texture sunTexture;
GLuint sunVAO, sunVBO, sunEBO;

glm::vec3 cameraPos = shipPos + glm::vec3(0, 3, 5);
glm::vec3 cameraFront = glm::normalize(planetPos - shipPos);
glm::vec3 cameraUp = glm::vec3(0, 1, 0);
float deltaTime = 0.1f;

GLfloat skyboxVertices[] =
{
	-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
};

int E_skybox;
GLuint skyboxVAO, skyboxVBO;
Texture skyboxTexture;
Shader skyboxShader;

struct Tangent {
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

Tangent* createTangentandBitangent(Model obj) {
	
	Tangent* tangents = (Tangent*) malloc(obj.indices.size()*sizeof(Tangent));

	for (int i = 0; i < obj.indices.size(); i += 3) {
		unsigned int i0 = obj.indices[i];
		unsigned int i1 = obj.indices[i + 1];
		unsigned int i2 = obj.indices[i + 2];

		glm::vec3 &v0 = obj.vertices[i0].position;
		glm::vec3 &v1 = obj.vertices[i1].position;
		glm::vec3 &v2 = obj.vertices[i2].position;

		glm::vec2 &uv0 = obj.vertices[i0].uv;
		glm::vec2 &uv1 = obj.vertices[i1].uv;
		glm::vec2 &uv2 = obj.vertices[i2].uv;

		glm::vec3 dp1 = v1 - v0;
		glm::vec3 dp2 = v2 - v0;

		glm::vec2 duv1 = uv1 - uv0;
		glm::vec2 duv2 = uv2 - uv0;

		float r = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);
		glm::vec3 tangent = (dp1 * duv2.y - dp2 * duv1.y) * r;
		glm::vec3 bitangent = (dp2 * duv1.x - dp1 * duv2.x) * r;

		//weighet average (sum without normalization)
		tangents[i0].tangent = tangent;
		tangents[i0].bitangent = bitangent;

		tangents[i1].tangent = tangent;
		tangents[i1].bitangent = bitangent;

		tangents[i2].tangent = tangent;
		tangents[i2].bitangent = bitangent;
	}

	for (int i = 0; i < obj.vertices.size(); i++) {
		tangents[i].tangent = glm::normalize(tangents[i].tangent);
		tangents[i].bitangent = glm::normalize(tangents[i].bitangent);
	}

	return tangents;
}

void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

void loadMeshes()
{
	//spacecraft
	spacecraft = loadOBJ("./resources/spacecraft/spacecraft.obj");
	spacecraftTexture.setupTexture("./resources/spacecraft/spacecraftTexture.bmp");

	glGenVertexArrays(1, &spacecraftVAO);
	glBindVertexArray(spacecraftVAO);

	glGenBuffers(1, &spacecraftVBO);
	glBindBuffer(GL_ARRAY_BUFFER, spacecraftVBO);
	glBufferData(GL_ARRAY_BUFFER, spacecraft.vertices.size() * sizeof(Vertex), &spacecraft.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &spacecraftEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spacecraftEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, spacecraft.indices.size() * sizeof(unsigned int), &spacecraft.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	//planet
	planet = loadOBJ("./resources/planet/planet.obj");
	planetTexture0.setupTexture("./resources/planet/earthTexture.bmp");
	planetTexture1.setupTexture("./resources/planet/earthNormal.bmp");

	Tangent* tangents = createTangentandBitangent(planet);

	glGenVertexArrays(1, &planetVAO);
	glBindVertexArray(planetVAO);

	glGenBuffers(1, &planetVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planetVBO);
	glBufferData(GL_ARRAY_BUFFER, planet.vertices.size() * sizeof(Vertex), &planet.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &planetEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planetEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, planet.indices.size() * sizeof(unsigned int), &planet.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	glGenBuffers(1, &planetTangentVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planetTangentVBO);
	glBufferData(GL_ARRAY_BUFFER, planet.vertices.size() * sizeof(Tangent), &tangents[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Tangent), (void*)offsetof(Tangent, tangent));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Tangent), (void*)offsetof(Tangent, bitangent));


	//Asteroid
	asteroid = loadOBJ("./resources/asteroid/rock.obj");
	asteroidTexture.setupTexture("./resources/asteroid/rockTexture.bmp");

	for (int i = 0; i < 200; i++) {
		//Calc Distance to planet and initialAngle
		asR[i] = (std::rand() % 100) / 50.0f + 6.0f;
		asY[i] = (std::rand() % 10) / 10.0f;
		asAng[i] = (float)(std::rand() % 365);
		asSelfR[i] = (float)(std::rand() % 365); //selfRotation
		asScale[i] = (std::rand() % 20) / 100.0f + 0.1f;
	}
	glGenVertexArrays(1, &asteroidVAO);
	glBindVertexArray(asteroidVAO);

	glGenBuffers(1, &asteroidVBO);
	glBindBuffer(GL_ARRAY_BUFFER, asteroidVBO);
	glBufferData(GL_ARRAY_BUFFER, asteroid.vertices.size() * sizeof(Vertex), &asteroid.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &asteroidEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, asteroidEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, asteroid.indices.size() * sizeof(unsigned int), &asteroid.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	//craft
	craft = loadOBJ("./resources/craft/craft.obj");
	craftTexture0.setupTexture("./resources/craft/craftTexture.bmp");

	for (int i = 0; i < 3; i++) {
		craftX[i] = (std::rand() % 13) - 6.0f;
		craftY[i] = (std::rand() % 5) - 2.0f;
		craftZ[i] = (std::rand() % 4) + 8.0f;
		craftUp[i] = (std::rand() % 2);
		craftScale[i] = (std::rand() % 10) / 100.0f + 0.09f;
	}

	glGenVertexArrays(1, &craftVAO);
	glBindVertexArray(craftVAO);

	glGenBuffers(1, &craftVBO);
	glBindBuffer(GL_ARRAY_BUFFER, craftVBO);
	glBufferData(GL_ARRAY_BUFFER, craft.vertices.size() * sizeof(Vertex), &craft.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &craftEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, craftEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, craft.indices.size() * sizeof(unsigned int), &craft.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	//sun
	sun = loadOBJ("./resources/sun/sol.obj");
	sunTexture.setupTexture("./resources/sun/sunTex.jpg");

	glGenVertexArrays(1, &sunVAO);
	glBindVertexArray(sunVAO);

	glGenBuffers(1, &sunVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
	glBufferData(GL_ARRAY_BUFFER, sun.vertices.size() * sizeof(Vertex), &sun.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &sunEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sun.indices.size() * sizeof(unsigned int), &sun.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
 }

void createSkybox()
{
	//Skybox
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	E_skybox = 36;
	std::vector<std::string> texPaths;
	texPaths.push_back(std::string("./resources/skybox/right.bmp"));
	texPaths.push_back(std::string("./resources/skybox/left.bmp"));
	texPaths.push_back(std::string("./resources/skybox/top.bmp"));
	texPaths.push_back(std::string("./resources/skybox/bottom.bmp"));
	texPaths.push_back(std::string("./resources/skybox/front.bmp"));
	texPaths.push_back(std::string("./resources/skybox/back.bmp"));
	skyboxTexture.setupTextureCubemap(texPaths);
}

void sendDataToOpenGL()
{   
    loadMeshes();
    createSkybox();
}

void initializedGL(void) //run only once
{
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW not OK." << std::endl;
    }

    get_OpenGL_info();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	sendDataToOpenGL();

	s.setupShader("./nm.vs", "./nm.fs");
	skyboxShader.setupShader("./skybox.vs", "./skybox.fs");
}

void updateShader(std::string object, int num) {
	glm::mat4 rotate = glm::mat4(1.0f);
	glm::mat4 selfrotate = glm::mat4(1.0f);
	glm::mat4 scale = glm::mat4(1.0f);
	glm::mat4 translate = glm::mat4(1.0f);
	glm::mat4 matrixTransformation = glm::mat4(1.0f);

	if (object == "spacecraft") {
		rotate = glm::rotate(glm::mat4(1.0f), glm::radians(scRotate) + glm::radians(scMouseRotate), glm::vec3(0, 0, 1));
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(scScale, scScale, scScale));
		translate = glm::translate(glm::mat4(1.0f), shipPos);
		matrixTransformation = translate * rotate * scale;
	}
	else if (object == "planet") {
		plRotate += 0.05f;
		rotate = glm::rotate(glm::mat4(1.0f), glm::radians(plRotate), glm::vec3(0, 1, 0));
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(plScale, plScale, plScale));
		matrixTransformation = translate * rotate * scale;
	}
	else if (object == "asteroid") {
		asAng[num] += 0.005f; 
		translate = glm::translate(glm::mat4(1.0f), glm::vec3(asR[num], 1.0f + asY[num], 0.0f)); //distance to planet 
		selfrotate = glm::rotate(glm::mat4(1.0f), asSelfR[num], glm::vec3(0, 1, 0)); //selfRotation of Asteroid
		rotate = glm::rotate(glm::mat4(1.0f), asAng[num], glm::vec3(0, 1, 0)); //rotation around planet
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(asScale[num], asScale[num], asScale[num])); //scale of asteroid
		matrixTransformation = rotate * translate * selfrotate * scale;
	}
	else if (object == "craft") {
		if (craftUp[num]) {
			craftY[num] = std::min(craftY[num] + 0.01f, 3.0f);
		}
		else {
			craftY[num] = std::max(craftY[num] - 0.01f, -3.0f);
		}
		if (craftY[num] == 3.0f || craftY[num] == -3.0f) {
			craftUp[num] = !craftUp[num];
		}
		translate = glm::translate(glm::mat4(1.0f), glm::vec3(craftX[num], craftY[num], craftZ[num]));
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(craftScale[num], craftScale[num], craftScale[num]));
		matrixTransformation = translate * scale;
	}
	else if (object == "sun") {

		translate = glm::translate(glm::mat4(1.0f), glm::vec3(sunX, sunY, sunZ));
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(sunScale, sunScale, sunScale));
		matrixTransformation = translate * scale;
	}

	glm::mat4 Projection = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		cameraPos,
		cameraFront,
		cameraUp
	);

	glm::mat4 projectionMatrix = Projection * View;

	s.setMat4("matrixTransformation", matrixTransformation);
	s.setMat4("projectionMatrix", projectionMatrix);

	glm::vec4 ambientLight(0.5f, 0.5f, 0.5f, 1.0f);
	s.setVec4("ambientLight", ambientLight);

	//Directional Light
	glm::vec3 lightIntensity(dirBrightness, dirBrightness, dirBrightness);
	s.setVec3("lightIntensity", lightIntensity);

	glm::vec3 lightPosition(-10.0f, 10.0f, -10.0f);
	s.setVec3("lightPositionWorld", lightPosition);

	glm::vec3 sunPosition(sunX, sunY, sunZ);
	s.setVec3("sunPosition", sunPosition);

	glm::vec3 sunIntensity(sunBrightness, sunBrightness, sunBrightness);
	s.setVec3("sunIntensity", sunIntensity);

	s.setVec3("eyePositionWorld", cameraPos);

	if (object == "skybox") {
		View = (glm::mat4(glm::mat3(View)));
		skyboxShader.setMat4("view", View);
		skyboxShader.setMat4("projection", Projection);
	}
}

void paintGL(void)  //always run
{
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	s.use();

	unsigned int slot = 0;
	s.setInt("myTextureSampler0", 0);
	glActiveTexture(GL_TEXTURE0 + slot);

	s.setInt("myTextureSampler1", 1);
	glActiveTexture(GL_TEXTURE0 + 1);

	s.setInt("multipleTex", 0);

	//Draw spacecraft
	updateShader("spacecraft", 0);
	glBindVertexArray(spacecraftVAO);
	spacecraftTexture.bind(slot);
	glDrawElements(GL_TRIANGLES, spacecraft.indices.size(), GL_UNSIGNED_INT, 0);

	s.setInt("multipleTex", 1);
	//Draw planet
	updateShader("planet", 0);
	glBindVertexArray(planetVAO);
	planetTexture0.bind(slot);
	planetTexture1.bind(1);
	glDrawElements(GL_TRIANGLES, planet.indices.size(), GL_UNSIGNED_INT, 0);
	planetTexture1.unbind();

	s.setInt("multipleTex", 0);
	//Draw Asteroids
	for (int i = 0; i < 200; i++) {
		updateShader("asteroid", i);
		glBindVertexArray(asteroidVAO);
		asteroidTexture.bind(slot);
		glDrawElements(GL_TRIANGLES, asteroid.indices.size(), GL_UNSIGNED_INT, 0);
	}

	//Draw craft
	for (int i = 0; i < 3; i++) {
		updateShader("craft", i);
		glBindVertexArray(craftVAO);
		craftTexture0.bind(slot);
		glDrawElements(GL_TRIANGLES, craft.indices.size(), GL_UNSIGNED_INT, 0);
	}

	//Draw sun
	updateShader("sun", 0);
	glBindVertexArray(sunVAO);
	sunTexture.bind(slot);
	glDrawElements(GL_TRIANGLES, sun.indices.size(), GL_UNSIGNED_INT, 0);
	
	glDepthFunc(GL_LEQUAL);
	skyboxShader.use();
	updateShader("skybox", 0);
	glBindVertexArray(skyboxVAO);
	skyboxTexture.bind(slot);
	glDrawArrays(GL_TRIANGLES, 0, E_skybox);
	glDepthFunc(GL_LESS);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		cur_previous_x = x;
		cur_previous_y = y;
	}
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	scMouseRotate = -1 * (x - 400) / 10.0;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) return;

	shipPos.x -= 0.01 * (x - cur_previous_x);
	shipPos.y += 0.01 * (y - cur_previous_y);
	cur_previous_x = x;
	cur_previous_y = y;

	cameraPos = shipPos + glm::vec3(0, 3, 5);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//No Scroll interaction    
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ESC to close the window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//Control Movement
	cameraFront = glm::normalize(planetPos - shipPos);
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		shipPos += deltaTime * cameraFront;
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		shipPos -= deltaTime * cameraFront;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		shipPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * deltaTime;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		shipPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * deltaTime;
	}
	cameraPos = shipPos + glm::vec3(0, 3, 5);
}

int main(int argc, char* argv[])
{
	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HW3", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                  
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

    if(GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    get_OpenGL_info();
	initializedGL();

	while (!glfwWindowShouldClose(window)) {
        //TODO: Get time information to make the planet, rocks and crafts moving across time
        //Hints: the function to get time -> float currentTIme = static_cast<float>(glfwGetTime());
        
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}