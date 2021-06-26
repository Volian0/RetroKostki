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
#include "thread"
#include "chrono"
#include "unordered_map"
#include "sliding.h"
//#include "audio.h"

SoLoud::Soloud soloud;
SoLoud::Wav bg_music;
SoLoud::Wav bg_music2;
SoLoud::Wav bg_music3;
SoLoud::Wav bg_music_crazy;
SoLoud::Wav bg_music_credits;
SoLoud::Wav sfx_jump;
SoLoud::Wav sfx_land;

SoLoud::Wav sfx_pop;
SoLoud::Wav sfx_boom;

SoLoud::Wav voice0;
SoLoud::Wav voice1;
SoLoud::Wav voice2;
SoLoud::Wav voice3;

SoLoud::Wav roll_loop;

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
SoLoud::handle roll_loopi;

extern double player_speed_m;

std::optional<double> credit_progress;



void play_voices()
{
	std::this_thread::sleep_for(std::chrono::seconds(3));
	soloud.play(voice0);
	std::this_thread::sleep_for(std::chrono::seconds(3));
	soloud.play(voice1);
}

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

void okurwa()
{
	soloud.fadeGlobalVolume(0, 3.0);
	std::this_thread::sleep_for(std::chrono::seconds(3));
	soloud.stopAll();
	soloud.setGlobalVolume(1.0);
	soloud.play(bg_music_credits);
	credit_progress = 0.0;
}

void start_credits()
{
	teleport_camera(glm::vec3{ 400,301,-49 }, -90, true);

	std::thread t(okurwa);
	t.detach();
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

	static bool was_up_pressed = false;
	bool is_up_pressed = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;

	static bool was_down_pressed = false;
	bool is_down_pressed = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

	static bool was_left_pressed = false;
	bool is_left_pressed = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;

	static bool was_right_pressed = false;
	bool is_right_pressed = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;

	if (is_up_pressed)
	{
		player.move_forward(deltaTime, 0, cubes);
		if (!was_up_pressed && sliding::motion_enabled)
		{
			sliding::up();
		}
	}
		//cameraPos += cameraSpeed * cameraFront;
	if (is_down_pressed)
	{
		player.move_forward(deltaTime, 180, cubes);
		if (!was_down_pressed && sliding::motion_enabled)
		{
			sliding::down();
		}
	}
		//cameraPos -= cameraSpeed * cameraFront;
	if (is_left_pressed)
	{
		player.move_forward(deltaTime, -90, cubes);
		if (!was_left_pressed && sliding::motion_enabled)
		{
			sliding::left();
		}
	}
		//cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (is_right_pressed)
	{
		player.move_forward(deltaTime, 90, cubes);
		if (!was_right_pressed && sliding::motion_enabled)
		{
			sliding::right();
		}
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		player.jump();
		//cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	was_up_pressed = is_up_pressed;
	was_down_pressed = is_down_pressed;
	was_left_pressed = is_left_pressed;
	was_right_pressed = is_right_pressed;
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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

	//glfwSwapInterval(0);

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

	Texture credit1("res/textures/credit1.png");
	Texture credit2("res/textures/credit2.png");
	Texture credit3("res/textures/credit3.png");
	Texture credit4("res/textures/post_credit.png");
	Texture black("res/textures/black.png");

	cubes.push_back(Cube{ glm::vec3{300,300,-50}, &text1, 1 });
	cubes.push_back(Cube{ glm::vec3{310,300,-50}, &text2,1 });
	cubes.push_back(Cube{ glm::vec3{320,300,-50}, &text3,1 });
	cubes.push_back(Cube{ glm::vec3{330,300,-50}, &text4,1 });
	cubes.push_back(Cube{ glm::vec3{400,300,-50}, &credit1,1 });
	cubes.push_back(Cube{ glm::vec3{400,299,-50}, &credit2,1 });
	cubes.push_back(Cube{ glm::vec3{400,298,-50}, &credit3,1 });
	cubes.push_back(Cube{ glm::vec3{400,297,-50}, &black,1 });
	cubes.push_back(Cube{ glm::vec3{400,301,-50}, &black,1 });
	cubes.push_back(Cube{ glm::vec3{500,300,-50}, &credit4,1 });

	teleport_camera(glm::vec3{ 300,300,-49 }, -90, true);
	
	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			cubes.push_back(Cube{ glm::vec3{x,-1,y}, &texture });

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
				cubes.push_back(Cube{ glm::vec3{x,-1,double(y) - 1000.0}, &texture3, true });
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
			cubes.push_back(Cube{ glm::vec3{x,5,double(y)-1000.0}, &texture3, true });


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
			cubes.push_back(Cube{ glm::vec3{-1,x,double(y) - 1000.0}, &texture3, true });
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
			cubes.push_back(Cube{ glm::vec3{10,x,double(y) - 1000.0}, &texture3, true });
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
			cubes.push_back(Cube{ glm::vec3{x,y,-1.0 - 1000.0}, &texture3, true });
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
			cubes.push_back(Cube{ glm::vec3{x,y,10.0-1000.0}, &texture3, true });
		}

	auto z_z_index = cubes.size();

	for (long x = 0; x < 2; ++x)
	{
		for (long y = 0; y < 10; ++y)
		{
			//floor
			cubes.push_back(Cube{ glm::vec3{-2-x,0,5 + y - 1000.0}, &texture3, true });

			//ceiling
			if (y == 9-0 || y == 9-3 || y == 9-5)
			{
				cubes.push_back(Cube{ glm::vec3{-2 - x,4,5 + y - 1000.0}, &texture3, true });
			}
			else cubes.push_back(Cube{ glm::vec3{-2 - x,5,5 + y - 1000.0}, &texture3, true });

			if (y < 2 && x == 0)
			{
				cubes.push_back(Cube{ glm::vec3{-4,0,5 + y - 1000.0}, &texture3, true });
				cubes.push_back(Cube{ glm::vec3{-4,5,5 + y - 1000.0}, &texture3, true });
			}
		}
	}

	cubes.push_back(Cube{ glm::vec3{-5,1,5 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-6,2,5 - 1000.0}, &texture4,1 });
	cubes.push_back(Cube{ glm::vec3{-6,3,5 - 1000.0}, &texture4,1 });
	cubes.push_back(Cube{ glm::vec3{-5,4,5 - 1000.0}, &texture3,1 });

	cubes.push_back(Cube{ glm::vec3{-5,1,6 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-5,2,6 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-5,3,6 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-5,4,6 - 1000.0}, &texture3,1 });

	cubes.push_back(Cube{ glm::vec3{-4,1,7 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,2,7 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,3,7 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,4,7 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,1,8 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,2,8 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,3,8 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,4,8 - 1000.0}, &texture3,1 });

	cubes.push_back(Cube{ glm::vec3{-4,1,9 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,2,9 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,3,9 - 1000.0}, &texture3,1 });

	cubes.push_back(Cube{ glm::vec3{-5,1,10 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-5,2,10 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-5,3,10 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,4,10 - 1000.0}, &texture3,1 });

	cubes.push_back(Cube{ glm::vec3{-4,0,10 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-1,0,10 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-1,4,10 - 1000.0}, &texture3,1 });

	cubes.push_back(Cube{ glm::vec3{-4,1,11 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,2,11 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,3,11 - 1000.0}, &texture3,1 });

	cubes.push_back(Cube{ glm::vec3{-4,1,14 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,2,14 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{-4,3,14 - 1000.0}, &texture3,1 });
	//
	cubes.push_back(Cube{ glm::vec3{-3,3,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-2,3,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-3,2,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-2,2,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-3,1,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-2,1,15 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{-1,3,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,2,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,1,14 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{-1,3,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,2,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,1,11 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{-1,0,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,0,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,0,13 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,0,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,4,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,4,13 - 1000.0}, &texture3, 1 });
	//
	cubes.push_back(Cube{ glm::vec3{-4,1,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-4,2,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-4,3,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-4,4,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-4,1,13 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-4,2,13 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-4,3,13 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-4,4,13 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{-1,4,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{-1,4,11 - 1000.0}, &texture3, 1 });

	for (int x = 0; x < 2; ++x)
	{
		cubes.push_back(Cube{ glm::vec3{0,6 * x - 1,14 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{0,6 * x - 1,13 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{0,6 * x - 1,12 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{0,6 * x - 1,11 - 1000.0}, &texture3, 1 });

		cubes.push_back(Cube{ glm::vec3{2,6 * x - 1,14 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{2,6 * x - 1,13 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{2,6 * x - 1,12 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{2,6 * x - 1,11 - 1000.0}, &texture3, 1 });

		cubes.push_back(Cube{ glm::vec3{1,7 * x - 1,14 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{1,7 * x - 1,13 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{1,7 * x - 1,12 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{1,7 * x - 1,11 - 1000.0}, &texture3, 1 });
	}

	cubes.push_back(Cube{ glm::vec3{3,-1,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,-1,13 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{3,-1,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,-1,16 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{4,-1,17 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,-1,16 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,-1,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,-1,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,-1,13 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,-1,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,-1,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,-1,10 - 1000.0}, &texture3, 1 });


	cubes.push_back(Cube{ glm::vec3{3,-1,10 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{3,3,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,3,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,3,13 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,3,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,4,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,4,12 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,4,13 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,4,14 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{3,2,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,2,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,1,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,1,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,0,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,0,14 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{4,3,11 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,3,14 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,3,17 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{4,4,16 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,4,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,4,13 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{4,4,12 - 1000.0}, &texture3, 1 });

	for (int i = 0; i < 5; ++i)
	{
		cubes.push_back(Cube{ glm::vec3{0,i,15 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{1,i,15 - 1000.0}, &texture3, 1 });
		cubes.push_back(Cube{ glm::vec3{2,i,15 - 1000.0}, &texture3, 1 });
	}
	cubes.push_back(Cube{ glm::vec3{1,5,15 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{1,5,10 - 1000.0}, &texture3, 1 });



	cubes.push_back(Cube{ glm::vec3{3,0,17 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,1,17 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,2,17 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{3,3,16 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{3,3,15 - 1000.0}, &texture3, 1 });

	cubes.push_back(Cube{ glm::vec3{2,0,16 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{2,1,16 - 1000.0}, &texture3, 1 });
	cubes.push_back(Cube{ glm::vec3{2,2,16 - 1000.0}, &texture3, 1 });

	auto z_z_index2 = cubes.size();

	for (int i = z_z_index; i < z_z_index2; ++i)
	{
		auto& cube = cubes[i];
		cubes.push_back(Cube{ glm::vec3{cube.pos.x,cube.pos.y,-996 - (995 + cube.pos.z)}, cube.texture, 1 });
	}
	z_z_index2 = cubes.size();
	for (int i = z_z_index; i < z_z_index2; ++i)
	{
		auto& cube = cubes[i];
		cubes.push_back(Cube{ glm::vec3{cube.pos.x * (-1) + 9,cube.pos.y,cube.pos.z}, cube.texture, 1 });
	}
	////////////////////////

	cubes.push_back(Cube{ glm::vec3{4,0,18 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{4,1,18 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{4,2,18 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{5,0,18 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{5,1,18 - 1000.0}, &texture3,1 });
	cubes.push_back(Cube{ glm::vec3{5,2,18 - 1000.0}, &texture3,1 });

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
	
	Texture lab_tile = std::string("res/textures/lab/tile.png");
	Texture lab_tile_v = std::string("res/textures/lab/tile_variation.png");
	Texture lab_yellow = std::string("res/textures/lab/yellow.png");
	Texture lab_black = std::string("res/textures/lab/gray.png");

	constexpr std::string_view test_text =
		"***.***X***.***"
		".*..*..X*....*."
		".*..***X***..*."
		".*..*.XXX.*..*."
		".*..***X***..*.";
	for (int x = 0; x < 17; ++x)
		for (int z = 0; z < 9; ++z)
	{
			cubes.push_back(Cube{ glm::vec3{-1000+x,-1,-1000.0+z}, &lab_tile, 1 });
			auto& mycubic = cubes.emplace_back(Cube{ glm::vec3{-1000+x,-1+7,-1000.0+z}, &lab_tile, 1 });
			
			if (x == 8)
			{
				mycubic.texture = &lab_yellow;
			}

			if (x != 0 && x != 16 && z == 1)
			{
				for (int y = 0; y < 5; ++y)
				{
					if (test_text.at(x-1+y*15) == '*')
						cubes.push_back(Cube{ glm::vec3{-1000 + x,4-y,-1000.0 + z}, &lab_tile, 1 });
				}
			}
	}


	for (int y = 0; y < 6; ++y)
		for (int x = 0; x < 17; ++x)
	{
			if (x != 0 && x != 16 && test_text.at(x - 1 + std::clamp<>(y-1, 0, 4) * 15) == 'X')
			{
				cubes.push_back(Cube{ glm::vec3{-1000 + x,5 - y,-1000.0 - 1}, &lab_yellow, 1 });
			}
			else
			{
				cubes.push_back(Cube{ glm::vec3{-1000 + x,5 - y,-1000.0 - 1}, &lab_tile, 1 });
			}
	}

	for (int y = 0; y < 6; ++y)
		for (int x = 0; x < 17; ++x)
		{
			auto& cube = cubes.emplace_back(Cube{ glm::vec3{-1000 + x,5 - y,-1000.0 + 9}, &lab_tile, 1 });
			if ((y == 3 && x >= 8) || (x==8&&y<=3))
			{
				cube.texture = &lab_yellow;
			}
			if (x == 7 || x == 9 || x == 8)
			{
				if (y == 2 || y == 3 || y == 4)
				{
					if (!(y == 3 && x == 8))
					{
						//cube.texture = &lab_yellow;
					}
					if (y == 3 && x == 8)
					{
						//cube.texture = &lab_tile;
					}
				}
			}
		}

	for (int y = 0; y < 6; ++y)
		for (int x = 0; x < 17; ++x)
		{
			if (x == 7 || x == 9 || x == 8)
			{
				if (y == 2 || y == 3 || y == 4)
				{

				}
			}
		}

	for (int y = 0; y < 6; ++y)
		for (int z = 0; z < 9; ++z)
		{
			{
				cubes.push_back(Cube{ glm::vec3{-1000 -1,5 - y,-1000.0 + z}, &lab_tile, 1 });
			}
		}

	for (int y = 0; y < 6; ++y)
		for (int z = 0; z < 9; ++z)
		{
			{
				if (z > 4 && z < 8)
				{
					if (y < 5 && y > 1)
					{
						continue;
					}
				}
				auto& cubic = cubes.emplace_back(Cube{ glm::vec3{-1000 + 17,5 - y,-1000.0 + z}, &lab_tile, 1 });
				if (z > 5 && y == 3)
					cubic.texture = &lab_yellow;
			}
		}

	for (int z = 4; z <= 8;++z)
		for (int y = 1; y <= 5;++y)
			for (int x = 18; x < 18 + 11;++x)
			{
				bool a = (z == 4 || z == 8);
				bool b = (y == 1 || y == 5);
				if (x > 18 + 7 && z == 4)
				{
					continue;
				}
				if (a ^ b)
				{
					auto& cubic = cubes.emplace_back(Cube{ glm::vec3{-1000 + x,5 - y,-1000.0 + z}, &lab_tile, 1 });
					if ((z == 8 && y == 3 && x < 18+10) || (x == 17 + 10 && z == 8 && y < 3) || (y == 1 && x == 17 + 10))
						cubic.texture = &lab_yellow;
				}
			}

	for (int x = 0; x < 3; ++x)
		for (int z = 0; z < 6; ++z)
		{
			auto& cubic = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + x,0,-1000.0 - z - 1 - 3 + 8}, &lab_tile, 1 });
			auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + x,4,-1000.0 - z - 1 - 3 + 8}, &lab_tile, 1 });
			if (x == 1)
			{
				cubic2.texture = &lab_yellow;
			}
		}

	for (int x = 0; x < 12; ++x)
		for (int y = 0; y < 3; ++y)
	{
		auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + x - 3,y+1,-1000.0 - 6 - 1 - 3 + 8}, &lab_tile, 1 });
		if ((y == 1 && x >= 4)||(x == 4 && y >= 2))
		{
			cubic2.texture = &lab_yellow;
		}
	}

	for (int x = 0; x < 6; ++x)
		for (int z = 0; z < 3; ++z)
		{
			auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + 9 + x - 6,0,-1000.0 - 6 - 1 - 3 + 8 - z + 3}, &lab_tile, 1 });
			auto& cubic3 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + 9 + x - 6,4,-1000.0 - 6 - 1 - 3 + 8 - z + 3}, &lab_tile, 1 });
		}

	for (int x = 0; x < 3; ++x)
		for (int z = 0; z < 3; ++z)
		{
			auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + 9 + x-6 - 6,0,-1000.0 - 6 - 1 - 3 + 8 - z + 3}, &lab_tile, 1 });
			auto& cubic3 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + 9 + x-6 - 6,4,-1000.0 - 6 - 1 - 3 + 8 - z + 3}, &lab_tile, 1 });
		}

	for (int x = 0; x < 3; ++x)
		for (int z = 0; z < 3; ++z)
		{
			auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + 9 + x - 9 - 6,0,-1000.0 - 6 - 1 - 3 + 8 - z + 3}, &lab_tile, 1 });
			auto& cubic3 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + 9 + x - 9 - 6,4,-1000.0 - 6 - 1 - 3 + 8 - z + 3}, &lab_tile, 1 });
		}

	for (int x = 0; x < 3; ++x)
		for (int z = 0; z < 6; ++z)
		{
			auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + 9 + x - 9 - 6,0,-1000.0 - 6 - 1 - 3 + 8 - z + 0}, &lab_tile, 1 });
			auto& cubic3 = cubes.emplace_back(Cube{ glm::vec3{-1000 + 18 + 8 + 9 + x - 9 - 6,4,-1000.0 - 6 - 1 - 3 + 8 - z + 0}, &lab_tile, 1 });
		}

	for (int z = 0; z < 5; ++z)
		for (int y = 0; y < 5; ++y)
		{
			auto zz1 = (z == 0 || z == 4);
			auto zz2 = (y == 0 || y == 4);
			if (zz1 ^ zz2)
			auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-965,y,-998 - z}, &lab_black, 1 });
		}

	cubes.emplace_back(Cube{ glm::vec3{-965,4,-998 - 0}, &lab_black, 1 });
	cubes.emplace_back(Cube{ glm::vec3{-965,4,-998 - 4}, &lab_black, 1 });

	for (int x = 0; x < 6; ++x)
	for (int z = 0; z < 7; ++z)
		for (int y = 0; y < 6; ++y)
		{
			auto zz1 = (z == 0 || z == 6);
			auto zz2 = (y == 0 || y == 5);
			if (zz1 ^ zz2)
				auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-965+x+1,y,-998 - z + 1}, &lab_black, 1 });
		}

	for (int z = 0; z < 5; ++z)
		for (int y = 0; y < 5; ++y)
		{
			auto zz1 = (z == 0 || z == 4);
			auto zz2 = (y == 0 || y == 4);
			if (zz1 ^ zz2)
				auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-965+6+1,y,-998 - z}, &lab_black, 1 });
		}

	for (int x = 0; x < 18; ++x)
	for (int z = 0; z < 5; ++z)
		for (int y = 0; y < 5; ++y)
		{
			auto zz1 = (z == 0 || z == 4);
			auto zz2 = (y == 0 || y == 4);
			if (zz1 ^ zz2)
			{
				auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-965 + 6 + 2 + x,y,-998 - z}, &lab_tile, 1 });
				if (y == 0 && z == 2)
					cubic2.texture = &lab_yellow;
			}
		}

	for (int z = 0; z < 5; ++z)
		for (int y = 0; y < 5; ++y)
		{
			auto zz1 = (z == 0 || z == 4);
			auto zz2 = (y == 0 || y == 4);
			if (zz1 ^ zz2)
				auto& cubic2 = cubes.emplace_back(Cube{ glm::vec3{-965 + 6 + 2 + 18,y,-998 - z}, &lab_black, 1 });
		}

	cubes.emplace_back(Cube{ glm::vec3{-965+6+1,4,-998 - 0}, &lab_black, 1 });
	cubes.emplace_back(Cube{ glm::vec3{-965+6+1,4,-998 - 4}, &lab_black, 1 });

	auto yacie_blockade_lab = cubes.size();

	cubes.emplace_back(Cube{ glm::vec3{-965 + 6 + 1,2,-998 - 1}, &lab_black, false });
	cubes.emplace_back(Cube{ glm::vec3{-965 + 6 + 1,2,-998 - 2}, &lab_black, false });
	cubes.emplace_back(Cube{ glm::vec3{-965 + 6 + 1,2,-998 - 3}, &lab_black, false });

	auto yacie_blockade_lab_end = cubes.size();

	constexpr std::string_view labmapa =
		"ZZZ/............"
		"ZZZ/............"
		"ZZZ/..#########."
		"ZZZ/.#ZZZZZZZZZ#"
		"ZZZ/.#ZZZZZZZZZ#"
		"ZZZ/.#ZZZZZZZZZ#"
		"ZZZ/.#ZZZ/#####."
		"ZZZ/.#ZZZ/......"
		"ZZZ/##ZZZ/......"
		"ZZZZZZZZZ/......"
		"ZZZZZZZZZ/......"
		"ZZZZZZZZZ/......"
		"######ZZZ/......"
		".....#ZZZ/......"
		".....#ZZZ/......"
		".....#ZZZ/......"
		".....#ZZZ/......"
		".....#ZZZ/......";

	for (int z = 0; z < 16;++z)
		for (int x = 0; x < 18; ++x)
		for (int y = 0; y < 3; ++y)
		{
			if (labmapa.at(x*16+z) == '#')
				cubes.emplace_back(Cube{ glm::vec3{-985 + x + 2,y+1,-998 - z + 5}, &lab_tile, true });
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
	bg_music2.setVolume(1.5);

	bg_music_credits.load("res/sounds/starry_dream.mp3");
	bg_music_credits.setVolume(1.25);

	sfx_jump.load("res/sounds/b1.wav");
	sfx_jump.setVolume(2.0);
	sfx_land.load("res/sounds/b0.wav");
	sfx_land.setVolume(1.0);

	sfx_pop.load("res/sounds/text.wav");
	sfx_pop.setVolume(2.0);
	sfx_boom.load("res/sounds/exp.wav");
	sfx_boom.setVolume(2.0);

	voice0.load("res/sounds/welcome_to_the_test.wav");
	voice1.load("res/sounds/follow_line.wav");
	voice2.load("res/sounds/can_you_just_follow.wav");
	voice3.load("res/sounds/good_job_on_the_test.wav");

	voice0.setVolume(2.0);
	voice1.setVolume(2.0);
	voice2.setVolume(2.0);
	voice3.setVolume(2.0);

	bg_music3.load("res/sounds/happy_dying_song.mp3");
	bg_music3.setLooping(true);

	roll_loop.load("res/sounds/roll_loop.wav");
	roll_loop.setLooping(true);
	roll_loop.setVolume(0);
	roll_loopi = soloud.play(roll_loop);

	lastFrame = glfwGetTime();
	ourShader.use();
	ourShader.setVec3("afog_color", { 0,0,0 });
	ourShader.setFloat("afog_farz", 10);
	srand(time(0));

	for (auto& cube : cubes)
	{
		if (cube.texture == &lab_tile)
		{
			if (rand() % 64 == 0)
			{
				cube.texture = &lab_tile_v;
			}
		}
	}

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

	std::vector<float> c_vertices;

	for (unsigned i = 0; i < 180; ++i)
	{
		c_vertices.push_back(vertices[i]);
	}

	std::unordered_map<Texture*, unsigned> c_offsets;
	std::unordered_map<Texture*, unsigned> c_count;
	std::unordered_map<Texture*, std::vector<glm::vec3>> c_positions;
	for (auto& cube : cubes)
	{
		if (cube.is_static)
		{
			c_positions[cube.texture].push_back(cube.pos);
		}
	}
	for (auto& [txt, pos] : c_positions)
	{
		c_offsets[txt] = c_vertices.size();
		c_count[txt] = pos.size();
		for (auto& my_pos : pos)
		{
			for (int i = 0; i < 36; ++i)
			{
				float x = vertices[i * 5];
				float y = vertices[i * 5 + 1];
				float z = vertices[i * 5 + 2];
				c_vertices.push_back(x + my_pos.x);
				c_vertices.push_back(y + my_pos.y);
				c_vertices.push_back(z + my_pos.z);
				c_vertices.push_back(vertices[i * 5 + 3]);
				c_vertices.push_back(vertices[i * 5 + 4]);
			}
		}
	}

	c_positions.clear();

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * c_vertices.size(), c_vertices.data(), GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	///
	//start_credits();
	/// 
	while (!glfwWindowShouldClose(window))
	{
		static uint32_t frames = 0;
		static uint32_t clock = 0;
		++frames;

		static std::optional<Player> player_save;
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = std::min<>(double(currentFrame - lastFrame), 1.0/30.0) * 1.125;
		
		if (uint32_t(currentFrame) > clock)
		{
			//std::cout << "FPS: " << frames << std::endl;
			frames = 0;
			clock++;
		}

		if (credit_progress.has_value())
		{
			player_save.reset();
			static bool onlyonce = true;
			if (credit_progress > 4.0)
			{
				if (onlyonce)
				{
					teleport_camera(glm::vec3{ 500,300.0,-49 }, -90, true);
					fade::fade({ 0,0,0 }, 5);
				}
				onlyonce = false;
			}
			else
			{
				*credit_progress += double(currentFrame - lastFrame) * 0.20;
				teleport_camera(glm::vec3{ 400,301.0 - *credit_progress,-49 }, -90, true);
			}
		}
		lastFrame = currentFrame;

		roll = std::max<>((player.pos.z - 15.0) * std::abs(player.pos.z / 10.0) * std::abs(player.pos.z / 15.0) * std::abs(player.pos.z / 15.0), 0.0);
		static bool transition = false;
		if (!transition)
		{
			if (player.pos.z > 11)
			{
				soloud.setPause(roll_loopi, false);
				soloud.setVolume(roll_loopi, std::clamp((player.pos.z - 10.0) / 20.0, 0.25, 0.75));
				soloud.setRelativePlaySpeed(roll_loopi, std::clamp((player.pos.z - 10.0) / 30.0, 0.5, 4.0));
				soloud.setPan(roll_loopi, sin(glm::radians(roll)));
			}
			else
			{
				soloud.setPause(roll_loopi, true);
			}
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
		}
		else
		{
			static bool angry_voice = false;
			if (!angry_voice && player.pos.x >= -980.0 && player.pos.x <= -978 && player.pos.z <= -2.0 - 1000.0+3.0 && player.pos.z >= -5.0 - 1000.0+3.0)
			{
				angry_voice = true;
				soloud.play(voice2);
			}
		}

		// input
		// -----
		processInput(window);
		player.gravity(deltaTime, cubes);
		cameraPos = player.pos;
		static bool transition2 = false;
		static bool was_sskip_pressed = false;
		bool is_sskip_pressed = glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS;
		static bool won_ss = false;
		static bool trans3last = false;
		if (player.pos.y < -50 || (is_sskip_pressed && !was_sskip_pressed && glfwGetMouseButton(window,2) == GLFW_PRESS))
		{
			if (transition)
			{
				if (transition2)
				{
					
					if (trans3last)
						abort();
					trans3last = true;
					if (won_ss)
					{
						ourShader.use();
						ourShader.setVec3("afog_color", { 0,0,0 });
						start_credits();
						fade::fade({ 1,1,1 }, 3.0);
					}
					else abort();
				}
				else
				{
					transition2 = true;

					ourShader.use();
					ourShader.setVec3("afog_color", glm::vec3{ 0.0,0.4,0.8 });
					teleport_camera(glm::vec3{ 8 - 1000,1,7 - 1000.0 }, -90, false);
					fade::fade({ 1,0,1 }, 3);
					soloud.stopAll();
					soloud.play(bg_music3);
					player_save.reset();

					player_speed_m = 1.25;

					//sliding::init();
					//sliding::enable();

					std::thread t(play_voices);
					t.detach();
				}

			}
			else
			{
				roll_loop.stop();
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
		}
		was_sskip_pressed = is_sskip_pressed;
		static bool blockade_moved = false;
		if (!blockade_moved && player.pos.z < -991)
		{
			player_save.reset();
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
		static bool inited_ss = false;
		static bool started_ss = false;
		
		if (!inited_ss && player.pos.x > -968.0 + 2.0 - 10.0 && player.pos.z < -900.0 && player.pos.x < -800.0f)
		{
			inited_ss = true;
			sliding::init();
		}
		if (inited_ss && !started_ss && player.pos.x > -964.0 + 2.5 && player.pos.z < -900.0 && player.pos.x < -800.0f)
		{
			started_ss = true;
			sliding::enable();

			for (int i = yacie_blockade_lab; i < yacie_blockade_lab_end; ++i)
			{
				auto& cube_pos = cubes[i].pos;
				cube_pos.x -= 7.0;
			}

			teleport_camera(glm::vec3(-964.0 + 2.5, 2.0, -1000.0 + 1.5), -90, false);
			fade::fade({ 0,0,0 }, 0.25);
		}

		if (started_ss && !won_ss && player.pos.x > -964.0 + 2.5 + 3.0 && player.pos.z < -900.0 && player.pos.x < -800.0f)
		{
			won_ss = true;
			soloud.play(voice3);
		}

		if (sliding::init)
		{
			sliding::update(deltaTime);
		}


		if (motion && !sliding::motion_enabled)
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
				if (player_save && !credit_progress.has_value())
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
		else
		{
			player_save.reset();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST); // enable depth testing (is disable

		// render
		// ------

		//glBindVertexArray(VAO);

		//glBindBuffer(GL_ARRAY_BUFFER, VBO);
		if (trans3last)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		}
		else if (transition2)
		{
			glClearColor(0.0f, 0.4f, 0.8f, 1.0f);
		}
		else if (transition)
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
			//if (glm::distance(cube.pos,player.pos) <= transition ? 30.0f : 20.0f)
			if (cube.is_static)
				continue;

			if ((!transition) ^ (cube.texture == &texture || cube.texture == &texture2))
				continue;

				cube.render(&ourShader);
		}
		if (sliding::inited)
		{
			sliding::render(&ourShader);
		}
		//render static boxes
		glm::mat4 model = glm::mat4(1.0f);
		ourShader.setMat4("model", model);
		for (auto& [txt, offset] : c_offsets)
		{
			if (!trans3last)
			{
				if ((transition ^ transition2) ^ (txt == &texture3))
					continue;
				if (transition2 ^ (txt == &lab_black || txt == &lab_yellow || txt == &lab_tile_v || txt == &lab_tile))
					continue;
			}

			txt->use();
			
			glDrawArrays(GL_TRIANGLES, c_offsets[txt] / 5U, c_count[txt] * 36U);
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