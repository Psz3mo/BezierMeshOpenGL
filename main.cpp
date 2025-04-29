#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "ShaderProgram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Camera.h"

using namespace std;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const unsigned int n = 5, m = 5; // n and m are the number of control points in the u and v direction
const unsigned int resolution = 50; // u and v resolution
const unsigned int pointSize = 5; // Point size
const unsigned int lineSize = 2; // Line size

void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void render(double time, ShaderProgram& shaderProgram, ShaderProgram& shaderProgram2);
void generateMeshVertices(); // Generate mesh vertices
void renderMesh(ShaderProgram& shaderProgram); // Render mesh
void renderBezierSurface(ShaderProgram& shaderProgram); // Render bezier surface
void generateLines(); // Generate lines
void generateTriangles(); // Generate triangles
void generateMeshLines(); // Generate mesh lines
void setShaderMatrices(ShaderProgram& shaderProgram); // Set shader matrices
void createVAO(); // Create VAO
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
float B(int n, int k, float u);
int binomialCoefficient(int n, int k);
glm::vec3 bezierSurface(float u, float v);
glm::vec3 calculateRayFromMouse(double xpos, double ypos, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
bool isPointIntersected(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& point, float threshold);

Camera camera(glm::vec3(0.0f, 0.0f, 7.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool editMode = false;
bool mouseLeftButtonPressed = false;

glm::vec3 controlPoints[n][m] = { // Control points
	{{0,0,4}, {1,0,4}, {2,0,4}, {3,0,4}, {4,1,4}},
	{{0,0,3}, {1,1,3}, {2,1,3}, {3,1,3}, {4,1,3}},
	{{0,1,2}, {1,2,2}, {2,6,2}, {3,2,2}, {4,1,2}},
	{{0,0,1}, {1,1,1}, {2,1,1}, {3,1,1}, {4,1,1}},
	{{0,0,0}, {1,0,0}, {2,0,0}, {3,0,0}, {4,1,0}}
};

glm::mat4 model, projection, view;

glm::vec3 meshVertices[resolution][resolution]; // Mesh vertices

vector<unsigned int> lines, triangles, meshLines, meshTriangles;

GLuint lineVBO, lineVAO, lineEBO, triangleVAO, triangleEBO;
GLuint meshVBO, meshVAO, meshEBO, meshTriangleVAO, meshTriangleEBO;

bool showBezier = false, pressed = false; // for change mesh 

int main() {

	if (!glfwInit()) {
		cout << "Failed to initialize GLFW" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Test OpenGL", NULL, NULL);
	if (window == NULL) {
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGL(glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	ShaderProgram shaderProgram("vertexShader.glsl", "fragmentShader.glsl");
	ShaderProgram bezierShader("bezierVertexShader.glsl", "bezierFragmentShader.glsl");

	generateLines();
	generateTriangles();
	generateMeshVertices();
	generateMeshLines();

	createVAO();

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {

		if (editMode)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		render(glfwGetTime(), shaderProgram, bezierShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &lineVAO);
	glDeleteVertexArrays(1, &triangleVAO);
	glDeleteBuffers(1, &lineVBO);
	glDeleteBuffers(1, &lineEBO);
	glDeleteBuffers(1, &triangleEBO);

	glDeleteVertexArrays(1, &meshVAO);
	glDeleteVertexArrays(1, &meshTriangleVAO);
	glDeleteBuffers(1, &meshVBO);
	glDeleteBuffers(1, &meshEBO);
	glDeleteBuffers(1, &meshTriangleEBO);

	glfwTerminate();
	return 0;
}

void frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	static bool waspressedKeyE = false;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !editMode)
		pressed = true;

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && pressed == true && !editMode) {
		showBezier = !showBezier;
		pressed = false;
	}
	bool pressedKeyE = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
	if (pressedKeyE && !waspressedKeyE && !showBezier) {
		editMode = !editMode;
		firstMouse = true;
	}
	waspressedKeyE = pressedKeyE;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !editMode)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !editMode)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !editMode)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !editMode)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && editMode && !showBezier) {
		mouseLeftButtonPressed = true;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && mouseLeftButtonPressed && !showBezier) {
		mouseLeftButtonPressed = false;
		generateMeshVertices();
		generateMeshLines();
		createVAO();
	}
}

void render(double time, ShaderProgram& shaderProgram, ShaderProgram& bezierProgram) {
	glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPointSize(pointSize);
	glLineWidth(lineSize);

	if (!showBezier)
		renderMesh(shaderProgram);
	else
		renderBezierSurface(bezierProgram);

	glBindVertexArray(0);
}

void renderMesh(ShaderProgram& shaderProgram) {
	setShaderMatrices(shaderProgram);

	GLint colorLoc = glGetUniformLocation(shaderProgram.ID, "ourColor");

	glBindVertexArray(triangleVAO);
	glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
	glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(lineVAO);
	glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
	glDrawElements(GL_LINES, lines.size(), GL_UNSIGNED_INT, 0);
	glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);
	glDrawArrays(GL_POINTS, 0, n * m);
}

void renderBezierSurface(ShaderProgram& shaderProgram) {
	setShaderMatrices(shaderProgram);

	GLint maxXLoc = glGetUniformLocation(shaderProgram.ID, "maxX");
	GLint maxZLoc = glGetUniformLocation(shaderProgram.ID, "maxZ");
	GLint lineLoc = glGetUniformLocation(shaderProgram.ID, "line");

	glUniform1f(maxXLoc, m - 1);
	glUniform1f(maxZLoc, n - 1);

	glUniform1f(lineLoc, 0.0f);
	glBindVertexArray(meshTriangleVAO);
	glDrawElements(GL_TRIANGLES, meshTriangles.size(), GL_UNSIGNED_INT, 0);
	glUniform1f(lineLoc, 1.0f);
	glBindVertexArray(meshVAO);
	glDrawElements(GL_LINES, meshLines.size(), GL_UNSIGNED_INT, 0);
}

void setShaderMatrices(ShaderProgram& shaderProgram) {
	shaderProgram.use();
	GLint modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
	GLint projectionLoc = glGetUniformLocation(shaderProgram.ID, "projection");

	model = glm::mat4(1.0f);
	view = camera.GetViewMatrix();
	projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void generateLines() {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			if (i < n - 1) {
				lines.push_back(i * m + j);
				lines.push_back((i + 1) * m + j);
			}
			if (j < m - 1) {
				lines.push_back(i * m + j);
				lines.push_back(i * m + j + 1);
			}
		}
	}
}

void generateTriangles() {
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < m - 1; j++) {
			if (max(controlPoints[i + 1][j].y, controlPoints[i + 1][j + 1].y) == controlPoints[i + 1][j + 1].y &&
				max(controlPoints[i][j + 1].y, controlPoints[i + 1][j + 1].y) == controlPoints[i + 1][j + 1].y) {
				triangles.push_back(i * m + j);
				triangles.push_back((i + 1) * m + j);
				triangles.push_back(i * m + j + 1);

				triangles.push_back(i * m + j + 1);
				triangles.push_back((i + 1) * m + j + 1);
				triangles.push_back((i + 1) * m + j);
			}
			else {
				triangles.push_back(i * m + j);
				triangles.push_back((i + 1) * m + j);
				triangles.push_back((i + 1) * m + j + 1);

				triangles.push_back(i * m + j);
				triangles.push_back((i + 1) * m + j + 1);
				triangles.push_back(i * m + j + 1);
			}
		}
	}
}

void generateMeshVertices() {
	for (int i = 0; i < resolution; i++) {
		float u = (float)i / resolution;
		for (int j = 0; j < resolution; j++) {
			float v = (float)j / resolution;
			meshVertices[i][j] = bezierSurface(u, v);
		}
	}
}

void generateMeshLines() {
	for (int i = 0; i < resolution; i++) {
		for (int j = 0; j < resolution; j++) {
			if (i < resolution - 1) {
				meshLines.push_back(i * resolution + j);
				meshLines.push_back((i + 1) * resolution + j);
			}
			if (j < resolution - 1) {
				meshLines.push_back(i * resolution + j);
				meshLines.push_back(i * resolution + j + 1);
			}
			if (i < resolution - 1 && j < resolution - 1) {
				meshTriangles.push_back(i * resolution + j);
				meshTriangles.push_back((i + 1) * resolution + j);
				meshTriangles.push_back(i * resolution + j + 1);

				meshTriangles.push_back(i * resolution + j + 1);
				meshTriangles.push_back((i + 1) * resolution + j + 1);
				meshTriangles.push_back((i + 1) * resolution + j);
			}
		}
	}
}

glm::vec3 bezierSurface(float u, float v) {
	glm::vec3 result = glm::vec3(0.0f);
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			result += controlPoints[i][j] * B(n - 1, i, u) * B(m - 1, j, v);
		}
	}
	return result;
}

float B(int n, int k, float u) {
	int C = binomialCoefficient(n, k);
	return C * pow(u, k) * pow(1 - u, n - k);
}

int binomialCoefficient(int n, int k) {
	if (k > n) return 0;
	if (k == 0 || k == n) return 1;

	int result = 1;
	for (int i = 1; i <= k; ++i) {
		result *= (n - i + 1);
		result /= i;
	}
	return result;
}

void createVAO() {
	// VAO for mesh with points
	glGenVertexArrays(1, &lineVAO);
	glGenBuffers(1, &lineVBO);
	glGenBuffers(1, &lineEBO);
	glBindVertexArray(lineVAO);

	// Lines and points
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(controlPoints), controlPoints, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, lines.size() * sizeof(unsigned int), lines.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Triangles
	glGenVertexArrays(1, &triangleVAO);
	glGenBuffers(1, &triangleEBO);
	glBindVertexArray(triangleVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(unsigned int), triangles.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// VAO form bezier surface
	glGenVertexArrays(1, &meshVAO);
	glGenBuffers(1, &meshVBO);
	glGenBuffers(1, &meshEBO);
	glBindVertexArray(meshVAO);

	// Lines
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(meshVertices), meshVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshLines.size() * sizeof(unsigned int), meshLines.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Triangles
	glGenVertexArrays(1, &meshTriangleVAO);
	glGenBuffers(1, &meshTriangleEBO);
	glBindVertexArray(meshTriangleVAO);

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshTriangleEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshTriangles.size() * sizeof(unsigned int), meshTriangles.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	static int selectedPointIndexI = -1, selectedPointIndexJ = -1;
	static glm::vec3 initialPlaneIntersection;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (editMode && !showBezier) {
		if (mouseLeftButtonPressed) {
			glm::vec3 rayOrigin = camera.position;
			glm::vec3 rayDir = calculateRayFromMouse(xpos, ypos, view, projection);

			if (selectedPointIndexI == -1 && selectedPointIndexJ == -1) {
				for (int i = 0; i < n; i++) {
					for (int j = 0; j < m; j++) {
						if (isPointIntersected(rayOrigin, rayDir, controlPoints[i][j], 0.2f)) {
							selectedPointIndexI = i;
							selectedPointIndexJ = j;

							glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
							glm::vec3 planePoint = controlPoints[selectedPointIndexI][selectedPointIndexJ];
							float t = glm::dot(planePoint - rayOrigin, planeNormal) / glm::dot(rayDir, planeNormal);
							initialPlaneIntersection = rayOrigin + t * rayDir;
							break;
						}
					}
					if (selectedPointIndexI != -1) break;
				}
			}
			else {
				glm::vec3 planeNormal(1.0f, 0.0f, 0.0f);
				glm::vec3 planePoint = initialPlaneIntersection;
				float t = glm::dot(planePoint - rayOrigin, planeNormal) / glm::dot(rayDir, planeNormal);
				glm::vec3 intersection = rayOrigin + t * rayDir;

				if (selectedPointIndexI != -1 && selectedPointIndexJ != -1) {
					glm::vec3 offset = intersection - initialPlaneIntersection;

					offset.x = 0.0f;
					offset.z = 0.0f;

					controlPoints[selectedPointIndexI][selectedPointIndexJ] += offset;
					initialPlaneIntersection += offset;

					glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
					glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(controlPoints), controlPoints);
				}
			}
		}
		else {
			selectedPointIndexI = -1;
			selectedPointIndexJ = -1;
		}
	}
	else {
		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!editMode) camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

glm::vec3 calculateRayFromMouse(double xpos, double ypos, glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
	float x = (2.0f * xpos) / SCR_WIDTH - 1.0f;
	float y = 1.0f - (2.0f * ypos) / SCR_HEIGHT;
	float z = 1.0f;

	glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
	glm::vec4 rayEye = glm::inverse(projectionMatrix) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
	glm::vec3 rayWorld = glm::vec3(glm::inverse(viewMatrix) * rayEye);
	rayWorld = glm::normalize(rayWorld);
	return rayWorld;
}

bool isPointIntersected(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& point, float threshold) {
	glm::vec3 toPoint = point - rayOrigin;
	float t = glm::dot(toPoint, rayDir);
	glm::vec3 closestPoint = rayOrigin + t * rayDir;

	return glm::length(closestPoint - point) < threshold;
}
