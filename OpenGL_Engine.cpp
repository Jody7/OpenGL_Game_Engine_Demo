// OpenGL_Engine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>


#include <GL\freeglut.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <GLFW\glfw3.h>

#include <iostream>
#include "tiny_obj_loader.h"


int windowWidth = 1000;
int windowHeight = 800;
int windowHandle = 0;

float horizontalAngle = 0.5;
float verticalAngle = 0.0;

float speed = 3.0f;

int currentTime = 0;
int lastTime = 0;
float deltaTime = 0;

int cursorX, cursorY;

glm::vec3* toNormal = new glm::vec3[3];

GLfloat light_diffuse[] = { 0.0, 0.0, 1.0, 1.0 };
GLfloat light_position[] = {1, 1, 1, 0.0};


glm::vec3 position = glm::vec3(0, 0, -8);
glm::vec3 direction = glm::vec3(
	cos(verticalAngle) * sin(horizontalAngle),
	sin(verticalAngle),
	cos(verticalAngle) * cos(horizontalAngle)
);
glm::vec3 right = glm::vec3(
	sin(horizontalAngle - 3.14f / 2.0f),
	0,
	cos(horizontalAngle - 3.14f / 2.0f)
);

glm::vec3 up = glm::cross(right, direction);

tinyobj::attrib_t attrib;
std::vector<tinyobj::shape_t> shapes;

void Clean(int errorCode, bool bExit)
{
	if (windowHandle != 0) {
		glutDestroyWindow(windowHandle);
		windowHandle = 0;
	}

	if (bExit) {
		exit(errorCode);
	}
}

void KeyboardUpdate() {
	if (GetAsyncKeyState('W') & 0x8000)
	{
		position += direction * deltaTime * speed;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		position -= direction * deltaTime * speed;
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		position -= right * deltaTime * speed;
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		position += right * deltaTime * speed;
	}
}

void MouseUpdate() {
	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);
}

void PrepareOBJ() {
	std::string inputfile = "D:\\magnolia.obj";
	std::vector<tinyobj::material_t> materials;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());

	if (!err.empty()) { // `err` may contain warning message.
		std::cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}
}

inline glm::vec3 CalculateNormalSurfaceVector(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
	return glm::normalize(glm::cross(c-a, b-a));
}

glm::vec3 SetupAdjacentAndNormalForShading(int s, int f) {
	//per face

	tinyobj::index_t vertexIndex1 = shapes[s].mesh.indices[f];
	tinyobj::index_t vertexIndex2 = shapes[s].mesh.indices[f+1];
	tinyobj::index_t vertexIndex3 = shapes[s].mesh.indices[f+2];

	float vertex1[3] = { attrib.vertices[3 * vertexIndex1.vertex_index], attrib.vertices[3 * vertexIndex1.vertex_index + 1] , attrib.vertices[3 * vertexIndex1.vertex_index + 2] };
	float vertex2[3] = { attrib.vertices[3 * vertexIndex2.vertex_index], attrib.vertices[3 * vertexIndex2.vertex_index + 1] , attrib.vertices[3 * vertexIndex2.vertex_index + 2] };
	float vertex3[3] = { attrib.vertices[3 * vertexIndex3.vertex_index], attrib.vertices[3 * vertexIndex3.vertex_index + 1] , attrib.vertices[3 * vertexIndex3.vertex_index + 2] };

	*vertexIndex1.adjacentFaces = *vertex1;
	*vertexIndex2.adjacentFaces = *vertex2;
	*vertexIndex3.adjacentFaces = *vertex3;

	return CalculateNormalSurfaceVector(glm::vec3(vertex1[0], vertex1[1], vertex1[2]), glm::vec3(vertex2[0], vertex2[1], vertex2[2]), glm::vec3(vertex3[0], vertex3[1], vertex3[2]));
}

void VertexNormalShading() {

	for (size_t s = 0; s < shapes.size(); s++) {//idk this only runs once so....
		size_t index_offset = 0;

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {//FACES
			glm::vec3 surfaceNormal = SetupAdjacentAndNormalForShading(s, f);
			int fv = shapes[s].mesh.num_face_vertices[f];
			std::cout << "FV" << std::endl;

			for (size_t v = 0; v < fv; v++) {//VERTIX
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				float vx = attrib.vertices[3 * idx.vertex_index + 0];
				float vy = attrib.vertices[3 * idx.vertex_index + 1];
				float vz = attrib.vertices[3 * idx.vertex_index + 2];

				int numAdjacentFaces = sizeof(idx.adjacentFaces)/sizeof(idx.adjacentFaces[0]);

				std::cout << "Num Of Adjacent Faces: ";
				std::cout << numAdjacentFaces << std::endl;

				float xNormalTotal = 0.0f;
				float yNormalTotal = 0.0f;
				float zNormalTotal = 0.0f;

				for (int j = 0; j < numAdjacentFaces; j++) {

					int faceIndex = idx.adjacentFaces[j];
					glm::vec3 faceNormal = surfaceNormal;
					xNormalTotal += faceNormal.x;
					yNormalTotal += faceNormal.y;
					zNormalTotal += faceNormal.z;
					
					/*
					std::cout << faceNormal.x << std::endl;
					std::cout << faceNormal.y << std::endl;
					std::cout << faceNormal.z << std::endl;
					std::cout << std::endl;
					*/
				}

				glm::vec3 normVector = glm::normalize(glm::vec3(xNormalTotal, yNormalTotal, zNormalTotal));

				//*shapes[s].mesh.indices[index_offset + v].normVector = normVector;

				std::cout << "REAL" << std::endl;
				std::cout << normVector.x << std::endl;
				std::cout << normVector.y << std::endl;
				std::cout << normVector.z << std::endl;
			}
		}

	}
}

void PrintTheThing() {

	for (size_t s = 0; s < shapes.size(); s++) {//idk this only runs once so....
		size_t index_offset = 0;

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {//FACES
			int fv = shapes[s].mesh.num_face_vertices[f];

			for (size_t v = 0; v < fv; v++) {//VERTIX
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				glm::vec3 normVector = *idx.normVector;

				std::cout << normVector.x << std::endl;
				std::cout << normVector.y << std::endl;
				std::cout << normVector.z << std::endl;
				std::cout << std::endl;
			}
		}

	}
}

void RenderOBJ() {
	//VertexNormalShading(); this was supposed to calculate the normals at all the vertex's and do shading via that instead of shading of the surface
	//PrintTheThing();

	// Loop over shapes	
	for (size_t s = 0; s < shapes.size(); s++) {

		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			glBegin(GL_TRIANGLE_FAN);

			int fv = shapes[s].mesh.num_face_vertices[f];
			// Loop over vertices in the face.

			int toNoramlIndex = 0;

			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				if (attrib.vertices.size() >= (3 * idx.vertex_index + 2)) {
					float vx = attrib.vertices[3 * idx.vertex_index + 0];
					float vy = attrib.vertices[3 * idx.vertex_index + 1];
					float vz = attrib.vertices[3 * idx.vertex_index + 2];

					toNormal[toNoramlIndex++] = glm::vec3(vx, vy, vz);

					glVertex3f(vx, vy, vz);
				}
			}

			glm::vec3 normal = (toNormal[0] - toNormal[2]) * (toNormal[1] - toNormal[2]);

			glNormal3f(normal.x, normal.y, normal.z);
			glEnd();
			index_offset += fv;
		}
	}

}

void RenderFloor() {
	glBegin(GL_QUADS);
	glColor3f(0, 1, 0);
	glVertex3f(-100.0f, -1.0f, -100.0f);
	glVertex3f(-100.0f, -1.0f, 100.0f);
	glVertex3f(100.0f, -1.0f, 100.0f);
	glVertex3f(100.0f, -1.0f, -100.0f);
	glEnd();
}

void RenderFrame() {
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = (currentTime - lastTime) / 1000.0f;
	std::cout << position.x;
	std::cout << ":";
	std::cout << position.y;
	std::cout << ":";
	std::cout << position.z << std::endl;
	std::cout << "FPS: ";
	std::cout << (1/deltaTime) << std::endl;

	KeyboardUpdate();
	MouseUpdate();

	//handle lighting before camera transformation
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glm::vec3 vm = (position + direction);
	gluLookAt(position.x, position.y, position.z,
		vm.x, vm.y, vm.z,
		up.x, up.y, up.z);

	RenderOBJ();

	//render floor
	//RenderFloor();


	glutSwapBuffers();
	glutPostRedisplay();

	lastTime = currentTime;
}

int lastMouseX = 0;
int lastMouseY = 0;

void MouseGL(int button, int state, int x, int y)
{
	lastMouseX = x;
	lastMouseY = y;
}

float mouseSpeed = 0.05f;

void MotionGL(int x, int y)
{
	horizontalAngle += mouseSpeed * (deltaTime) * float(lastMouseX - x);
	verticalAngle += mouseSpeed * (deltaTime) * float(lastMouseY - y);

	lastMouseX = x;
	lastMouseY = y;
}

void LightingInit() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glShadeModel(GL_SMOOTH);

	GLfloat white[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat cyan[] = { 0.f, .8f, .8f, 1.f };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, cyan);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);

	GLfloat shininess[] = {1};

	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
}

void StartGL(int argc, char* argv[])
{
	glutInit(&argc, argv);

	std::cout << "GL Start";

	int iScreenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int iScreenHeight = glutGet(GLUT_SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(windowWidth, windowHeight);

	windowHandle = glutCreateWindow("gl_engine");

	LightingInit();

	// occulasion
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	gluPerspective(75.0, 1.0, 1.0, 200.0); //fov, aspect, near, far
	glMatrixMode(GL_MODELVIEW);

	glutDisplayFunc(RenderFrame);
	glutMouseFunc(MouseGL);
	glutMotionFunc(MotionGL);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glShadeModel(GL_SMOOTH);

	glfwSetTime(1);

	glutMainLoop();
}

int main(int argc, char* argv[])
{
	PrepareOBJ();
	StartGL(argc, argv);
	Clean(0, true);
	system("pause");
	return 0;
}
