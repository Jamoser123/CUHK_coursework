/*
Student Information
Student ID: 1155210428
Student Name: Janic Moser
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

/*TODO:
* - test project
* - check requirements
* - add personal scene -> pumpkin + zombies
* - during night pumpking becomes a light source and scary
* - zombie is there during day and night
*/

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

// screen setting
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

//experimental
float yaw = -90.0f;
float pitch = 17.0f;

//variables
bool isDay = 1;
bool dogForm = 0;
double brightness = 10;
bool lBpressed = 0;

float transX = 0;
float transZ = 0;
float angle = 0.0f;

double cameraX, cameraY, cameraZ;

double initialXpos, initialYpos;

/*GLuint vaoDog, vaoGround, vaoZombie, vaoPumpkin,
		vboDog, vboGround, vboZombie, vboPumpkin,
	    eboDog, eboGround, eboZombie, eboPumpkin;
		*/
Shader s;

Model dog;
GLuint vaoDog, vboDog, eboDog;
//Texture dT0, dT1;

/*Model ground;
Texture gT0, gT1;

Model zombie;
Texture zBT0, zBT1, zCT0, zCT1;
*/
/*Model pumpkin;
Texture pT0;// , pT1;
*/

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
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

void sendDataToOpenGL()
{
	//Dog
	dog = loadOBJ("./resources/dog/dog.obj");
	std::cout << "Dog Vertices: " << dog.vertices.size() << std::endl;
	std::cout << "Dog Indices: " << dog.indices.size() << std::endl;
	//dT0.setupTexture("./resources/dog/dog_01.jpg");
	//dT1.setupTexture("./resources/dog/dog_02.jpg");

	//bind dog
	glGenVertexArrays(1, &vaoDog);
	glBindVertexArray(vaoDog);

	//vbo
	glGenBuffers(1, &vboDog); //position
	glBindBuffer(GL_ARRAY_BUFFER, vboDog);
	glBufferData(GL_ARRAY_BUFFER, dog.vertices.size() * sizeof(Vertex), &dog.vertices[0], GL_STATIC_DRAW);

	// bind pos 0: position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	// bind pos 1: UV
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	// bind pos 2: normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);

	//ebo
	glGenBuffers(1, &eboDog); //index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboDog);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, dog.indices.size() * sizeof(unsigned int), &dog.indices[0], GL_STATIC_DRAW);
	/*
	//Ground
	ground = loadOBJ("./resources/ground/ground.obj");
	gT0.setupTexture("./resources/ground/ground_01.jpg");
	gT1.setupTexture("./resources/ground/ground_02.jpg");

	//vao
	glGenVertexArrays(1, &vaoGround);
	glBindVertexArray(vaoGround);

	//vbo
	glGenBuffers(1, &vboGround);
	glBindBuffer(GL_ARRAY_BUFFER, vboGround);
	glBufferData(GL_ARRAY_BUFFER, ground.vertices.size() * sizeof(Vertex), &ground.vertices[0], GL_STATIC_DRAW);

	//ebo
	glGenBuffers(1, &eboGround);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboGround);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ground.indices.size() * sizeof(unsigned int), &ground.indices[0], GL_STATIC_DRAW);

	// bind pos 0: position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	// bind pos 1: UV
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	// bind pos 2: normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);

	//Zombie
	zombie = loadOBJ("./resources/zombies/uploads_files_1005291_zombie_full.obj");
	zBT0.setupTexture("./resources/zombies/Textures/body/Zombie_defaultMat_AlbedoTransparency.png");
	zCT0.setupTexture("./resources/zombies/Textures/cloth/cloth_zombie_Material3739_AlbedoTransparency.png");
	zBT1.setupTexture("./resources/zombies/Textures/body2/Zombie_defaultMat_AlbedoTransparency.png");
	zCT1.setupTexture("./resources/zombies/Textures/cloth2/cloth_zombie_Material3739_AlbedoTransparency.png");

	//vao
	glGenVertexArrays(1, &vaoZombie);
	glBindVertexArray(vaoZombie);

	//vbo
	glGenBuffers(1, &vboZombie);
	glBindBuffer(GL_ARRAY_BUFFER, vboZombie);
	glBufferData(GL_ARRAY_BUFFER, zombie.vertices.size() * sizeof(Vertex), &zombie.vertices[0], GL_STATIC_DRAW);

	//ebo
	glGenBuffers(1, &eboZombie);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboZombie);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, zombie.indices.size() * sizeof(unsigned int), &zombie.indices[0], GL_STATIC_DRAW);

	// bind pos 0: position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	// bind pos 1: UV
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	// bind pos 2: normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);
	/*
	//Pumpkin
	pumpkin = loadOBJ("./resources/pumpkin/pumpkin.obj");
	pT0.setupTexture("./resources/pumpkin/pumpkin_material/pumpkin_Base_Color.png");
	//pT1.setupTexture("./resources/ground/ground_02.jpg");

	//vao
	glGenVertexArrays(1, &vaoPumpkin);
	glBindVertexArray(vaoPumpkin);

	//vbo
	glGenBuffers(1, &vboPumpkin);
	glBindBuffer(GL_ARRAY_BUFFER, vboPumpkin);
	glBufferData(GL_ARRAY_BUFFER, pumpkin.vertices.size() * sizeof(Vertex), &pumpkin.vertices[0], GL_STATIC_DRAW);

	//ebo
	glGenBuffers(1, &eboPumpkin);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboPumpkin);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, pumpkin.indices.size() * sizeof(unsigned int), &pumpkin.indices[0], GL_STATIC_DRAW);

	// bind pos 0: position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, position) // array buffer offset
	);

	// bind pos 1: UV
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, // attribute
		2, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, uv) // array buffer offset
	);

	// bind pos 2: normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, // attribute
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized?
		sizeof(Vertex), // stride
		(void*)offsetof(Vertex, normal) // array buffer offset
	);*/

}

void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();
	s.setupShader("./VertexShaderCode.glsl", "./FragmentShaderCode.glsl");
	//TODO: set up the camera parameters	

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void updateShader(std::string obj) 
{
	glm::mat4 Rotate = glm::mat4(1.0f);
	glm::mat4 Scale = glm::mat4(1.0f);
	glm::mat4 Translate = glm::mat4(1.0f);

	if (obj == "ground") {
		Scale = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
	}
	else if (obj == "dog") {
		Rotate = glm::rotate(glm::mat4(1.0f),  angle, glm::vec3(0, 1, 0));
		Scale = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));
		Translate = glm::translate(glm::mat4(1.0f), glm::vec3(transX, 0, transZ));
	}

	cameraX = 25.0 * sin(cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
	cameraY = 25.0 * sin(glm::radians(pitch));
	cameraZ = -25.0 * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 20, 15),//cameraX, cameraY, cameraZ), //cam  camX, camY, camZ -> todo cam manipulation!
		glm::vec3(0, 0, 0), //look
		glm::vec3(0, 1, 0)
	);

	//We send the matrices to the shaders
	glm::mat4 modelTransformMatrix = Translate * Rotate * Scale;
	s.setMat4("modelTransformMatrix", modelTransformMatrix);

	glm::mat4 projectionMatrix = Projection * View;
	s.setMat4("projectionMatrix", projectionMatrix);

	//ambient light
	glm::vec4 ambientLight(1.0f, 1.0f, 1.0f, 1.0f);
	s.setVec4("ambientLight", ambientLight);

	glm::vec4 lightIntensity(brightness, brightness, brightness, 1.0f);
	s.setVec4("lightIntensity", lightIntensity);

	glm::vec3 lightPosition(2.0f, 15.0f, -10.0f); //was before 2.0f, 15.0f, -10.0f
	s.setVec3("lightPositionWorld", lightPosition);

	glm::vec3 eyePosition(cameraX, cameraY, cameraZ);
	s.setVec3("eyePositionWorld", eyePosition);
}

void paintGL(void)  //always run
{
	//glClearColor(isDay ? 0.529f : 0.7f, isDay ? 0.808f : 0.208f, isDay ? 0.98f : 0.0f, 0.0f); // -> last value previously was 0.5f
	glClearColor(1.0f, 1.0f, 1.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*unsigned int slot = 0;
	s.setInt("myTextureSampler0", slot); //unklar ob das worked
	glActiveTexture(GL_TEXTURE0 + slot);*/
	
	//Draw dog
	//updateShader("dog");//"dog");

	glm::mat4 Rotate = glm::mat4(1.0f);
	glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
	glm::mat4 Translate = glm::mat4(1.0f);

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 20, 15),//cameraX, cameraY, cameraZ), //cam  camX, camY, camZ -> todo cam manipulation!
		glm::vec3(0, 0, 0), //look
		glm::vec3(0, 1, 0)
	);

	//We send the matrices to the shaders
	glm::mat4 modelTransformMatrix = Translate * Rotate * Scale;
	s.setMat4("modelTransformMatrix", modelTransformMatrix);

	glm::mat4 projectionMatrix = Projection * View;
	s.setMat4("projectionMatrix", projectionMatrix);

	glBindVertexArray(vaoDog);
	//if (!dogForm) dT0.bind(slot);
	//else dT1.bind(slot);
	glDrawElements(GL_TRIANGLES, dog.indices.size(), GL_UNSIGNED_INT, 0);
	/*
	//Draw Ground
	updateShader("ground");
	glBindVertexArray(vaoGround);
	if (isDay) dT0.bind(slot);
	else dT1.bind(slot);
	glDrawElements(GL_TRIANGLES, ground.indices.size(), GL_UNSIGNED_INT, 0);

	//Draw Zombie
	updateShader("zombie");
	glBindVertexArray(vaoZombie);
	zBT0.bind(slot);
	zCT0.bind(slot);
	glDrawElements(GL_TRIANGLES, zombie.indices.size(), GL_UNSIGNED_INT, 0);

	//Draw Pumpkin
	updateShader("pumpkin");
	glBindVertexArray(vaoPumpkin);
	pT0.bind(slot);
	glDrawElements(GL_TRIANGLES, pumpkin.indices.size(), GL_UNSIGNED_INT, 0);*/
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	/*if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		lBpressed = 1;
		glfwGetCursorPos(window, &initialXpos, &initialYpos);
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		lBpressed = 0;
	}*/
}
float sens = 0.05f;

void cursor_position_callback(GLFWwindow* window, double x, double y)
{	
	/*if (lBpressed) { //TODO: scale y difference
		yaw += sens * (initialXpos - x);
		pitch += sens * (initialYpos - y);
		pitch = glm::clamp(pitch, -89.0, 89.0);
	}*/
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) //Todo manipulate z position?
{
	// Sets the scoll callback for the current window.
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) //Exit
		glfwSetWindowShouldClose(window, true);

	//Manipulate Setting
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		isDay = 1;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) { //Change setting -> evtl nur 1 Button
		isDay = 0;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		dogForm = 0;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) { //Change DragonForm -> evtl nur 1 Button for changing forms
		dogForm = 1;
	}
	/*if (key == GLFW_KEY_W && action == GLFW_PRESS) { //decrease Brightness from directional Light
		brightness = std::max(brightness - 1, 0.0);
	}
	if (key == GLFW_KEY_S && action == GLFW_PRESS) { //increase Brightness from directional Light
		brightness = std::min(brightness + 1, 1000.0);
	}

	//Manipulate Object
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		transX = std::max(transX - 1.0f, 0.0f);
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		transX = std::min(transX + 1.0f, 100.0f);
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		transZ = std::min(transZ + 1.0f, 100.0f);
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		transZ = std::max(transZ - 1.0f, 0.0f);
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		//TODO: implement random movement
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		angle += glm::radians(45.0f);
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		angle -= glm::radians(45.0f);
	}*/
}


int main(int argc, char* argv[])
{
	GLFWwindow* window;

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
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
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

	initializedGL();

	while (!glfwWindowShouldClose(window)) {
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






