#define WITH_GLFW
#define WITH_OPENGP
#define WITH_EIGEN

#include "common\icg_common.h"
#include "common\imshow.h"

#include <math.h>
#include <OpenGP/GL/Eigen.h>
#include <OpenGP/GL/check_error_gl.h>

#include "Mesh/mesh.h"

//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include "Skybox/skybox.h"

using namespace OpenGP;

Mesh grid;
Skybox skybox;
typedef Eigen::Transform<float,3,Eigen::Affine> Transform;
int window_width = 1280;
int window_height = 720;
int pxwidth, pxheight;

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 480;

// camera
vec3 cameraPos(0.0f, 0.0f, 3.0f);
vec3 cameraFront(0.0f, 0.0f, -1.0f);
vec3 cameraUp(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = window_width / 2.0;
float lastY = window_height / 2.0;
float fov = 45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float radians(float a) {
	return a * M_PI / 180;
}

void processInput(GLFWwindow *window) {	
	float cameraSpeed = 2.5f * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPos -= (cameraFront.cross(cameraUp)).normalized() * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos += (cameraFront.cross(cameraUp)).normalized() * cameraSpeed;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	//glfwGetCursorPos(window,xpos,ypos);
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = lastX - xpos;
	float yoffset = ypos - lastY; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.5f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	vec3 front;
	front(0) = cos(radians(pitch)) * cos(radians(yaw));
	front(1) = sin(radians(pitch));
	front(2) = cos(radians(pitch)) * sin(radians(yaw));

	cameraFront = front.normalized();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

void display() {
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, pxwidth, pxheight);
    //glViewport(0,0,window_width,window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float ratio = window_width / (float) window_height;
    //Turn the bunny to face the camera
    Transform modelTransform = Transform::Identity();/*
    modelTransform *= Eigen::Translation3f(0, 2.0, 1.0);
    modelTransform *= Eigen::AngleAxisf(M_PI, vec3::UnitZ());
    modelTransform *= Eigen::AngleAxisf(M_PI / 2.0f, vec3::UnitX());*/
    mat4 model = modelTransform.matrix();
    mat4 projection = OpenGP::perspective(fov, ratio, 0.1f, 10.0f);

    //camera movement
    float time = .5 * glfwGetTime();
    //vec3 cam_pos(2*cos(time), 2.0, 2*sin(time));
    //vec3 cam_pos(0, 2, 4);
    //vec3 cam_look(1.5f, 0.2f, 0.0f);
    //vec3 cam_up(0.0f, 1.0f, 0.0f);
    //mat4 view = OpenGP::lookAt(cam_pos, cam_look, cam_up);

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	processInput(window);
	vec3 cameraLook = cameraPos + cameraFront;
	mat4 view = OpenGP::lookAt(cameraPos, cameraLook, cameraUp);

	// rotate the sky
	Transform TRS = Transform::Identity();
	TRS *= Eigen::AlignedScaling3f(2, 2, 2);
	TRS *= Eigen::AngleAxisf(0.3*time, vec3(1,0,0));

	grid.draw(model, view, projection);
	skybox.draw(model, view, projection, TRS.matrix());
}

///
/// You can use this sub-project as a  starting point for your second
/// assignemnt. See the files triangle.h and quad.h for examples of
/// basic OpenGL code.
///

int main(int, char**) {

	/*GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 3", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}*/

    OpenGP::glfwInitWindowSize(640, 480);
    OpenGP::glfwMakeWindow("Assignment 3");
	glfwGetFramebufferSize(window, &pxwidth, &pxheight);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glClearColor(0.0,0.0,0.0,0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    grid.init();
	skybox.init();

	glfwSetCursorPosCallback(OpenGP::window, reinterpret_cast<GLFWcursorposfun>(mouse_callback));
	glfwSetScrollCallback(OpenGP::window, reinterpret_cast<GLFWscrollfun>(scroll_callback));
	glfwSetInputMode(OpenGP::window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    OpenGP::glfwDisplayFunc(&display);

    OpenGP::glfwMainLoop();

    return EXIT_SUCCESS;

}
