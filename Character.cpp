#include "Character.h"

Character::Character()
// Initialize offsets for different body parts
    : headOffset(0.0f, 1.0f, 0.0f),
    leftArmOffset(-0.6f, 0.0f, 0.0f),
    rightArmOffset(0.6f, 0.0f, 0.0f),
    leftLegOffset(-0.3f, -1.0f, 0.0f),
    rightLegOffset(0.3f, -1.0f, 0.0f),
    // Initialize animation-related variables
    armSwing(0.0f),
    legSwing(0.0f),
    swingSpeed(7.0f)
{    
    
}

// Method to draw the entire character
void Character::drawCharacter(GLuint shaderProgram, GLuint headVAO, GLuint torsoVAO,GLuint armVAO, GLuint legVAO,
                             const glm::mat4& view, const glm::mat4& projection,const glm::vec3& scale, 
                             const glm::vec3& rotation, const glm::vec3& position) {
    
    // Calculate root transformation matrix
    glm::mat4 rootMatrix = glm::mat4(1.0f);
    rootMatrix = glm::translate(rootMatrix, position);
    rootMatrix = glm::rotate(rootMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    rootMatrix = glm::scale(rootMatrix, scale);

    // Draw torso (root)
    glm::mat4 torsoMatrix = rootMatrix;
    torsoMatrix = glm::scale(torsoMatrix, glm::vec3(0.8f, 1.5f, 0.5f));
    drawPart(shaderProgram, torsoVAO, torsoMatrix, view, projection,
        glm::vec3(0.0f, 0.0f, 1.0f)); // Blue color

    // Draw head
    glm::mat4 headMatrix = rootMatrix;
    headMatrix = glm::translate(headMatrix, headOffset);
    headMatrix = glm::scale(headMatrix, glm::vec3(0.3f, 0.4f, 0.3f));
    drawPart(shaderProgram, headVAO, headMatrix, view, projection,
        glm::vec3(1.0f, 0.8f, 0.6f)); // Skin color

    // Draw left arm with swing animation
    glm::mat4 leftArmMatrix = rootMatrix;
    leftArmMatrix = glm::translate(leftArmMatrix, leftArmOffset);
    leftArmMatrix = glm::translate(leftArmMatrix, glm::vec3(0.0f, 0.75f, 0.0f)); // Move the pivot to top of arm
    leftArmMatrix = glm::rotate(leftArmMatrix, glm::radians(armSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    leftArmMatrix = glm::translate(leftArmMatrix, glm::vec3(0.0f, -0.75f, 0.0f)); // Move back
    leftArmMatrix = glm::scale(leftArmMatrix, glm::vec3(0.2f, 1.5f, 0.2f));
    drawPart(shaderProgram, armVAO, leftArmMatrix, view, projection, glm::vec3(1.0f, 0.8f, 0.6f));

    // Draw right arm with opposite swing animation
    glm::mat4 rightArmMatrix = rootMatrix;
    rightArmMatrix = glm::translate(rightArmMatrix, rightArmOffset);
    rightArmMatrix = glm::translate(rightArmMatrix, glm::vec3(0.0f, 0.75f, 0.0f)); // Move the pivot to top of arm
    rightArmMatrix = glm::rotate(rightArmMatrix, glm::radians(-armSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    rightArmMatrix = glm::translate(rightArmMatrix, glm::vec3(0.0f, -0.75f, 0.0f)); // Move back
    rightArmMatrix = glm::scale(rightArmMatrix, glm::vec3(0.2f, 1.5f, 0.2f));
    drawPart(shaderProgram, armVAO, rightArmMatrix, view, projection, glm::vec3(1.0f, 0.8f, 0.6f));

    // Draw left leg with swing animation
    glm::mat4 leftLegMatrix = rootMatrix;
    leftLegMatrix = glm::translate(leftLegMatrix, leftLegOffset);
    leftLegMatrix = glm::translate(leftLegMatrix, glm::vec3(0.0f, 0.75f, 0.0f)); // Move the pivot to top of leg
    leftLegMatrix = glm::rotate(leftLegMatrix, glm::radians(legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    leftLegMatrix = glm::translate(leftLegMatrix, glm::vec3(0.0f, -0.75f, 0.0f)); // Move back
    leftLegMatrix = glm::scale(leftLegMatrix, glm::vec3(0.3f, 1.5f, 0.3f));
    drawPart(shaderProgram, legVAO, leftLegMatrix, view, projection, glm::vec3(0.0f, 0.0f, 0.0f));

    // Draw right leg with opposite swing animation
    glm::mat4 rightLegMatrix = rootMatrix;
    rightLegMatrix = glm::translate(rightLegMatrix, rightLegOffset);
    rightLegMatrix = glm::translate(rightLegMatrix, glm::vec3(0.0f, 0.75f, 0.0f)); // Move the pivot to top of leg
    rightLegMatrix = glm::rotate(rightLegMatrix, glm::radians(-legSwing), glm::vec3(1.0f, 0.0f, 0.0f));
    rightLegMatrix = glm::translate(rightLegMatrix, glm::vec3(0.0f, -0.75f, 0.0f)); // Move back
    rightLegMatrix = glm::scale(rightLegMatrix, glm::vec3(0.3f, 1.5f, 0.3f));
    drawPart(shaderProgram, legVAO, rightLegMatrix, view, projection, glm::vec3(0.0f, 0.0f, 0.0f));
}

// Method to draw individual body parts
void Character::drawPart(GLuint shaderProgram, GLuint VAO, const glm::mat4& model,
    const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& color) {
    // Use the shader program
    glUseProgram(shaderProgram);

    // Get uniform locations for transformation matrices
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // Set uniform values for transformation matrices
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Set the object color
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), color.r, color.g, color.b);

    // Bind VAO and draw the part
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Method to update the swing animation of the character
void Character::updateSwing(float deltaTime, bool isMoving) {
    if (isMoving) {
        // Calculate arm and leg swing based on time and swing speed
        armSwing = 45.0f * sin(glfwGetTime() * swingSpeed);
        legSwing = 30.0f * sin(glfwGetTime() * swingSpeed);
    }
    else {
        // Reset swing to neutral position when not moving
        armSwing = 0.0f;
        legSwing = 0.0f;
    }
}