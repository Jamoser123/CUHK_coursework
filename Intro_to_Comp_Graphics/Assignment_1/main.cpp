/*
Type your name and student ID here
	- Name: Janic Moser
	- Student ID: 1155210428
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"

#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <fstream>


GLint programID;
GLuint vao[8];

//Variables used to Manipulate Pyramid and Worker
float rotateWorker = 0;
float scaleWorker = 0.5;
int translateWorker = 0;
int scalePyramid = 0;
int yMove = 0;


void get_OpenGL_info() {
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		std::cout << buffer << std::endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID) {
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

std::string readShaderCode(const char* fileName) {
	std::ifstream meInput(fileName);
	if (!meInput.good()) {
		std::cout << "File failed to load ... " << fileName << std::endl;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders() {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	//adapter[0] = vertexShaderCode;
	std::string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	//adapter[0] = fragmentShaderCode;
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void sendDataToOpenGL() {
	// create 3D objects and/or 2D objects and/or lines (points) here and bind to VAOs & VBOs

	const GLfloat pyramid[]{
		-6.0f, +10.0f, +0.0f,  // top 0
		-11.0f, +0.0f, -5.0f,  // near left
		-1.0f, +0.0f, -5.0f,  // near right
		-1.0f, +0.0f, +5.0f,  // far right
		-11.0f, +0.0f, +5.0f,  // far left
	};

	const GLfloat pyramidCol[]{
		+0.72f, +0.53f, +0.04f,
		+0.72f, +0.53f, +0.04f,
		+0.72f, +0.53f, +0.04f,
		+0.72f, +0.53f, +0.04f,
		+0.72f, +0.53f, +0.04f,
	};

	GLuint indicesPyramid[]{
		0, 1, 2,
		0, 2, 3,
		0, 3, 4,
		0, 1, 4,
		1, 2, 4,
		2, 3, 4
	};

	//Variables to manipulate worker
	float body_top = 5.5;
	float body_bottom = 2;
	float thickness = 0.7;
	float head_multiplier = 1.42;

	const GLfloat workerBody[]{
		0.0f, body_bottom, -thickness, //0
		0.0f, body_bottom, thickness, //1
		0.0f, body_top, thickness, //2
		2.0f, body_top, -thickness, //3
		0.0f, body_top, -thickness, //4
		2.0f, body_bottom, thickness, //5
		2.0f, body_bottom, -thickness, //6
		2.0f, body_top, thickness, //7
	};

	const GLfloat workerBodyCol[]{
		0.6f, 0.0f, 0.0f,
		0.6f, 0.0f, 0.0f,
		0.6f, 0.0f, 0.0f,
		0.6f, 0.0f, 0.0f,
		0.6f, 0.0f, 0.0f,
		0.6f, 0.0f, 0.0f,
		0.6f, 0.0f, 0.0f,
		0.6f, 0.0f, 0.0f
	};

	const GLfloat workerLeg[]{
		0.0f, 0.0f, -thickness, //0
		0.0f, 0,thickness, //1
		0.0f, body_bottom, thickness, //2
		2.0f, body_bottom, -thickness, //3
		0.0f, body_bottom, -thickness, //4
		2.0f, 0.0f, thickness, //5
		2.0f, 0.0f, -thickness, //6
		2.0f, body_bottom, thickness, //7
	};

	const GLfloat workerLegCol[]{
		0.6,0.3,0.0f,
		0.6,0.3,0.0f,
		0.6,0.3,0.0f,
		0.6,0.3,0.0f,
		0.6,0.3,0.0f,
		0.6,0.3,0.0f,
		0.6,0.3,0.0f,
		0.6,0.3,0.0f
	};

	const GLfloat workerHead[]{
		0.0f, body_top,-head_multiplier * thickness , //0
		0.0f, body_top, head_multiplier * thickness , //1
		0.0f, body_top + 2, head_multiplier * thickness , //2
		2.0f, body_top + 2,-head_multiplier * thickness , //3
		0.0f, body_top + 2,-head_multiplier * thickness , //4
		2.0f, body_top, head_multiplier * thickness , //5
		2.0f, body_top,-head_multiplier * thickness, //6
		2.0f, body_top + 2, head_multiplier * thickness , //7
	};

	const GLfloat workerHeadCol[]{
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,

	};

	const GLfloat workerArmRight[]{
		2.0f, body_top - 3, -thickness, //0
		2.0f, body_top - 3, thickness, //1
		2.0f, body_top, thickness, //2
		3.0f, body_top, -thickness, //3
		2.0f, body_top, -thickness, //4
		3.0f, body_top - 3, thickness, //5
		3.0f, body_top - 3, -thickness, //6
		3.0f, body_top, thickness, //7
	};

	const GLfloat workerArmCol[]{
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
		0.91f, 0.75f, 0.67f,
	};

	const GLfloat workerArmLeft[]{
	-1.0f, body_top - 3, -thickness, //0
	-1.0f, body_top - 3, thickness, //1
	-1.0f, body_top, thickness, //2
	0.0f, body_top, -thickness, //3
	-1.0f, body_top, -thickness, //4
	0.0f, body_top - 3, thickness, //5
	0.0f, body_top - 3, -thickness, //6
	0.0f, body_top, thickness, //7
	};

	GLuint indicesWorkerPart[]{
		0, 1, 2,
		3, 0, 4,
		5, 0, 6,
		3, 6, 0,
		0, 2, 4,
		5, 1, 0,
		2, 1, 5,
		7, 6, 3,
		7, 6, 5,
		7, 3, 4,
		7, 4, 2,
		7, 2, 5 
	};

	const GLfloat floor[]{ //Make floor big
		500.0f, 0.0f, -500.0f,
		500.0f, 0.0f, 500.0f,
		-500.0f, 0.0f, 500.0f,

		500.0f, 0.0f, -500.0f,
		-500.0f,0.0f, -500.0f,
		-500.0f, 0.0f, 500.0f,
	};

	const GLfloat floorCol[]{
		1.0f, 0.84f, 0.0f,
		1.0f, 0.84f, 0.0f,
		1.0f, 0.84f, 0.0f,
		1.0f, 0.84f, 0.0f,
		1.0f, 0.84f, 0.0f,
		1.0f, 0.84f, 0.0f,

	};

	GLuint vboVertex;
	GLuint vboColor;
	GLuint indexBufferID; //also vbo but for index!

	//Bind Pyramid
	glGenVertexArrays(1, &vao[0]);
	glBindVertexArray(vao[0]);

	glGenBuffers(1, &vboVertex); //position
	glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid), pyramid, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &vboColor); //color
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidCol), pyramidCol, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &indexBufferID); //index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesPyramid), indicesPyramid, GL_STATIC_DRAW);

	//Bind body
	glGenVertexArrays(1, &vao[1]);
	glBindVertexArray(vao[1]);

	glGenBuffers(1, &vboVertex); //position
	glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerBody), workerBody, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &vboColor); //color
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerBodyCol), workerBodyCol, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &indexBufferID); //index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesWorkerPart), indicesWorkerPart, GL_STATIC_DRAW);

	//Bind leg
	glGenVertexArrays(1, &vao[2]);
	glBindVertexArray(vao[2]);

	glGenBuffers(1, &vboVertex); //position
	glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerLeg), workerLeg, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &vboColor); //color
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerLegCol), workerLegCol, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &indexBufferID); //index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesWorkerPart), indicesWorkerPart, GL_STATIC_DRAW);

	//Bind head
	glGenVertexArrays(1, &vao[3]);
	glBindVertexArray(vao[3]); 

	glGenBuffers(1, &vboVertex); //position
	glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerHead), workerHead, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &vboColor); //color
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerHeadCol), workerHeadCol, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &indexBufferID); //index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesWorkerPart), indicesWorkerPart, GL_STATIC_DRAW);

	//Bind right arm
	glGenVertexArrays(1, &vao[4]);
	glBindVertexArray(vao[4]); 

	glGenBuffers(1, &vboVertex); //position
	glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerArmRight), workerArmRight, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &vboColor); //color
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerArmCol), workerArmCol, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &indexBufferID); //index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesWorkerPart), indicesWorkerPart, GL_STATIC_DRAW);

	//Bind left arm
	glGenVertexArrays(1, &vao[5]);
	glBindVertexArray(vao[5]);  

	glGenBuffers(1, &vboVertex); //position
	glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerArmLeft), workerArmLeft, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &vboColor); //color
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(workerArmCol), workerArmCol, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &indexBufferID); //index
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesWorkerPart), indicesWorkerPart, GL_STATIC_DRAW);

	//Bind Floor
	glGenVertexArrays(1, &vao[6]);
	glBindVertexArray(vao[6]);  //first VAO

	glGenBuffers(1, &vboVertex); //position
	glBindBuffer(GL_ARRAY_BUFFER, vboVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor), floor, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &vboColor); //color
	glBindBuffer(GL_ARRAY_BUFFER, vboColor);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorCol), floorCol, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
};

void paintGL(void) {
	// always run
	// render your objects and control the transformation here

	//Background color changes with size of pyramid
	glClearColor(1.0f + scalePyramid*-0.1f, 0.27f + scalePyramid*0.05f, 0.0f + scalePyramid*0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 Model = glm::mat4(1.0f);

	glm::mat4 TranslateWorker = glm::translate(glm::mat4(1.0f), glm::vec3(translateWorker, 0.0f, 0.0f));
	glm::mat4 RotateWorker = glm::rotate(glm::mat4(1.0f), glm::degrees(rotateWorker), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 ScaleWorker = glm::scale(glm::mat4(1.0f), glm::vec3(scaleWorker, scaleWorker, scaleWorker));
	glm::mat4 ScalePyramid = glm::scale(glm::mat4(1.0f), glm::vec3(0.15*scalePyramid, 0.15*scalePyramid, 0.15*scalePyramid));

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 unts
	glm::mat4 Projection = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 20 + yMove, 15), // Camera Position 
		glm::vec3(0, 0, 0), // Looking at Origin (0,0,0)
		glm::vec3(0, 1, 0)  // Look downwards
	);

	GLint modelTransformMatrixUniformLocation =
		glGetUniformLocation(programID, "modelTransformMatrix");

	//ModelViewProjection Matrix for pyramid
	glm::mat4 MVP = Projection * View * ScalePyramid * Model;
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &MVP[0][0]);

	//Draw Pyramid
	glBindVertexArray(vao[0]);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	//ModelViewProjection Matrix for Worker
	MVP = Projection * View * TranslateWorker * RotateWorker * ScaleWorker * Model;
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &MVP[0][0]);

	//Draw Worker
	for (int i = 1; i < 6; i++) {
		glBindVertexArray(vao[i]);
		glDrawElements(GL_TRIANGLES, 54, GL_UNSIGNED_INT, 0);
	}

	//ModelViewProjection Matrix for Floor
	MVP = Projection * View * Model;
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &MVP[0][0]);
	
	//Draw Floor
	glBindVertexArray(vao[6]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) //Exit
		glfwSetWindowShouldClose(window, true);

	if (key == GLFW_KEY_W && action == GLFW_PRESS) { //Worker builds Pyramid
		scalePyramid = std::min(10, scalePyramid + 1);
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) { //Worker destroys Pyramid
		scalePyramid = std::max(0, scalePyramid - 1); 
	}
	if (key == GLFW_KEY_J && action == GLFW_PRESS) { //Worker moves left
		translateWorker = std::max(1, translateWorker - 1);
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) { //Worker moves right
		translateWorker = std::min(11, translateWorker + 1);
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) { //Worker rotates left
			rotateWorker = rotateWorker + 0.78;
	}
	if (key == GLFW_KEY_K && action == GLFW_PRESS) { //Worker rotates right
			rotateWorker = rotateWorker - 0.78;
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	yMove += yoffset; //Ability to change Camera perspective interactively
	yMove = std::max(-15, yMove);
	yMove = std::min(0, yMove);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void initializedGL(void) {
	// run only once
	sendDataToOpenGL();
	installShaders();
	glEnable(GL_DEPTH_TEST); // no clipping
}

int main(int argc, char* argv[]) {
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

	/* do not allow resizing */
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(800, 600, "Assignment 1", NULL, NULL);
	if (!window) { 
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);

	/* Initialize the glew */
	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	get_OpenGL_info();
	initializedGL();

	/* Loop until the user closes the window */
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
