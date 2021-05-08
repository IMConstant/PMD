//
// Created by Koko on 05.04.2021.
//

#include "Mesh.h"


float tetrahedronSignedVolume(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
    return glm::dot(v0, glm::cross(v1, v2)) / 6.0f;
}
