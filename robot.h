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

    Robot(const std::string& path, glm::vec3 startPos)
        : body(path), position(startPos), rotationY(0.0f) {
    }

    void moveTo(glm::vec3 target, float speed) {
        glm::vec3 dir = glm::normalize(target - position);
        position += dir * speed;
        rotationY = glm::degrees(atan2(dir.x, dir.z));
    }

    bool isNear(glm::vec3 target, float threshold = 1.0f) {
        return glm::distance(position, target) < threshold;
    }


    void draw(Shader& shader) {
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, position);
        modelMat = glm::rotate(modelMat, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMat = glm::scale(modelMat, glm::vec3(0.5f));
        shader.setMat4("model", modelMat);

        glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), true);
        glUniform3f(glGetUniformLocation(shader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
        body.Draw(shader);
    }
};

#endif
