#include "Frustum.h"

float Plane::getSignedDistance(const glm::vec3 &point) const
{
    return glm::dot(normal, point) + distance;
}

void Frustum::update(const glm::mat4 &m)
{

    planes[0].normal.x = m[0][3] + m[0][0];
    planes[0].normal.y = m[1][3] + m[1][0];
    planes[0].normal.z = m[2][3] + m[2][0];
    planes[0].distance = m[3][3] + m[3][0];

    planes[1].normal.x = m[0][3] - m[0][0];
    planes[1].normal.y = m[1][3] - m[1][0];
    planes[1].normal.z = m[2][3] - m[2][0];
    planes[1].distance = m[3][3] - m[3][0];

    planes[2].normal.x = m[0][3] - m[0][1];
    planes[2].normal.y = m[1][3] - m[1][1];
    planes[2].normal.z = m[2][3] - m[2][1];
    planes[2].distance = m[3][3] - m[3][1];

    planes[3].normal.x = m[0][3] + m[0][1];
    planes[3].normal.y = m[1][3] + m[1][1];
    planes[3].normal.z = m[2][3] + m[2][1];
    planes[3].distance = m[3][3] + m[3][1];

    planes[4].normal.x = m[0][3] + m[0][2];
    planes[4].normal.y = m[1][3] + m[1][2];
    planes[4].normal.z = m[2][3] + m[2][2];
    planes[4].distance = m[3][3] + m[3][2];

    planes[5].normal.x = m[0][3] - m[0][2];
    planes[5].normal.y = m[1][3] - m[1][2];
    planes[5].normal.z = m[2][3] - m[2][2];
    planes[5].distance = m[3][3] - m[3][2];

    for (auto &plane : planes)
    {
        float length = glm::length(plane.normal);
        plane.normal /= length;
        plane.distance /= length;
    }
}

bool Frustum::intersects(const AABB &aabb) const
{
    for (const auto &plane : planes)
    {
        const glm::vec3 p(
            plane.normal.x < 0 ? aabb.min.x : aabb.max.x,
            plane.normal.y < 0 ? aabb.min.y : aabb.max.y,
            plane.normal.z < 0 ? aabb.min.z : aabb.max.z);
        if (glm::dot(plane.normal, p) + plane.distance < 0)
            return false;
    }

    return true;
}