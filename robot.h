#ifndef ROBOT_H
#define ROBOT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "model.h"
#include "shaderClass.h"

class Robot {
public:
    glm::vec3 position;
    float rotationY;
    Model body;
    Model arm;

    Robot(const std::string& bodyPath, const std::string& armPath, glm::vec3 startPos)
        : body(bodyPath), arm(armPath), position(startPos), rotationY(0.0f) {
    }


    void moveTo(glm::vec3 target, float speed) {
        glm::vec3 dir = glm::normalize(target - position);
        position += dir * speed;
        rotationY = glm::degrees(atan2(dir.x, dir.z));
    }

    bool isNear(glm::vec3 target, float threshold = 1.0f) {
        return glm::distance(position, target) < threshold;
    }


    void draw(Shader& shader, float armAngle) {
        // === GÖVDE ===
        glm::mat4 bodyMat = glm::mat4(1.0f);
        bodyMat = glm::translate(bodyMat, position + glm::vec3(0.0f, 0.6f, 0.0f));
        bodyMat = glm::rotate(bodyMat, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        bodyMat = glm::scale(bodyMat, glm::vec3(0.5f));
        shader.setMat4("model", bodyMat);
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), false);
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.6f, 0.6f, 0.6f);
        body.Draw(shader);

        // === KOL (gövdeye bağlı) ===
        glm::mat4 armMat = glm::mat4(1.0f);

        armMat = glm::translate(armMat, glm::vec3(-0.030f, -0.015f, -0.02f));

        armMat = glm::rotate(armMat, glm::radians(-armAngle), glm::vec3(1.0f, 0.0f, 0.0f));

        armMat = glm::scale(armMat, glm::vec3(1.0f));

        armMat = bodyMat * armMat;


        shader.setMat4("model", armMat);
        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), false);
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 0.6f, 0.6f, 0.6f);
        arm.Draw(shader);
    }


};

#endif
