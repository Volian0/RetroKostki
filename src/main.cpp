#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <learnopengl/filesystem.h>
#include <C:\LearnOpenGL-master\includes\learnopengl\shader_m.h>

#include <soloud.h>
#include <soloud_wav.h>
#include <optional>

#include <iostream>
#include <algorithm>
#include "windows.h"
#include "texture.h"
#include "player.h"
#include "cube.h"
#include "minesweeper.h"
#include "fade.h"
//#include "audio.h"

SoLoud::Soloud soloud;
SoLoud::Wav bg_music;
SoLoud::Wav bg_music2;
SoLoud::Wav sfx_jump;
SoLoud::Wav sfx_land;

SoLoud::Wav sfx_pop;
SoLoud::Wav sfx_boom;

Player player;
std::vector<Cube> cubes;

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 0;
float lastY = 0;
float fov = 90;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

uint8_t yacieowo = 0;

bool motion = true;

extern int ms_offset_t;

SoLoud::handle bg_music2i;

void teleport_camera(glm::vec3 position, float yaw_, bool lock)
{
	motion = !lock;
	player.pos = position;
	player.on_ground = false;
	player.y_velocity = 0.0;
	pitch = 0.0;
	yaw = yaw_;
	//roll = 0.0;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

uint8_t advtext = 0;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	static bool was_lb_pressed = false;
	bool left_button_pressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	if (advtext < 4 && !was_lb_pressed && left_button_pressed)
	{
		++advtext;
		if (advtext == 4)
		{
			soloud.play(bg_music);
			teleport_camera({ -1, 1.0, 10 }, -90, false);
			fade::fade({ 0,0,0 }, 3);
		}
		else
		{
			teleport_camera(glm::vec3{ 300 + float(advtext) * 10.0,300,-49 }, -90, true);
			soloud.play(sfx_jump);
		}
	}

	was_lb_pressed = left_button_pressed;

	if (!motion)
		return;

	//float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		player.move_forward(deltaTime, 0, cubes);
		//cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		player.move_forward(deltaTime, 180, cubes);
		//cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		player.move_forward(deltaTime, -90, cubes);
		//cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		player.move_forward(deltaTime, 90, cubes);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		player.jump();
		//cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

float roll = 0;
float sensitivity = 0.5f;
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	sensitivity = std::clamp<>(sensitivity + yoffset*0.01, 0.01, 4.0);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{


	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}



	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	if (!motion)
		return;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	auto monitor = glfwGetPrimaryMonitor();
	auto video_mode = glfwGetVideoMode(monitor);
	SCR_WIDTH = video_mode->width;
	SCR_HEIGHT = video_mode->height;
	GLFWwindow* window = glfwCreateWindow(video_mode->width, video_mode->height, "RetroKostki", monitor, 0);
	assert(window);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		abort();

	// build and compile our shader zprogram
	// ------------------------------------
	Shader ourShader("res/shaders/basic.vs", "res/shaders/basic.fs");
	Shader ourShader2("res/shaders/screen.vs", "res/shaders/screen.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	// world space positions of our cubes
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use();
	ourShader.setInt("texture1", 0);

	ourShader2.use();
	ourShader2.setInt("screenTexture", 0);


	float cubeVertices[] = {
		// positions          // texture Coords
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	float planeVertices[] = {
		// positions          // texture Coords 
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};
	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	// plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	// screen quad VAO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


	// framebuffer configuration
	// -------------------------
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Frameb";

	// render loop
	// -----------
	//Texture texture("res/textures/brick.png");
	Texture texture("res/textures/block.png");
	Texture texture2("res/textures/block2.png");
	Texture texture4("res/textures/block4.png");


	Texture text1("res/textures/text1.png");
	Texture text2("res/textures/text2.png");
	Texture text3("res/textures/text3.png");
	Texture text4("res/textures/text4.png");

	cubes.push_back(Cube{ glm::vec3{300,300,-50}, &text1 });
	cubes.push_back(Cube{ glm::vec3{310,300,-50}, &text2 });
	cubes.push_back(Cube{ glm::vec3{320,300,-50}, &text3 });
	cubes.push_back(Cube{ glm::vec3{330,300,-50}, &text4 });

	teleport_camera(glm::vec3{ 300,300,-49 }, -90, true);
	
	//for (int x = 0; x < 10; ++x)
		//for (int y = 0; y < 10; ++y)
			//cubes.push_back(Cube{ glm::vec3{x,-1,y}, &texture });

	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			cubes.push_back(Cube{ glm::vec3{x,10,y}, &texture });


	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			cubes.push_back(Cube{ glm::vec3{-1,x,y}, &texture });

	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			cubes.push_back(Cube{ glm::vec3{10,x,y}, &texture });

	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			cubes.push_back(Cube{ glm::vec3{x,y,-1}, &texture });

	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
		{
			if (((x < 6 || x > 8) || (y < 6 || y > 8)) && (x != 1 || y > 2))
			cubes.push_back(Cube{ glm::vec3{x,y,10}, &texture });
		}

	cubes.push_back(Cube{ glm::vec3{1,-1,10}, &texture });

	cubes.push_back(Cube{ glm::vec3{1,-1,11}, &texture });
	cubes.push_back(Cube{ glm::vec3{0,-1,11}, &texture });
	cubes.push_back(Cube{ glm::vec3{-1,-1,11}, &texture });
	cubes.push_back(Cube{ glm::vec3{-1,-1,10}, &texture });

	cubes.push_back(Cube{ glm::vec3{1,3,11}, &texture });
	cubes.push_back(Cube{ glm::vec3{0,3,11}, &texture });
	cubes.push_back(Cube{ glm::vec3{-1,3,11}, &texture });
	cubes.push_back(Cube{ glm::vec3{-1,3,10}, &texture });

	for (int z = 0; z < 3; ++z)
	{
		cubes.push_back(Cube{ glm::vec3{-2,z,10}, &texture });
		cubes.push_back(Cube{ glm::vec3{-2,z,11}, &texture });

		cubes.push_back(Cube{ glm::vec3{2,z,11}, &texture });
		cubes.push_back(Cube{ glm::vec3{1,z,12}, &texture });
		cubes.push_back(Cube{ glm::vec3{0,z,12}, &texture });
		cubes.push_back(Cube{ glm::vec3{-1,z,12}, &texture });
	}

	cubes.push_back(Cube{ glm::vec3{0,0,0}, &texture });
	cubes.push_back(Cube{ glm::vec3{1,0,0}, &texture });
	cubes.push_back(Cube{ glm::vec3{2,0,0}, &texture });
	cubes.push_back(Cube{ glm::vec3{0,0,1}, &texture });
	cubes.push_back(Cube{ glm::vec3{1,0,1}, &texture });
	cubes.push_back(Cube{ glm::vec3{2,0,1}, &texture });
	cubes.push_back(Cube{ glm::vec3{0,0,2}, &texture });
	cubes.push_back(Cube{ glm::vec3{1,0,2}, &texture });
	cubes.push_back(Cube{ glm::vec3{2,0,2}, &texture });

	cubes.push_back(Cube{ glm::vec3{5,0,1}, &texture });
	cubes.push_back(Cube{ glm::vec3{5,1,1}, &texture });

	cubes.push_back(Cube{ glm::vec3{8,2,0}, &texture });
	cubes.push_back(Cube{ glm::vec3{9,2,0}, &texture });
	cubes.push_back(Cube{ glm::vec3{9,2,1}, &texture });

	cubes.push_back(Cube{ glm::vec3{8,1,0}, &texture });
	//cubes.push_back(Cube{ glm::vec3{9,1,0}, &texture });
	cubes.push_back(Cube{ glm::vec3{9,1,1}, &texture });

	cubes.push_back(Cube{ glm::vec3{8,0,0}, &texture });
	//cubes.push_back(Cube{ glm::vec3{9,0,0}, &texture });
	cubes.push_back(Cube{ glm::vec3{9,0,1}, &texture });


	cubes.push_back(Cube{ glm::vec3{9,3,4}, &texture });
	//cubes.push_back(Cube{ glm::vec3{9,2,8}, &texture });
	cubes.push_back(Cube{ glm::vec3{7,2,7}, &texture });
	cubes.push_back(Cube{ glm::vec3{7,1,7}, &texture });
	cubes.push_back(Cube{ glm::vec3{7,0,7}, &texture });

	cubes.push_back(Cube{ glm::vec3{7,3,8}, &texture });
	cubes.push_back(Cube{ glm::vec3{7,4,8}, &texture });

	cubes.push_back(Cube{ glm::vec3{6,4,8}, &texture });
	cubes.push_back(Cube{ glm::vec3{6,4,7}, &texture });

	cubes.push_back(Cube{ glm::vec3{7,2,8}, &texture });

	cubes.push_back(Cube{ glm::vec3{8,1,8}, &texture });
	cubes.push_back(Cube{ glm::vec3{8,1,9}, &texture });
	cubes.push_back(Cube{ glm::vec3{9,1,8}, &texture });
	cubes.push_back(Cube{ glm::vec3{9,1,9}, &texture });
	cubes.push_back(Cube{ glm::vec3{7,1,8}, &texture });
	cubes.push_back(Cube{ glm::vec3{7,1,9}, &texture });

	cubes.push_back(Cube{ glm::vec3{4, 2, 9}, &texture });
	cubes.push_back(Cube{ glm::vec3{4, 1, 9}, &texture });

	cubes.push_back(Cube{ glm::vec3{1, 3, 8}, &texture });
	cubes.push_back(Cube{ glm::vec3{1, 2, 8}, &texture });
	cubes.push_back(Cube{ glm::vec3{1, 1, 8}, &texture });
	cubes.push_back(Cube{ glm::vec3{1, 0, 8}, &texture });


	cubes.push_back(Cube{ glm::vec3{0, 4, 5}, &texture });

	cubes.push_back(Cube{ glm::vec3{3, 5, 4}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 4, 4}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 3, 4}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 2, 4}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 1, 4}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 0, 4}, &texture });

	cubes.push_back(Cube{ glm::vec3{4, 5, 1}, &texture });

	cubes.push_back(Cube{ glm::vec3{3, 5, 1}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 6, 1}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 7, 1}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 8, 1}, &texture });
	cubes.push_back(Cube{ glm::vec3{3, 9, 1}, &texture });

	cubes.push_back(Cube{ glm::vec3{7, 5, 4}, &texture });
	cubes.push_back(Cube{ glm::vec3{7, 5, 3}, &texture });
	cubes.push_back(Cube{ glm::vec3{7, 4, 3}, &texture });
	cubes.push_back(Cube{ glm::vec3{7, 3, 3}, &texture });
	cubes.push_back(Cube{ glm::vec3{7, 2, 3}, &texture });
	cubes.push_back(Cube{ glm::vec3{7, 1, 3}, &texture });
	cubes.push_back(Cube{ glm::vec3{7, 0, 3}, &texture });
	//////////////////////
	//PURPLE CUBES

	Texture texture3("res/textures/block3.png");

	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
		{
			if (x == 0 || y == 0 || x == 9 || y == 9)
			{
				cubes.push_back(Cube{ glm::vec3{x,-1,double(y) - 1000.0}, &texture3 });
			}
		}

	ms_offset_t = cubes.size();

	for (int x = 1; x < 9; ++x)
		for (int y = 1; y < 9; ++y)
		{
			
			{
				cubes.push_back(Cube{ glm::vec3{x,-1,double(y) - 1000.0}, &texture3 });
			}
		}

	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			cubes.push_back(Cube{ glm::vec3{x,5,double(y)-1000.0}, &texture3 });


	for (int x = 0; x < 5; ++x)
		for (int y = 0; y < 10; ++y)
		{
			if (x != 0 && x != 4)
			{
				if (y != 0 && y != 3 && y != 6 && y != 9)
				{
					//cubes.push_back(Cube{ glm::vec3{-2,x,double(y) - 1000.0}, &texture4 });
					continue;
				}
			}
			cubes.push_back(Cube{ glm::vec3{-1,x,double(y) - 1000.0}, &texture3 });
		}



	for (int x = 0; x < 5; ++x)
		for (int y = 0; y < 10; ++y)
		{
			if (x != 0 && x != 4)
			{
				if (y != 0 && y != 3 && y != 6 && y != 9)
				{
					//cubes.push_back(Cube{ glm::vec3{11,x,double(y) - 1000.0}, &texture4 });
					continue;
				}
			}
			cubes.push_back(Cube{ glm::vec3{10,x,double(y) - 1000.0}, &texture3 });
		}



	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 5; ++y)
		{
			if (y == 3)
			{
				if (x == 4 || x == 5)
					continue;
			}
			else if (y < 3)
			{
				if (x >= 3 && x <= 6)
					continue;
			}
			cubes.push_back(Cube{ glm::vec3{x,y,-1.0 - 1000.0}, &texture3 });
		}

	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 5; ++y)
		{
			if (y == 3)
			{
				if (x == 4 || x == 5)
					continue;
			}
			else if (y < 3)
			{
				if (x >= 3 && x <= 6)
					continue;
			}
			cubes.push_back(Cube{ glm::vec3{x,y,10.0-1000.0}, &texture3 });
		}

	auto z_z_index = cubes.size();

	for (long x = 0; x < 2; ++x)
	{
		for (long y = 0; y < 10; ++y)
		{
			//floor
			cubes.push_back(Cube{ glm::vec3{-2-x,0,5 + y - 1000.0}, &texture3 });

			//ceiling
			if (y == 9-0 || y == 9-3 || y == 9-5)
			{
				cubes.push_back(Cube{ glm::vec3{-2 - x,4,5 + y - 1000.0}, &texture3 });
			}
			else cubes.push_back(Cube{ glm::vec3{-2 - x,5,5 + y - 1000.0}, &texture3 });

			if (y < 2 && x == 0)
			{
				cubes.push_back(Cube{ glm::vec3{-4,0,5 + y - 1000.0}, &texture3 });
				cubes.push_back(Cube{ glm::vec3{-4,5,5 + y - 1000.0}, &texture3 });
			}
		}
	}

	cubes.push_back(Cube{ glm::vec3{-5,1,5 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-6,2,5 - 1000.0}, &texture4 });
	cubes.push_back(Cube{ glm::vec3{-6,3,5 - 1000.0}, &texture4 });
	cubes.push_back(Cube{ glm::vec3{-5,4,5 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-5,1,6 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-5,2,6 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-5,3,6 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-5,4,6 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-4,1,7 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,2,7 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,3,7 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,4,7 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,1,8 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,2,8 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,3,8 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,4,8 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-4,1,9 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,2,9 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,3,9 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-5,1,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-5,2,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-5,3,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,4,10 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-4,0,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,0,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,4,10 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-4,1,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,2,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,3,11 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-4,1,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,2,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,3,14 - 1000.0}, &texture3 });
	//
	cubes.push_back(Cube{ glm::vec3{-3,3,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-2,3,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-3,2,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-2,2,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-3,1,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-2,1,15 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-1,3,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,2,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,1,14 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-1,3,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,2,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,1,11 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-1,0,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,0,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,0,13 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,0,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,4,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,4,13 - 1000.0}, &texture3 });
	//
	cubes.push_back(Cube{ glm::vec3{-4,1,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,2,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,3,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,4,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,1,13 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,2,13 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,3,13 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-4,4,13 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{-1,4,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{-1,4,11 - 1000.0}, &texture3 });

	for (int x = 0; x < 2; ++x)
	{
		cubes.push_back(Cube{ glm::vec3{0,6*x-1,14 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{0,6*x-1,13 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{0,6*x-1,12 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{0,6*x-1,11 - 1000.0}, &texture3 });

		cubes.push_back(Cube{ glm::vec3{2,6*x-1,14 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{2,6*x-1,13 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{2,6*x-1,12 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{2,6*x-1,11 - 1000.0}, &texture3 });

		cubes.push_back(Cube{ glm::vec3{1,7*x-1,14 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{1,7*x-1,13 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{1,7*x-1,12 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{1,7*x-1,11 - 1000.0}, &texture3 });
	}
	
	cubes.push_back(Cube{ glm::vec3{3,-1,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,-1,13 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{3,-1,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,-1,16 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{4,-1,17 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,-1,16 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,-1,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,-1,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,-1,13 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,-1,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,-1,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,-1,10 - 1000.0}, &texture3 });


	cubes.push_back(Cube{ glm::vec3{3,-1,10 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{3,3,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,3,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,3,13 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,3,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,4,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,4,12 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,4,13 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,4,14 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{3,2,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,2,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,1,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,1,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,0,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,0,14 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{4,3,11 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,3,14 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,3,17 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{4,4,16 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,4,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,4,13 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,4,12 - 1000.0}, &texture3 });

	for (int i = 0; i < 5; ++i)
	{
		cubes.push_back(Cube{ glm::vec3{0,i,15 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{1,i,15 - 1000.0}, &texture3 });
		cubes.push_back(Cube{ glm::vec3{2,i,15 - 1000.0}, &texture3 });
	}
	cubes.push_back(Cube{ glm::vec3{1,5,15 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{1,5,10 - 1000.0}, &texture3 });



	cubes.push_back(Cube{ glm::vec3{3,0,17 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,1,17 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,2,17 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{3,3,16 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{3,3,15 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{2,0,16 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{2,1,16 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{2,2,16 - 1000.0}, &texture3 });

	auto z_z_index2 = cubes.size();

	for (int i = z_z_index; i < z_z_index2; ++i)
	{
		auto& cube = cubes[i];
		cubes.push_back(Cube{ glm::vec3{cube.pos.x,cube.pos.y,-996 - (995 + cube.pos.z)}, cube.texture });
	}
	z_z_index2 = cubes.size();
	for (int i = z_z_index; i < z_z_index2; ++i)
	{
		auto& cube = cubes[i];
		cubes.push_back(Cube{ glm::vec3{cube.pos.x * (-1) + 9,cube.pos.y,cube.pos.z}, cube.texture });
	}
	////////////////////////

	cubes.push_back(Cube{ glm::vec3{4,0,18 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,1,18 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,2,18 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,0,18 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,1,18 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,2,18 - 1000.0}, &texture3 });

	//fake blockade
	auto fake_blockade_index = cubes.size();
	cubes.push_back(Cube{ glm::vec3{4,0,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,0,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,1,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,1,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,2,10 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,2,10 - 1000.0}, &texture3 });
	//

	//mineblocker
	auto mineblocker_index = cubes.size();
	cubes.push_back(Cube{ glm::vec3{4,0,-4 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,0,-4 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,1,-4 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,1,-4 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,2,-4 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,2,-4 - 1000.0}, &texture3 });

	cubes.push_back(Cube{ glm::vec3{4,0,-3 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,0,-3 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,1,-3 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,1,-3 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{4,2,-3 - 1000.0}, &texture3 });
	cubes.push_back(Cube{ glm::vec3{5,2,-3 - 1000.0}, &texture3 });
	//
	auto mineblocker_end_index = cubes.size();

	//bridge
	for (int z = 0; z < 40; ++z)
	{
		cubes.push_back(Cube{ glm::vec3{7, 5, 11+z}, &texture });
		cubes.push_back(Cube{ glm::vec3{8, 5, 11+z}, &texture });
		cubes.push_back(Cube{ glm::vec3{6, 5, 11+z}, &texture });

		cubes.push_back(Cube{ glm::vec3{5, 7, 11+z}, &texture });
		cubes.push_back(Cube{ glm::vec3{9, 7, 11+z}, &texture });
	}
	
	auto normal_size = cubes.size();

	for (int ix = 7 - 11; ix <= 7 + 11; ++ix)
		for (int iy = 7 - 11; iy <= 7 + 11; ++iy)
			for (int iz = 11 - 11; iz <= 11+39 + 11; ++iz)
		{
				if (rand() % 16 != 0)
					continue;
				glm::vec3 my_new_pos = { ix, iy, iz };
				bool should_spawn = true;
				for (int i = 0; i < normal_size; ++i)
				{
					if (glm::distance(my_new_pos, cubes[i].pos) < 5.0)
					{
						should_spawn = false;
						break;
					}
				}
				for (int i = normal_size; i < cubes.size(); ++i)
				{
					if (glm::distance(my_new_pos, cubes[i].pos) < 3.0)
					{
						should_spawn = false;
						break;
					}
				}
				if (should_spawn)
					cubes.push_back(Cube{ my_new_pos, &texture });
		}
	
	soloud.init();

	
	bg_music.load("res/sounds/volian_office_ambience.mp3");
	bg_music.setLooping(true);
	bg_music.setLoopPoint(SoLoud::time(2.6797052154195011337868480725624L));

	bg_music2.load("res/sounds/starry_dream2.mp3");
	bg_music2.setLooping(true);
	bg_music2.setVolume(1.25);

	sfx_jump.load("res/sounds/b1.wav");
	sfx_jump.setVolume(2.0);
	sfx_land.load("res/sounds/b0.wav");
	sfx_land.setVolume(1.0);

	sfx_pop.load("res/sounds/text.wav");
	sfx_pop.setVolume(2.0);
	sfx_boom.load("res/sounds/exp.wav");
	sfx_boom.setVolume(2.0);

	lastFrame = glfwGetTime();
	ourShader.use();
	ourShader.setVec3("afog_color", { 0,0,0 });
	ourShader.setFloat("afog_farz", 10);
	srand(time(0));
	while (!glfwWindowShouldClose(window))
	{
	
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = std::min<>(double(currentFrame - lastFrame), 1.0/30.0) * 1.125;
		lastFrame = currentFrame;

		roll = std::max<>((player.pos.z - 15.0) * std::abs(player.pos.z / 10.0) * std::abs(player.pos.z / 15.0) * std::abs(player.pos.z / 15.0), 0.0);

		for (auto& cube : cubes)
		{
			if (cube.texture != &texture2 && cube.texture != &texture)
			{
				continue;
			}
			if (unsigned(cube.pos.x + cube.pos.y + cube.pos.z) % 4 == yacieowo % 4)
			{
				cube.texture = &texture2;
			}
			else
			{
				cube.texture = &texture;
			}
		}

		// input
		// -----
		processInput(window);
		player.gravity(deltaTime, cubes);
		cameraPos = player.pos;
		static bool transition = false;
		static std::optional<Player> player_save;
		if (player.pos.y < -50)
		{
			if (transition) abort();
			transition = true;

			bg_music.stop();
			bg_music2i = soloud.play(bg_music2);

			ourShader.use();
			ourShader.setVec3("afog_color", glm::vec3{ 0,0.2,0.4 });
			ourShader.setFloat("afog_farz", 15);
			teleport_camera(glm::vec3{ 4.5,1,15.5 - 1000.0 }, -90, false);
			player_save.reset();
			minesweeper::hard_reset();

			fade::fade({ 1,1,1 }, 5);
		}

		static bool blockade_moved = false;
		if (!blockade_moved && player.pos.z < -991)
		{
			blockade_moved = true;
			for (auto i = fake_blockade_index; i < mineblocker_index; ++i)
			{
				auto& cube_pos = cubes[i].pos;
				cube_pos.z = 15 - 1000;
			}
		}
		static bool won_minesweeper = false;
		if (!won_minesweeper && minesweeper::winner)
		{
			won_minesweeper = true;
			for (auto i = mineblocker_index; i < mineblocker_end_index; ++i)
			{
				cubes.erase(cubes.begin() + mineblocker_index);
			}
		}

		if (motion)
		{
			
			static float my_yaw;
			static float my_pitch;
			if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS)
			{
				player_save = player;
				my_yaw = yaw;
				my_pitch = pitch;
			}
			if (glfwGetKey(window, GLFW_KEY_F7) == GLFW_PRESS)
			{
				if (player_save)
				{
					player = *player_save;
					yaw = my_yaw;
					pitch = my_pitch;

					glm::vec3 front;
					front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
					front.y = sin(glm::radians(pitch));
					front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
					cameraFront = glm::normalize(front);
				}
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST); // enable depth testing (is disable

		// render
		// ------

		//glBindVertexArray(VAO);

		//glBindBuffer(GL_ARRAY_BUFFER, VBO);
		if (transition)
		{
			glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
		}
		else
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		texture.use();

		// activate shader
		ourShader.use();

		// pass projection matrix to shader (note that in this case it could change every frame)
		glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, transition ? 15.0f : 10.0f);
		ourShader.setMat4("projection", projection);

		// camera/view transformation

		glm::mat4 roll_mat = glm::rotate(glm::mat4(1.0f), glm::radians(roll), cameraFront);
		auto zcameraUp = glm::mat3(roll_mat) * glm::vec3(0.0f, 1.0f, 0.0f);
		

		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, zcameraUp);
		ourShader.setMat4("view", view);

		// render boxes
		glBindVertexArray(VAO);
		for (auto& cube : cubes)
		{
			cube.anim_tp = std::max<>(cube.anim_tp - deltaTime, 0.0);
			if (glm::distance(cube.pos,player.pos) <= 20.0)
				cube.render(&ourShader);
		}

		// second pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ourShader2.use();
		fade::update(deltaTime, &ourShader2);
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

#include <Windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int lCmdShow)
{
	return main();
}