

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


//#define EXIT_ON_GL_ERROR
// change relative shader file path for Xcode debug build executable location
//#define XCODE



using namespace glm;
using namespace std;



constexpr unsigned	SCALE = 18.0;
constexpr unsigned  WINDOW_WIDTH =      64 * SCALE;
constexpr unsigned  WINDOW_HEIGHT =     32 * SCALE;
constexpr float     FOV =               45.0;
constexpr unsigned  MSAA_SAMPLES =      4;



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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_SAMPLES, MSAA_SAMPLES);
	
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "picube", NULL, NULL);
	if (!window) {
		cout << "Error creating GLFW window." << endl;
		return nullptr;
	}
	
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	
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

GLuint CreateProgram(string baseName) {
	
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
	CheckProgramLink(prog);
	
	return prog;
}

unsigned CreatCube(GLint vPos, GLint vNorm, GLint vColor) {
	
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
		{ vec3(HDIM, -HDIM, HDIM),		vec3(0.0f, -1.0f, 0.0f),	PURPLE_COLOR },		// -x, -y, -z
		
		// LEFT, 1
		
		{ vec3(-HDIM, HDIM, HDIM),		vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, y, z
		{ vec3(-HDIM, HDIM, -HDIM),		vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, y, -z
		{ vec3(-HDIM, -HDIM, -HDIM),		vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, -y, -z
		
		// LEFT, 2
		
		{ vec3(-HDIM, -HDIM, -HDIM),		vec3(-1.0f, 0.0f, 0.0f),	GREEN_COLOR },		// -x, -y, -z
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
	
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
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

int main(int argc, const char* argv[]) {
	
	GLFWwindow* window = InitializeGLFW();
	
	if (window) {
		if (InitializeGLEW()) {
			srand(time(NULL));
			
			GLuint program = CreateProgram("cube");
			glUseProgram(program);

			GLint vPos = glGetAttribLocation(program, "vPos");
			GLint vNorm = glGetAttribLocation(program, "vNorm");
			GLint vColor = glGetAttribLocation(program, "vColor");

			unsigned numVerts = CreatCube(vPos, vNorm, vColor);
			
			// MODEL
			mat4 model = mat4(1.0f);
			
			// VIEW
			vec3 view_eye = { 0, 0, 3 };
			vec3 view_center = { 0, 0, 0 };
			vec3 view_up = { 0, 1, 0 };
			mat4 view = lookAt(view_eye, // eye - location
							   view_center, // center - look at
							   view_up); // up
			
			// PROJECTION
			mat4 projection = perspectiveFov((float)radians(FOV),
											 (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT,
											 1.0f, 100.0f);

			GLint modelLoc = glGetUniformLocation(program, "model");
			GLint viewLoc = glGetUniformLocation(program, "view");
			GLint projectionLoc = glGetUniformLocation(program, "projection");

			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);
			
			while (!glfwWindowShouldClose(window)) {
				
				float time = glfwGetTime();
				static float previousSeconds = time;
				float deltaSeconds = time - previousSeconds;
				previousSeconds = time;
				
				glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
				glClearColor(0.0, 0.0, 0.0, 1.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
				
				static float angle = 0.0;
				constexpr float degSec = 30.0;
				angle += degSec * deltaSeconds;
				model = rotate(mat4(1.0), radians(angle), {1, 2, 3});
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));
				

				glDrawArrays(GL_TRIANGLES, 0, numVerts);

				glfwSwapBuffers(window);

				glfwPollEvents();
				
				if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
					glfwSetWindowShouldClose(window, GLFW_TRUE);
				}
				
				CheckError(__LINE__);
			}
			
			glfwDestroyWindow(window);
		}
		
		glfwTerminate();
	}
}

