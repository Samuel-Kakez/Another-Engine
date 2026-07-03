#version 430 core

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;
in float ClipSpaceZ;

// ============================================================================
// STRUCTS & UNIFORMS
// ============================================================================

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
    bool castsShadows;
};

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    sampler2D albedoTexture;
    bool hasAlbedoTexture;
    sampler2D normalTexture;
    bool hasNormalTexture;
    float normalMapIntensity;
};

uniform Material material;
uniform DirectionalLight dirLight;

// Cascaded Shadow Mapping
const int NUM_CASCADES = 4;
uniform sampler2DArray shadowMapArray;
uniform mat4 lightSpaceMatrices[NUM_CASCADES];
uniform float cascadePlaneDistances[NUM_CASCADES];

// Shadow softness: radius in texels to spread the Poisson samples over.
// 1.0-2.0 gives a subtle, cheap blur. 3.0+ gets noticeably soft.
uniform float shadowSoftness;

uniform vec3 camPos;
uniform float exposure;

// Hemispheric ambient
uniform vec3 skyColorZenith;
uniform vec3 skyColorHorizon;
uniform vec3 groundColor;
uniform float ambientIntensity;

const float PI = 3.14159265359;

// ============================================================================
// POISSON DISK (12 samples — good quality/perf sweet spot)
// ============================================================================

const int POISSON_SAMPLES = 12;
const vec2 poissonDisk[12] = vec2[](
    vec2(-0.9262, -0.0972),
    vec2(-0.4593,  0.5765),
    vec2( 0.1710, -0.8880),
    vec2( 0.5765,  0.2389),
    vec2(-0.3210, -0.4150),
    vec2( 0.7948,  0.7570),
    vec2(-0.6912,  0.2260),
    vec2( 0.0382, -0.3510),
    vec2( 0.4420, -0.5102),
    vec2(-0.1580,  0.8685),
    vec2( 0.8322, -0.2130),
    vec2(-0.7610, -0.6340)
);

// ============================================================================
// PBR (Cook-Torrance)
// ============================================================================

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a2 = roughness * roughness * roughness * roughness;
    float NdotH = max(dot(N, H), 0.0);
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return (NdotV / (NdotV * (1.0 - k) + k)) * (NdotL / (NdotL * (1.0 - k) + k));
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ============================================================================
// CASCADED POISSON-DISK SHADOW
// ============================================================================

// Cheap per-fragment pseudo-random rotation to break up sampling patterns.
// Uses screen-space coords so the noise is stable in screen space (no shimmer).
float InterleavedGradientNoise(vec2 screenPos) {
    return fract(52.9829189 * fract(dot(screenPos, vec2(0.06711056, 0.00583715))));
}

float CascadedShadowTest(vec3 fragWorldPos, vec3 normal, vec3 lightDir) {
    // Pick cascade
    int cascadeIndex = NUM_CASCADES - 1;
    for (int i = 0; i < NUM_CASCADES; ++i) {
        if (ClipSpaceZ < cascadePlaneDistances[i]) {
            cascadeIndex = i;
            break;
        }
    }

    // Project into this cascade's light space
    vec4 fragPosLightSpace = lightSpaceMatrices[cascadeIndex] * vec4(fragWorldPos, 1.0);
    vec3 coords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    coords = coords * 0.5 + 0.5;

    if (coords.z > 1.0 || coords.x < 0.0 || coords.x > 1.0
        || coords.y < 0.0 || coords.y > 1.0)
        return 0.0;

    // Adaptive bias per cascade
    float baseBias = 0.00001;
    float bias = baseBias * float(cascadeIndex + 1);

    // ---- Poisson disk sampling ----
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMapArray, 0).xy);

    // Fixed spread in texels — NOT scaled by cascade index.
    // Each cascade's shadow map covers a different world-space range but has the
    // same resolution, so the same texel-space radius naturally produces a tighter
    // world-space blur on closer cascades and a proportional one on far cascades.
    // This avoids the "enormous blur on cascade 3" problem.
    vec2 radius = texelSize * shadowSoftness;

    // Rotate the entire disk per-fragment to hide the repeated pattern
    float angle = InterleavedGradientNoise(gl_FragCoord.xy) * 2.0 * PI;
    float s = sin(angle);
    float c = cos(angle);
    mat2 rotation = mat2(c, s, -s, c);

    float shadow = 0.0;
    for (int i = 0; i < POISSON_SAMPLES; ++i) {
        vec2 offset = rotation * poissonDisk[i] * radius;
        float depth = texture(shadowMapArray, vec3(coords.xy + offset, float(cascadeIndex))).r;
        shadow += (coords.z - bias > depth) ? 1.0 : 0.0;
    }
    shadow /= float(POISSON_SAMPLES);

    // Fade the last cascade edge to avoid hard cutoff
    float fadeFactor = 1.0;
    if (cascadeIndex == NUM_CASCADES - 1) {
        float edgeDist = max(
            max(abs(coords.x - 0.5), abs(coords.y - 0.5)),
            abs(coords.z - 0.5)
        );
        fadeFactor = 1.0 - smoothstep(0.4, 0.5, edgeDist);
    }

    return shadow * fadeFactor;
}

// ============================================================================
// TONE MAPPING (ACES Filmic)
// ============================================================================

vec3 ACESFilm(vec3 x) {
    return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
}

// ============================================================================
// MAIN
// ============================================================================

void main() {
    // Material
    vec3 albedo = material.albedo;
    if (material.hasAlbedoTexture)
        albedo *= texture(material.albedoTexture, TexCoords).rgb;

    float metallic = material.metallic;
    float roughness = max(material.roughness, 0.04);

    // Normal
    vec3 N = normalize(TBN[2]);
    if (material.hasNormalTexture) {
        vec3 tn = texture(material.normalTexture, TexCoords).rgb * 2.0 - 1.0;
        tn.xy *= material.normalMapIntensity;
        N = normalize(TBN * tn);
    }

    vec3 V = normalize(camPos - FragPos);
    vec3 L = normalize(-dirLight.direction);
    vec3 H = normalize(V + L);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // PBR
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 spec = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
    vec3 kD = (1.0 - F) * (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);

    // Shadow
    float shadow = dirLight.castsShadows ? CascadedShadowTest(FragPos, N, L) : 0.0;

    // Direct lighting
    vec3 radiance = dirLight.color * dirLight.intensity;
    vec3 direct = (kD * albedo / PI + spec) * radiance * NdotL * (1.0 - shadow);

    // Hemispheric ambient
    float skyBlend = N.y * 0.5 + 0.5;
    vec3 skyIrradiance = mix(groundColor, mix(skyColorHorizon, skyColorZenith, skyBlend), skyBlend);
    vec3 ambient = skyIrradiance * albedo * ambientIntensity;

    // Tone mapping + output (gamma via GL_FRAMEBUFFER_SRGB)
    FragColor = vec4(ACESFilm((ambient + direct) * exposure), 1.0);
}
