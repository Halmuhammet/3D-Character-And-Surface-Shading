
/*
* Title: Surface Shading - Module 6
* Description: This program renders 3D character simulation employing surface shading
* Author: Halmuhammet Muhamedorazov
* Date: 11/02/2024
* Version number: g++ 13.2.0, gcc 11.4.0
* Requirements: This program requires GLAD, GLFW, GLM, gif
* Note: The user can move with w,s,a,d, and rotate with right and left arrow keys
* Version requirement: This program requires GLFW 3.3 or above
*/

// Import the libraries that will be used in this program
#include "Character.h"
#include "gif.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <numbers>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace std;


// Constants
float characterSpeed = 0.009f;
float characterRotationSpeed = 0.05f;
// Global character instance
Character character;
Character scaledCharacter;


/*--------------------------------------------------------------
Function prototypes which are defined at the end of this program
---------------------------------------------------------------*/

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void setupBuffers(GLuint& VAO, GLuint& VBO, GLuint& EBO);
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
void processInput(GLFWwindow* window);
void drawCube(GLuint shaderProgram, GLuint VAO, glm::mat4 view, glm::mat4 projection, vector<float> scale, float rotationAngle, vector<float> position, vector<float> color);

/*---------------------------------------------
Shader Program Source Code
-----------------------------------------------*/
// Vertex Shader
const char* vertexShaderSource = R"(#version 330 core
layout(location = 0) in vec3 aPos; //cube vertices
layout(location = 1) in vec3 aNormal; //normals of cube

out vec3 FragPos;  
out vec3 Normal;  

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Calculate position correctly
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // World space fragment position
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Calculate normal using normal matrix
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);  
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;  

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main() {
    // Normalize the normal again as it might have been interpolated
    vec3 norm = normalize(Normal);
    
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
)";

int main()
{
	/*-----------------------------------------------------------------------
	Setup the Window
	-------------------------------------------------------------------------*/

	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 950 by 950 pixels, naming it "Linear Transformations"
	GLFWwindow* window = glfwCreateWindow(950, 950, "3D Animation and Shading", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to initialize the window object" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Make the context of our window the main context in current window
	glfwMakeContextCurrent(window);

	// This function dynamically sets the viewport size when the user resizes window
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Load GLAD so it configures OpenGL
	//Glad helps getting the address of OpenGL functions which are OS specific
	gladLoadGL();

	/*---------------------------------------------------------------------------
	Setup and compile the Vertex and Fragment Shader programs
	----------------------------------------------------------------------------*/

	GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

	/*------------------------------------------------------------------------------
	 Set up VBO and VAO for the cubes - used multiple VAOs to separate object data
	--------------------------------------------------------------------------------*/

	GLuint cubeVAO, cubeVBO, cubeEBO;
	GLuint headVAO, headVBO, headEBO;
	GLuint torsoVAO, torsoVBO, torsoEBO;
	GLuint armVAO, armVBO, armEBO;
	GLuint legVAO, legVBO, legEBO;

	setupBuffers(cubeVAO, cubeVBO, cubeEBO);
	setupBuffers(headVAO, headVBO, headEBO);
	setupBuffers(torsoVAO, torsoVBO, torsoEBO);
	setupBuffers(armVAO, armVBO, armEBO);
	setupBuffers(legVAO, legVBO, armEBO);


	// Initialize character position and rotation
	character.setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
	character.setRotation(glm::vec3(0.0f));
	character.setScale(glm::vec3(1.0f));

	// Initialize the scaled character position
	scaledCharacter.setPosition(glm::vec3(5.0, 1.0, 0.0));
	scaledCharacter.setRotation(glm::vec3(45.0f));


	// Initialize GIF
	GifWriter gifWriter;
	GifBegin(&gifWriter, "output.gif", 950, 950, 0);

	// rendering loop
	while (!glfwWindowShouldClose(window))
	{
		// Set the background color to light blue (clear sky)
		glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
		// Clear the color and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Enable depth testing for 3D rendering
		glEnable(GL_DEPTH_TEST);

		// Get window dimensions for calculating aspect ratio
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		float aspect = (float)width / (float)height;

		// Set up perspective projection matrix
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

		// Set up a static camera position and view
		glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 15.0f);  // Fixed camera position
		glm::vec3 cameraTarget = glm::vec3(0.0f, 1.0f, 0.0f); // Looking at the center
		glm::mat4 view = glm::lookAt(
			cameraPos,
			cameraTarget,
			glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
		);

		// Draw the ground
		drawCube(shaderProgram, cubeVAO, view, projection,
			{ 20.0f, 0.1f, 20.0f },    // Scale: wide and flat
			0.0f,                      // No rotation
			{ 0.0f, -2.0f, 0.0f },     // Position: slightly below center
			{ 0.0f, 1.0f, 0.0f });     // Color: green

		// Draw the character
		character.drawCharacter(shaderProgram, headVAO, torsoVAO, armVAO, legVAO,
			view, projection,
			glm::vec3(1.0, 1.0, 1.0),  // scale
			character.getRotation(),   // rotation
			character.getPosition());  // position

		// Draw the 1.5 times scaled character in all directions
		scaledCharacter.drawCharacter(shaderProgram, headVAO, torsoVAO, armVAO, legVAO,
			view, projection,
			glm::vec3(1.5, 1.5, 1.5),      // scale
			scaledCharacter.getRotation(), // rotation
			scaledCharacter.getPosition()); // position

		// Process user input for character movement
		processInput(window);

		// Capture the frame
		std::vector<uint8_t> frame(950 * 950 * 4); // RGBA
		glReadPixels(0, 0, 950, 950, GL_RGBA, GL_UNSIGNED_BYTE, frame.data());

		// Flip the frame vertically
		for (int y = 0; y < 950 / 2; ++y) {
			for (int x = 0; x < 950; ++x) {
				int topIndex = (y * 950 + x) * 4;
				int bottomIndex = ((950 - y - 1) * 950 + x) * 4;

				// Swap the pixels
				std::swap(frame[topIndex], frame[bottomIndex]);
				std::swap(frame[topIndex + 1], frame[bottomIndex + 1]);
				std::swap(frame[topIndex + 2], frame[bottomIndex + 2]);
				std::swap(frame[topIndex + 3], frame[bottomIndex + 3]);
			}
		}
		// Add frame to GIF
		GifWriteFrame(&gifWriter, frame.data(), 950, 950, 0);

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}
	// end the gif writer
	GifEnd(&gifWriter);

	// Delete all the objects we've created
	/*glDeleteVertexArrays(1, &planet1VAO);
	glDeleteBuffers(1, &planet1VBO);*/
	glDeleteProgram(shaderProgram);
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}

/**
 * Renders a cube with specified transformations, color, and lighting.
 * Sets shader uniforms, applies transformations, and draws the cube.
 */

void drawCube(GLuint shaderProgram, GLuint VAO, glm::mat4 view, glm::mat4 projection,
	vector<float> scale, float rotationAngle, vector<float> position, vector<float> color) {
	// Activate the shader program
	glUseProgram(shaderProgram);

	// Create and transform the model matrix
	glm::mat4 model = glm::mat4(1.0f);  // Start with an identity matrix
	model = glm::translate(model, glm::vec3(position[0], position[1], position[2]));  // Apply translation
	model = glm::rotate(model, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));  // Apply rotation around Y-axis
	model = glm::scale(model, glm::vec3(scale[0], scale[1], scale[2]));  // Apply scaling

	// Get uniform locations for transformation matrices
	GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

	// Set uniform values for transformation matrices
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Set uniform values for lighting
	glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 5.0f, 8.0f, 12.0f);  // Light position
	glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 6.0f);   // Camera position
	glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);  // White light
	glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), color[0], color[1], color[2]);  // Object color

	// Bind the VAO and draw the cube
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);  // Draw 36 vertices (6 faces * 2 triangles * 3 vertices)
	glBindVertexArray(0);  // Unbind the VAO
}

/*
This function is used to send background vertices to the GPU that will be used by the vertex and fragment shaders
*/

void setupBuffers(GLuint& VAO, GLuint& VBO, GLuint& EBO) {

	// Vertex data for a cube
	float vertices[] = {
		// positions          // normals
		// Front face
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		// Back face
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		// Left face
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		// Right face
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		 // Bottom face
		 -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,

		 // Top face
		 -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
	};

	// Indices for drawing the cube using triangles
	unsigned int indices[] = {
		0, 1, 2, 2, 3, 0,     // Front face
		4, 5, 6, 6, 7, 4,     // Back face
		8, 9, 10, 10, 11, 8,  // Left face
		12, 13, 14, 14, 15, 12, // Right face
		16, 17, 18, 18, 19, 16, // Bottom face
		20, 21, 22, 22, 23, 20  // Top face
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	//Bind the VAO
	glBindVertexArray(VAO);
	// Bind the VBO and set the buffer data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind the EBO and set the buffer data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// Set up vertex attribute pointers - for coordinates - first 3 elements in the row of vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//For normals - which is the last 3 elements in the row of vertices
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Unbind the VBO and VAO to avoid accidental modifications
	glBindVertexArray(0);
}

/*
Helper function to create and compile a shader program - returns shader ID
*/
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
	// Create Vertex Shader Object and get its reference
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// Attach Vertex Shader source to the Vertex Shader Object
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(vertexShader);

	// Create Fragment Shader Object and get its reference
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Attach Fragment Shader source to the Fragment Shader Object
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	// Compile the Fragment Shader into machine code
	glCompileShader(fragmentShader);

	// Create Shader Program Object and get its reference
	GLuint shaderProgram = glCreateProgram();
	// Attach the Vertex and Fragment Shaders to the Shader Program
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	// Wrap-up/Link all the shaders together into the Shader Program
	glLinkProgram(shaderProgram);

	// Delete the now useless shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// return the shaderProgram ID as integer
	return shaderProgram;
}

/*
Function to adjust viewport dynamically
glfw: whenever the window size changed (by OS or user resize) this callback function executes
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

/*
Funtion used in the render loop to process user input
*/

void processInput(GLFWwindow* window) {
	// Check for ESC key to close the window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Get current position and rotation of the character
	glm::vec3 currentPos = character.getPosition();
	glm::vec3 currentRot = character.getRotation();

	// Calculate forward and right vectors based on current rotation
	float forwardX = sin(currentRot.y);
	float forwardZ = cos(currentRot.y);
	glm::vec3 forward(forwardX, 0.0f, forwardZ);
	glm::vec3 right(forwardZ, 0.0f, -forwardX);

	// Flag to track if the character is moving
	bool isMoving = false;

	// Handle character movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		currentPos += characterSpeed * forward;
		isMoving = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		currentPos -= characterSpeed * forward;
		isMoving = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		currentPos += characterSpeed * right;
		isMoving = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		currentPos -= characterSpeed * right;
		isMoving = true;
	}

	// Handle character rotation
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		currentRot.y += characterRotationSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		currentRot.y -= characterRotationSpeed;
	}

	// Bound checking to keep character within visible area
	float bounds = 9.0f; // Adjust based on ground size
	currentPos.x = glm::clamp(currentPos.x, -bounds, bounds);
	currentPos.z = glm::clamp(currentPos.z, -bounds, bounds);

	// Update character position and rotation
	character.setPosition(currentPos);
	character.setRotation(currentRot);

	// Calculate delta time for smooth animations
	static float lastFrame = 0.0f;
	float currentFrame = glfwGetTime();
	float deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Update character's swing animation
	character.updateSwing(deltaTime, isMoving);
}