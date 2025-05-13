//Import libraries
#ifndef CHARACTER_H
#define CHARACTER_H
#include "Character.h"
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
#include "Character.h"

// Structure to store transformation parameters
struct TransformParams {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

class Character {
public:
    // Constructor
    Character();

    // Public methods
    void updateRootTransform(float rotationAngle);
    void drawCharacter(GLuint shaderProgram, GLuint headVAO, GLuint torsoVAO,
        GLuint armVAO, GLuint legVAO,
        const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& scale, const glm::vec3& rotation, const glm::vec3& position);

    void updateSwing(float deltaTime, bool isMoving);


    // Getters and setters for position
    void setPosition(const glm::vec3& pos) { rootTransform.position = pos; }
    glm::vec3 getPosition() const { return rootTransform.position; }

    // Getters and setters for rotation
    void setRotation(const glm::vec3& rot) { rootTransform.rotation = rot; }
    glm::vec3 getRotation() const { return rootTransform.rotation; }

    // Getters and setters for scale
    void setScale(const glm::vec3& sc) { rootTransform.scale = sc; }
    glm::vec3 getScale() const { return rootTransform.scale; }

private:
    TransformParams rootTransform;
    float armSwing;
    float legSwing;
    float swingSpeed;
    
    // Relative offsets from the torso (root)
    const glm::vec3 headOffset;
    const glm::vec3 leftArmOffset;
    const glm::vec3 rightArmOffset;
    const glm::vec3 leftLegOffset;
    const glm::vec3 rightLegOffset;

    // Private helper method
    void drawPart(GLuint shaderProgram, GLuint VAO, const glm::mat4& model, 
                 const glm::mat4& view, const glm::mat4& projection, 
                 const glm::vec3& color);
};

#endif 