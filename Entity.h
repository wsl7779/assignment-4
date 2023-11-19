/**
* Author: Will Lee
* Assignment: Rise of the AI
* Date due: 2023-11-18, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Map.h"

enum EntityType { PLAYER, ENEMY };
enum AIType { WALKER, JUMPER, BOSS };
enum AIState { CHILL, HUNGO, JUMP, OHO, BOSSFIGHT };


class Entity
{
private:
    bool m_is_active = true;
    EntityType m_type;
    AIType m_aitype;
    AIState m_aistate;

    // ––––– ANIMATION ––––– //
    int* m_animation_right = NULL, // move to the right
        * m_animation_left = NULL, // move to the left
        * m_animation_up = NULL, // move upwards
        * m_animation_down = NULL; // move downwards

    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    // ————— TRANSFORMATIONS ————— //
    float     m_speed;
    glm::vec3 m_movement;
    glm::mat4 m_model_matrix;

    float m_width = 1;
    float m_height = 1;
    float patrol_sec = 1.5f;
    float current_step = 0.0f;
    bool is_left = true;

    float jump_time = 5.0f;
    float current_jump = 0.0f;
    int enemies_killed = 0;
    bool lose = false;

    float current_time = 0.0f;
    float cooldown = 2.0f;
    bool cd = false;

public:

    bool const check_collision(Entity* other) const;
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);

    void const check_collision_y(Map* map);
    void const check_collision_x(Map* map);

    // ————— STATIC VARIABLES ————— //
    static const int SECONDS_PER_FRAME = 4;

    // ————— ANIMATION ————— //

    int m_animation_frames = 0,
        m_animation_index = 0,
        m_animation_cols = 0,
        m_animation_rows = 0;

    int* m_animation_indices = NULL;
    float m_animation_time = 0.0f;

    bool m_is_jumping = false;
    float m_jumping_power = 0;

    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

    GLuint    m_texture_id;

    // ————— METHODS ————— //
    Entity();
    Entity(EntityType input_type);
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map);
    //void follow(float delta_time, Entity* parent);
    void render(ShaderProgram* program);

    void shoot(Entity* player, glm::vec3 aim);
    void got_shot(Entity* parent, float delta_time);

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };
    void toggle_active() { m_is_active = !m_is_active; };

    void ai_activate(Entity* player, Entity* objects, float delta_time);
    void ai_guard(Entity* player);
    void ai_patrol(Entity* player, float delta_time);
    void ai_jump(Entity* player, float delta_time);
    void ai_boss(Entity* player, float delta_time);


    void move_left() { m_movement.x = -1.0f; };
    void move_right() { m_movement.x = 1.0f; };



    // ————— GETTERS ————— //
    glm::vec3 const get_position()     const { return m_position; };
    glm::vec3 const get_velocity()     const { return m_velocity; };
    glm::vec3 const get_acceleration() const { return m_acceleration; };
    glm::vec3 const get_movement()     const { return m_movement; };
    float     const get_jumping_power()  const { return m_jumping_power; };
    EntityType const get_type()     const { return m_type; };
    AIType     const get_ai_type()        const { return m_aitype; };
    AIState    const get_ai_state()       const { return m_aistate; };
    float      const get_speed()          const { return m_speed; };
    int const get_killed() const { return enemies_killed; };
    bool const get_lose() const { return lose; };

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position) { m_position = new_position; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_acceleration(glm::vec3 new_position) { m_acceleration = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    //void const set_scale(glm::vec3 new_scale, float new_height, float new_width) { m_scale = new_scale; m_height = new_height; m_width = new_width; };
    void const set_type(EntityType new_type, bool active) { m_type = new_type; m_is_active = active; };
    void const set_wh(float new_w, float new_h) { m_width = new_w; m_height = new_h; };
    void const set_speed(float new_speed) { m_speed = new_speed; };
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power; };
    void const set_entity_type(EntityType new_entity_type) { m_type = new_entity_type; };
    void const set_ai_type(AIType new_ai_type) { m_aitype = new_ai_type; };
    void const set_ai_state(AIState new_state) { m_aistate = new_state; };

};