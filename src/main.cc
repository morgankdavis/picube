

#include <ctime>
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rotator.h"
#include "yarandom.h"



#include <unistd.h>


//#define EXIT_ON_GL_ERROR


using namespace glm;
using namespace std;



constexpr unsigned	FB_SCALE = 			18.0;
constexpr unsigned  FB_WIDTH =      	64 * FB_SCALE;
constexpr unsigned  FB_HEIGHT =     	32 * FB_SCALE;
constexpr unsigned  MSAA_SAMPLES =      4;
constexpr float		TARGET_FPS =		60.0;
constexpr float     FOV =               30.0;
constexpr float		CAM_DISTANCE =		4.0;

// configure the random movement of the object

constexpr float 	SPIN_SPEED = 		0.15;
constexpr float 	WANDER_SPEED = 		0.0035;
constexpr float 	SPIN_ACCEL = 		0.2;
constexpr float		WANDER_X =			3.0;
constexpr float		WANDER_Y =			1.0;
constexpr float		WANDER_Z =			1.0;


typedef struct {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
} Vertex;



float D2R(float d) {
	return d * (M_PI/180.0);
}

float R2D(float r) {
	return r * (180.0/M_PI);
}

void CheckError(unsigned line) {
	
	GLenum glErr = glGetError();
	if (glErr != GL_NO_ERROR) {
		cout << "OpenGL error(" << line << "): " << glErr << endl;
#ifdef EXIT_ON_GL_ERROR
		exit(1);
#endif
	}
}

void GLFWErrorCallback(int error, const char* description) {
	cout << "GLFWErrorCallback(): error: " << error << ", description: " << description << endl;
}

GLFWwindow* InitializeGLFW() {
	
	cout << "InitializeGLFW()" << endl;
	
	int glfwMajVers, glfwMinVers, glfwRev;
	glfwGetVersion(&glfwMajVers, &glfwMinVers, &glfwRev);
	cout << "Starting GLFW version " << glfwMajVers << "." << glfwMinVers << "." << glfwRev << endl;
	
	glfwSetErrorCallback(GLFWErrorCallback);
	
	if (glfwInit()) {
		cout << "GLFW Initialized." << endl;
	}
	else {
		cout << "Error initializing GLFW." << endl;
		return nullptr;
	}
	
	// create the GLFW window
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLES);
	
	GLFWwindow* window = glfwCreateWindow(FB_WIDTH, FB_HEIGHT, "picube", NULL, NULL);
	if (!window) {
		cout << "Error creating GLFW window." << endl;
		return nullptr;
	}
	
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	
	return window;
}

bool InitializeGLEW() {
	
	// NOTE: OpenGL context must be setup first
	
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		cout << "Error initializing GLEW: " << err << endl;
		return false;
	}
	
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	
	cout << "Renderer: " << renderer << endl;
	cout << "Version: " << version << endl;
	
	return true;
}

string LoadTextFile(const string &path) {
	
	string line;
	string source = "";
	ifstream infile;

	infile.open(path);
	
	if (infile.is_open()) {
		
		while (!infile.eof()) {
			getline(infile, line);
			//cout << line << endl;
			source += line;
			source += "\n";
		}
		
		infile.close();
	}
	else {
		cout << "Couldn't open file: " << path << endl;
	}
	
	return source;
}

bool CheckShaderCompile(GLuint shaderID) {
	
	int params = -1;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &params);
	if (params != GL_TRUE) {
		fprintf(stderr, "ERROR, could not compile shader at index %d.", shaderID);
		
		int max_length = 2048;
		int actual_length = 0;
		char log[2048];
		glGetShaderInfoLog(shaderID, max_length, &actual_length, log);
		printf("shader info log for GL index %u:\n%s\n", shaderID, log);
		
		return false;
	}
	return true;
}

bool CheckProgramLink(GLuint programID) {
	
	int params = -1;
	glGetProgramiv(programID, GL_LINK_STATUS, &params);
	if (params != GL_TRUE) {
		fprintf(stderr, "ERROR, could not link program at index %d.", programID);
		
		int max_length = 2048;
		int actual_length = 0;
		char log[2048];
		glGetProgramInfoLog(programID, max_length, &actual_length, log);
		printf("program info log for GL index %u:\n%s\n", programID, log);
		
		return false;
	}
	return true;
}

bool CreateProgram(string baseName, GLuint* programID) {
	
	string prefix = "";
#ifdef XCODE
	prefix = "../../";
#endif
	string vert_path = prefix + "shaders/" + baseName + ".vert";
	string vert_source = LoadTextFile(vert_path);
	const char *vert_source_cstr = vert_source.c_str();
	
	string frag_path = prefix + "shaders/" + baseName + ".frag";
	string frag_source = LoadTextFile(frag_path);
	const char *frag_source_cstr = frag_source.c_str();
	
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vert_source_cstr, NULL);
	glCompileShader(vs);
	CheckShaderCompile(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &frag_source_cstr, NULL);
	glCompileShader(fs);
	CheckShaderCompile(fs);
	
	GLuint prog = glCreateProgram();
	glAttachShader(prog, fs);
	glAttachShader(prog, vs);
	
	glLinkProgram(prog);
	if (!CheckProgramLink(prog)) {
		return false;
	}
	
	*programID = prog;
	
	return true;
}

unsigned CreatCube(GLint vPos, GLint vNorm, GLint vColor, GLuint* vbo) {
	
	constexpr float DIM = 1.0;
	constexpr float HDIM = DIM/2.0;
	
	const vec3 BLUE_COLOR = { 0.0/255.0, 112.0/255.0, 175.0/255.0 };
	const vec3 GREEN_COLOR = { 29.0/255.0, 122.0/255.0, 51.0/255.0 };
	const vec3 PURPLE_COLOR = { 133.0/255.0, 95.0/255.0, 167.0/255.0 };
	
	Vertex verts[] = {
		
		// default winding order is ccw
		
		// TOP, 1
		
		{ vec3(HDIM, HDIM, HDIM),		vec3(0.0f, 1.0f, 0.0f),		PURPLE_COLOR }, 	// x, y, z
		{ vec3(HDIM, HDIM, -HDIM),		vec3(0.0f, 1.0f, 0.0f),		PURPLE_COLOR }, 	// x, y, -z
		{ vec3(-HDIM, HDIM, -HDIM),		vec3(0.0f, 1.0f, 0.0f),		PURPLE_COLOR }, 	// -x, y, -z
		
		// TOP, 2
		
		{ vec3(-HDIM, HDIM, -HDIM),		vec3(0.0f, 1.0f, 0.0f),		PURPLE_COLOR }, 	// -x, y, -z
		{ vec3(-HDIM, HDIM, HDIM),		vec3(0.0f, 1.0f, 0.0f),		PURPLE_COLOR }, 	// -x, y, z
		{ vec3(HDIM, HDIM, HDIM),		vec3(0.0f, 1.0f, 0.0f),		PURPLE_COLOR }, 	// x, y, z
		
		// BOTTOM, 1
		
		{ vec3(-HDIM, -HDIM, -HDIM),	vec3(0.0f, -1.0f, 0.0f),	PURPLE_COLOR },		// -x, -y, -z
		{ vec3(HDIM, -HDIM, -HDIM),		vec3(0.0f, -1.0f, 0.0f),	PURPLE_COLOR }, 	// x, -y, -z
		{ vec3(HDIM, -HDIM, HDIM),		vec3(0.0f, -1.0f, 0.0f),	PURPLE_COLOR },		// x, -y, z
		
		// BOTTOM, 2
		
		{ vec3(HDIM, -HDIM, HDIM),		vec3(0.0f, -1.0f, 0.0f),	PURPLE_COLOR },		// x, -y, z
		{ vec3(-HDIM, -HDIM, HDIM),		vec3(0.0f, -1.0f, 0.0f),	PURPLE_COLOR },		// -x, -y, z
		{ vec3(-HDIM, -HDIM, -HDIM),	vec3(0.0f, -1.0f, 0.0f),	PURPLE_COLOR },		// -x, -y, -z
		
		// LEFT, 1
		
		{ vec3(-HDIM, HDIM, HDIM),		vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, y, z
		{ vec3(-HDIM, HDIM, -HDIM),		vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, y, -z
		{ vec3(-HDIM, -HDIM, -HDIM),	vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, -y, -z
		
		// LEFT, 2
		
		{ vec3(-HDIM, -HDIM, -HDIM),	vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, -y, -z
		{ vec3(-HDIM, -HDIM, HDIM),		vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, -y, z
		{ vec3(-HDIM, HDIM, HDIM),		vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, y, z
		
		// RIGHT, 1
		
		{ vec3(HDIM, -HDIM, -HDIM),		vec3(1.0f, 0.0f, 0.0f),		GREEN_COLOR },		// x, -y, -z
		{ vec3(HDIM, HDIM, -HDIM),		vec3(1.0f, 0.0f, 0.0f),		GREEN_COLOR },		// x, y, -z
		{ vec3(HDIM, HDIM, HDIM),		vec3(1.0f, 0.0f, 0.0f),		GREEN_COLOR },		// x, y, z
		
		// RIGHT, 2
		
		{ vec3(HDIM, HDIM, HDIM),		vec3(1.0f, 0.0f, 0.0f),		GREEN_COLOR },		// x, y, z
		{ vec3(HDIM, -HDIM, HDIM),		vec3(1.0f, 0.0f, 0.0f),		GREEN_COLOR },		// x, -y, z
		{ vec3(HDIM, -HDIM, -HDIM),		vec3(1.0f, 0.0f, 0.0f),		GREEN_COLOR },		// x, -y, -z
		
		// FRONT, 1
		
		{ vec3(HDIM, -HDIM, HDIM),		vec3(0.0f, 0.0f, 1.0f),		BLUE_COLOR },		// x, -y, z
		{ vec3(HDIM, HDIM, HDIM),		vec3(0.0f, 0.0f, 1.0f),		BLUE_COLOR },		// x, y, z
		{ vec3(-HDIM, HDIM, HDIM),		vec3(0.0f, 0.0f, 1.0f),		BLUE_COLOR },		// -x, y, z
		
		// FRONT, 2
		
		{ vec3(-HDIM, HDIM, HDIM),		vec3(0.0f, 0.0f, 1.0f),		BLUE_COLOR },		// -x, y, z
		{ vec3(-HDIM, -HDIM, HDIM),		vec3(0.0f, 0.0f, 1.0f),		BLUE_COLOR },		// -x, -y, z
		{ vec3(HDIM, -HDIM, HDIM),		vec3(0.0f, 0.0f, 1.0f),		BLUE_COLOR },		// x, -y, z
		
		// BACK, 1
		
		{ vec3(-HDIM, -HDIM, -HDIM),	vec3(0.0f, 0.0f, -1.0f),	BLUE_COLOR },		// -x, -y, -z
		{ vec3(-HDIM, HDIM, -HDIM),		vec3(0.0f, 0.0f, -1.0f),	BLUE_COLOR },		// -x, y, -z
		{ vec3(HDIM, HDIM, -HDIM),		vec3(0.0f, 0.0f, -1.0f),	BLUE_COLOR },		// x, y, -z
		
		// BACK, 2
		
		{ vec3(HDIM, HDIM, -HDIM),		vec3(0.0f, 0.0f, -1.0f),	BLUE_COLOR },		// x, y, -z
		{ vec3(HDIM, -HDIM, -HDIM),		vec3(0.0f, 0.0f, -1.0f),	BLUE_COLOR },		// x, -y, -z
		{ vec3(-HDIM, -HDIM, -HDIM),	vec3(0.0f, 0.0f, -1.0f),	BLUE_COLOR }		// -x, -y, -z
	};
	
	unsigned numTriangles = 12;
	
	//GLuint vbo;
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 3 * numTriangles, verts, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(vPos);
	glVertexAttribPointer(vPos,
						  3,
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(Vertex),
						  0);

	glEnableVertexAttribArray(vNorm);
	glVertexAttribPointer(vNorm,
						  3,
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(Vertex),
						  (void *)sizeof(vec3));
	
	
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor,
						  3,
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(Vertex),
						  (void *)(sizeof(vec3) + sizeof(vec3)));
	
	return 3 * numTriangles;
}

unsigned char* CreateSnapshot() {
	
	unsigned framebufferWidth = FB_WIDTH;
	unsigned framebufferHeight = FB_HEIGHT;
	unsigned char* pixelBuf = (unsigned char*)malloc(framebufferWidth * framebufferHeight * 4);
	
	glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuf);
	
	return pixelBuf;
}

int main(int argc, const char* argv[]) {
	
	GLFWwindow* window = InitializeGLFW();
	
	if (window) {
		if (InitializeGLEW()) {
			
			GLuint program;
			if (CreateProgram("cube", &program)) {
				glUseProgram(program);
				
				GLint vPos = glGetAttribLocation(program, "vPos");
				GLint vNorm = glGetAttribLocation(program, "vNorm");
				GLint vColor = glGetAttribLocation(program, "vColor");
				
				GLuint vbo;
				unsigned numVerts = CreatCube(vPos, vNorm, vColor, &vbo);
				
				// MODEL
				mat4 model = mat4(1.0f);
				
				// VIEW
				vec3 view_eye = { 0, 0, CAM_DISTANCE };
				vec3 view_center = { 0, 0, 0 };
				vec3 view_up = { 0, 1, 0 };
				mat4 view = lookAt(view_eye, // eye - location
								   view_center, // center - look at
								   view_up); // up
				
				// PROJECTION
				mat4 projection = perspectiveFov((float)radians(FOV),
												 (float)FB_WIDTH, (float)FB_HEIGHT,
												 1.0f, 100.0f);
				
				GLint modelLoc = glGetUniformLocation(program, "model");
				GLint viewLoc = glGetUniformLocation(program, "view");
				GLint projectionLoc = glGetUniformLocation(program, "projection");
				
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
				glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));
				
				
				
				ya_rand_init(0); // normally this is done internally by xscreensaver
				rotator* rotator = make_rotator(SPIN_SPEED,
												SPIN_SPEED,
												SPIN_SPEED,
												SPIN_ACCEL,
												WANDER_SPEED,
												true);

				
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				glDepthMask(GL_TRUE);
				

				while (!glfwWindowShouldClose(window)) {
					
					
					
					float time = glfwGetTime();
					static float lastFrameTime = time;
					float deltaSeconds = time - lastFrameTime;
					//previousSeconds = time;
					
					if (deltaSeconds > 1.0/TARGET_FPS) {
						
						lastFrameTime = time;
						
						//				float a = 500.0;
						//				static float timeOffset = ((float)rand()/(float)(RAND_MAX)) * a;
						//				float time = glfwGetTime() + timeOffset;
						//				static float previousSeconds = time;
						//				float deltaSeconds = time - previousSeconds;
						//				previousSeconds = time;
						
						glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
						glClearColor(0.0, 0.0, 0.0, 1.0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						
						{
							double x, y, z;
							
							get_position(rotator, &x, &y, &z, 1);
							x -= 0.5; y -= 0.5; z -= 0.5;
							mat4 translate = glm::translate(mat4(1.0), { x * WANDER_X, y * WANDER_Y, z * WANDER_Z});
							
							get_rotation(rotator, &x, &y, &z, 1);
							mat4 rotateX = rotate(mat4(1.0), radians((float)x * 360.0f), { 1, 0, 0 });
							mat4 rotateY = rotate(mat4(1.0), radians((float)y * 360.0f), { 0, 1, 0 });
							mat4 rotateZ = rotate(mat4(1.0), radians((float)z * 360.0f), { 0, 0, 1 });
							
							model = translate * rotateZ * rotateY * rotateX;
							
							glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));
						}
						
						glDrawArrays(GL_TRIANGLES, 0, numVerts);
						
						glfwSwapBuffers(window);
						
						//unsigned char* snapshot = CreateSnapshot();
						
						glfwPollEvents();
						
						if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
							glfwSetWindowShouldClose(window, GLFW_TRUE);
						}
						
						CheckError(__LINE__);
					}
					else {
						usleep(1);
					}
				}
				
				free_rotator(rotator);
				glDeleteProgram(program);
				glDeleteBuffers(1, &vbo);
			}
			
			glfwDestroyWindow(window);
		}
		
		glfwTerminate();
	}
}
