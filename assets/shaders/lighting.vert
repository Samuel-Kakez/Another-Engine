#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec2 TexCoords;
out mat3 TBN;
out float ClipSpaceZ;

uniform mat4 model;
uniform vec2 tiling;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
    mat4 inverseProjection;
    mat4 inverseView;
};

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoords = aTexCoords * tiling;

    // Matrice normale pour support du scale non-uniforme
    // Note : pour un engine pur perf, précalculer côté CPU et passer en uniform
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(mat3(model) * aTangent);

    // Ré-orthogonalisation Gram-Schmidt
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    TBN = mat3(T, B, N);

    vec4 clipPos = projection * view * vec4(FragPos, 1.0);
    ClipSpaceZ = clipPos.z;
    gl_Position = clipPos;
}
