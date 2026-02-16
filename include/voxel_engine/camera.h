#pragma once

// This code was made following LearnOpenGL course.
// you can find it here: https://learnopengl.com/Getting-started/Camera


#include "voxel_engine/math_utils.h"

enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

#include "config.h"

#ifdef RENDERING_OPENGL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#endif

using namespace voxeng::math;

class Camera {
  // cam attributes
  Vec3f m_pos;
  Vec3f m_right;
  Vec3f m_world_up;
  Vec3f m_up;
  Vec3f m_front; //TODO: use this
  // euleur angles
  float m_yaw = DEFAULT_CAM_YAW;
  float m_pitch = DEFAULT_CAM_PITCH;
  // cam options
  float m_movement_speed = CAMERA_MOV_SPEED;
  float m_mouse_sensitivity = MOUSE_SENSITIVITY; //TODO: maybe move this in voxelengine parameter (runtime)
  float m_zoom = CAMERA_DEFAULT_ZOOM;

  unsigned int m_screen_width = 800; // Default
  unsigned int m_screen_height = 600; // Default

  void updateCameraVectors()
  {
    Vec3f front{};
    front.x = cos(deg_to_rad(m_yaw)) * cos(deg_to_rad(m_pitch));
    front.y = sin(deg_to_rad(m_pitch));
    front.z = sin(deg_to_rad(m_yaw)) * cos(deg_to_rad(m_pitch));
    m_front = Vec3f::normalize(front);
    // also re-calculate the Right and Up vector
    m_right = Vec3f::normalize(Vec3f::cross(m_front, m_world_up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    m_up    = Vec3f::normalize(Vec3f::cross(m_right, m_front));
  }

public:
  Camera(Vec3f position, unsigned int width, unsigned int height): 
    m_pos(position),
    m_world_up({0.0f, 1.0f, 0.0f}),
    m_screen_width(width),
    m_screen_height(height) {
    updateCameraVectors();
  }

  void setViewportSize(unsigned int width, unsigned int height) {
    m_screen_width = width;
    m_screen_height = height;
  }

  unsigned int getWidth() const { return m_screen_width; }
  unsigned int getHeight() const { return m_screen_height; }

  #ifdef RENDERING_OPENGL
  glm::mat4 getViewMatrix() const
  {
    glm::vec3 pos(m_pos.x, m_pos.y, m_pos.z);
    glm::vec3 front(m_front.x, m_front.y, m_front.z);
    glm::vec3 up(m_up.x, m_up.y, m_up.z);
    return glm::lookAt(pos, pos + front, up);
  }

  glm::mat4 getProjectionMatrix() const {
    return glm::perspective(
      glm::radians(m_zoom),
      (float)m_screen_width / (float)m_screen_height,
      0.1f,
      3000.0f
    );
  }
  #endif

  void processMovement(Movement mov, float deltaTime) {
    float velocity = m_movement_speed * deltaTime;
    switch (mov) {
      case FORWARD: {
        m_pos += m_front * velocity;
        break;
      }
      case BACKWARD: {
        m_pos -= m_front * velocity;
        break;
      }
      case LEFT: {
        m_pos -= m_right * velocity;
        break;
      }
      case RIGHT: {
        m_pos += m_right * velocity;
        break;
      }
      case UP: {
        m_pos += m_up * velocity;
        break;
      }
      case DOWN: {
        m_pos -= m_up * velocity;
      }
    }
  }

  void processEulerMovement(float xoffset, float yoffset, bool constrain_pitch = true)
  {
    xoffset *= m_mouse_sensitivity;
    yoffset *= m_mouse_sensitivity;

    m_yaw   += xoffset;
    m_pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrain_pitch)
    {
      if (m_pitch > 89.0f)
        m_pitch = 89.0f;
      if (m_pitch < -89.0f)
        m_pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
  }

  void processZoom(float yoffset) {
    m_zoom -= (float)yoffset;
    if (m_zoom < 1.0f)
      m_zoom = 1.0f;
    if (m_zoom > 80.0f)
      m_zoom = 80.0f;
  }

  float getZoom() const {
    return m_zoom;
  }

  Vec3f getPos() const {
    return m_pos;
  }

  Vec3f getFront() const {
    return m_front;
  }

};
