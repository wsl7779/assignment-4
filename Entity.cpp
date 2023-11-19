/**
* Author: Will Lee
* Assignment: Rise of the AI
* Date due: 2023-11-18, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"
#include "Map.h"

Entity::Entity()
{


    // ––––– PHYSICS ––––– //
    m_position = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);

    // ––––– TRANSLATION ––––– //
    m_movement = glm::vec3(0.0f);
    m_speed = 0;
    m_model_matrix = glm::mat4(1.0f);
}

Entity::Entity(EntityType input_type)
{
    m_type = input_type;

    // ––––– PHYSICS ––––– //
    m_position = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_acceleration = glm::vec3(0.0f);

    // ––––– TRANSLATION ––––– //
    m_movement = glm::vec3(0.0f);
    m_speed = 0;
    m_model_matrix = glm::mat4(1.0f);
}


Entity::~Entity()
{
    delete[] m_animation_up;
    delete[] m_animation_down;
    delete[] m_animation_left;
    delete[] m_animation_right;
}

bool const Entity::check_collision(Entity* other) const
{
    if (other == this) return false;
    if (!m_is_active || !other->m_is_active) return false;

    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}


void Entity::update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map)
{
    if (!m_is_active) return;

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    if (m_type == ENEMY) { ai_activate(player, objects, delta_time); }

    if (m_animation_indices != NULL)
    {
        if (glm::length(m_movement) != 0)
        {
            m_animation_time += delta_time;
            float frames_per_second = (float)1 / SECONDS_PER_FRAME;

            if (m_animation_time >= frames_per_second)
            {
                m_animation_time = 0.0f;
                m_animation_index++;

                if (m_animation_index >= m_animation_frames)
                {
                    m_animation_index = 0;
                }
            }
        }
    }


    m_velocity.x = m_movement.x * m_speed;
    m_velocity += m_acceleration * delta_time;

    // We make two calls to our check_collision methods, one for the collidable objects and one for
    // the map.
    m_position.y += m_velocity.y * delta_time;
    check_collision_y(objects, object_count);
    check_collision_y(map);

    m_position.x += m_velocity.x * delta_time;
    check_collision_x(objects, object_count);
    check_collision_x(map);

    if (m_is_jumping)
    {
        m_is_jumping = false;

        m_velocity.y += m_jumping_power;
    }

    // Apply transformation
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
}

//void Entity::follow(float delta_time, Entity* parent)
//{
//    m_model_matrix = glm::mat4(1.0f);
//    m_model_matrix = glm::translate(m_model_matrix, parent->m_position);
//    m_model_matrix = glm::translate(m_model_matrix, glm::vec3(0.0f, -0.5f, 0.0f));
//}

void Entity::ai_activate(Entity* player, Entity* objects, float delta_time)
{
    switch (m_aitype)
    {
    case WALKER:
        ai_patrol(objects, delta_time);
        break;
    case JUMPER:
        ai_jump(objects, delta_time);
        break;
    case BOSS:
        ai_boss(objects, delta_time);
        break;
    default:
        break;
    }
}

void Entity::ai_patrol(Entity* player, float delta_time)
{
    switch (m_aistate) {
    case CHILL:
        if (current_step > patrol_sec) {
            is_left = !is_left;
            current_step = 0;
        }
        if (is_left) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
            current_step += delta_time;
        }
        else {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
            current_step += delta_time;
        }
        if (player->m_type == PLAYER) {
            if (glm::distance(m_position, player->m_position) < 1.5f) {m_aistate = HUNGO; }
        }
        break;

    case HUNGO:
        if (m_position.x > player->m_position.x) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        }
        else {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        break;


    default:
        break;
    }
}

void Entity::ai_jump(Entity* player, float delta_time)
{
    switch (m_aistate) {
    case JUMP:
        if (current_jump > jump_time) {
            m_velocity.y += m_jumping_power;
            current_jump = 0;
        }
        break;
    default:
        break;
    }
    current_jump += delta_time;
}

void Entity::ai_boss(Entity* player, float delta_time)
{
        switch (m_aistate) {
            if (current_time > cooldown) {
                cd = false;
            }
        case BOSSFIGHT:
                if (glm::distance(m_position, player->get_position()) < 3.0f && !cd)
                {
                    m_aistate = OHO;
                }
            break;

        case OHO:
            if (!cd) {
                m_position.x = player->m_position.x;
                m_position.y = player->m_position.y + 4.0f;
                current_time = 0;
                cd = true;
                m_aistate = BOSSFIGHT;
            }
            break;
        }
        if (cd) {
            current_time += delta_time;
        }
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0) {
                m_position.y -= y_overlap;
                m_velocity.y = 0;

                // Collision!
                m_collided_top = true;
                if (m_type == PLAYER && collidable_entity->m_type == ENEMY) {
                    lose = true;
                }
            }
            else if (m_velocity.y < 0) {
                m_position.y += y_overlap;
                m_velocity.y = 0;
                m_velocity.x = 0;

                // Collision!
                m_collided_bottom = true;
                if (m_type == PLAYER && collidable_entity->m_type == ENEMY) {
                    collidable_entity->m_is_active = false;
                    enemies_killed++;
                }
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    if (!m_is_active) return;
        for (int i = 0; i < collidable_entity_count; i++)
        {
            Entity* collidable_entity = &collidable_entities[i];

            if (check_collision(collidable_entity))
            {
                float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
                float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
                if (m_velocity.x > 0) {
                    m_position.x -= x_overlap;
                    m_velocity.x = 0;

                    // Collision!
                    m_collided_right = true;
                    if (m_type == PLAYER && collidable_entity->m_type == ENEMY) {
                        lose = true;
                    }
                }
                else if (m_velocity.x < 0) {
                    m_position.x += x_overlap;
                    m_velocity.x = 0;

                    // Collision!
                    m_collided_left = true;
                    if (m_type == PLAYER && collidable_entity->m_type == ENEMY) {
                        lose = true;
                    }
                }
            }
        }
}

void const Entity::check_collision_y(Map* map)
{
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);

    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }

    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;

    }
}

void const Entity::check_collision_x(Map* map)
{
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}

void Entity::render(ShaderProgram* program)
{
    if (!m_is_active) return;

    program->set_model_matrix(m_model_matrix);

    if (m_animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, m_texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}