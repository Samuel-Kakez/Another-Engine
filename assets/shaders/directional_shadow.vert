#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;

// Le vertex shader ne fait que transformer en world-space.
// Le geometry shader applique la lightSpaceMatrix pour chaque cascade.
void main()
{
    gl_Position = model * vec4(aPos, 1.0);
}
