
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <assert.h>
using namespace std;
using namespace glm;

#define PI 3.14159

GLFWwindow *window;
int w, h;
double mouseX, mouseY;

vector<vec2> controls = {vec2(-0.5f, -0.25f), vec2(0.f, -0.25f), vec2(0.f, 0.25f), vec2(0.5f, 0.25f)};
float cRadius = 0.01f;
int selected = -1;

vector<float> knots = {0.f, 0.f, 1.f, 1.f};
int order = 2;
int uParam = 0;

void render () {
	glEnable (GL_DEPTH_TEST);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Functions for changing transformation matrix
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glTranslatef (0.f, 0.f, 0.f);
	glRotatef (0.f, 0.f, 0.f, 1.f);
	glScalef (1.f, 1.f, 1.f);

	//Functions for changing projection matrix
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (-1, 1, -1, 1, -1, 1);
	//gluPerspective (fov, aspect ratio, near plane, far plane)
	//glFrustum

	//Draws the points as little circles
	for (int i = 0; i < controls.size(); i++){
		glBegin (GL_TRIANGLE_STRIP); //GL_LINE_STRIP, GL_POINTS, GL_QUADS, etc...
			for (float t = 0.f; t < 2*PI; t += 0.01f){
				vec2 circle = vec2(cRadius*cos(t), cRadius*sin(t));
				glVertex2f(controls[i].x + circle.x, controls[i].y + circle.y);
				glVertex2f(controls[i].x, controls[i].y);
			}
		glEnd ();
	}
}

void buildKnots(){
	int numPoints = controls.size();
	/*	the denominator for the uniform step size is (numPoints - order + 2).
		if numPoints < order - 1 , then step size will be negative or have denominator 0.
		to avoid this we assert that numPoints > order - 2 */
	assert(numPoints > (order - 2));

	//add knots from current m and k values
	float stepSize = 1.f / float(numPoints - order + 2);
	for (int iter = order; iter <= numPoints; iter++){
		float value = knots[iter-1] + stepSize;
		knots.insert(knots.begin() + iter, value);
	}

	//delete knots from previous knot sequence
	while (knots.size() > (numPoints + order + 1)){
		knots.erase(knots.begin() + (numPoints + 1));
	}

	/* prints out the knot values
	for (int iter = 0; iter < knots.size(); iter++){
		cout << knots[iter] << " ";
	}
	cout << endl;
	*/
}

void keyboard (GLFWwindow *sender, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_UP && action == GLFW_PRESS){
		if (controls.size() > (order - 2) + 1){
			order++;
			knots.insert(knots.begin(), 0.f);
			knots.push_back(1.f);
			buildKnots();
		}
		else {
			cout << "The order of the curve is too big for the number of control points." << endl;
		}
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
		if (order > 1){
			order--;
			knots.erase(knots.begin());
			knots.pop_back();
			buildKnots();
		}
		else {
			cout << "The order of the curve must be bigger than 0." << endl;
		}
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
		if (uParam < 1.f){
			uParam++;
		}
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
		if (uParam > 0.f){
			uParam--;
		}
	}
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS){
			cout << "Order: " << order << endl;
			cout << "Number of Points: " << controls.size() << endl;
	}
}

bool canMove = false;

void mouseClick (GLFWwindow *sender, int button, int action, int mods) {
	selected = -1;
	canMove = false;
	if (action == GLFW_PRESS){
		for (int i = 0; i < controls.size(); i++){
			if ((abs(controls[i].x - mouseX) <= cRadius) && (abs(controls[i].y - mouseY) <= cRadius)){
				selected = i;
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (selected == -1){
				controls.push_back(vec2(mouseX, mouseY));
				cout << "New control point: " << controls.size() - 1 << endl;
			}
			else{
				cout << "Selected control point: " << selected << endl;
			}
			canMove = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT){
			if (controls.size() - 1 > (order - 2)){
				if (selected != -1){
					controls.erase(controls.begin() + selected);
					cout << "Deleted control point: " << selected << endl;
				}
			}
			else
				cout << "There are too few control points for the current order of the curve." << endl;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			if (selected != -1){
				controls.insert(controls.begin() + selected, vec2(mouseX, mouseY));
				cout << "New control point: " << selected << endl;
			}
			canMove = true;
		}
		buildKnots();
	}
}

void mousePos (GLFWwindow *sender, double x, double y) {
	mouseX = (2 * x / w) - 1;
	mouseY = -((2 * y / h) - 1);
	if (selected != -1 && canMove){
		controls[selected].x = mouseX;
		controls[selected].y = mouseY;
	}
}

int main() {
	if (!glfwInit())
		return 1;

	window = glfwCreateWindow (640, 640, "My Window", NULL, NULL);
	if (!window)
		return 1;

	glfwMakeContextCurrent (window);
	glfwSetKeyCallback (window, keyboard);
	glfwSetMouseButtonCallback (window, mouseClick);
	glfwSetCursorPosCallback (window, mousePos);
	while (!glfwWindowShouldClose (window)) {
		glfwGetFramebufferSize (window, &w, &h);
		glViewport (0, 0, w, h);

		render ();

		glfwSwapBuffers (window);
		glfwPollEvents();
	}

	glfwDestroyWindow (window);
	glfwTerminate();
	return 0;
}

