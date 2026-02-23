#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in mat3 TBN;
in float ClipSpaceZ;

// ============================================================================
// STRUCTS et UNIFORMS
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

uniform vec3 camPos;
uniform float exposure;

// Ambient hémisphérique
uniform vec3 skyColorZenith;
uniform vec3 skyColorHorizon;
uniform vec3 groundColor;
uniform float ambientIntensity;

const float PI = 3.14159265359;

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
// CASCADED SOFT SHADOW - Poisson Disk Sampling (Rotated)
// ============================================================================

// Distribution de Poisson (16 samples)
const vec2 poissonDisk[16] = vec2[](
   vec2( -0.94201624, -0.39906216 ), vec2( 0.94558609, -0.76890725 ),
   vec2( -0.094184101, -0.92938870 ), vec2( 0.34495938, 0.29387760 ),
   vec2( -0.91588581, 0.45771432 ), vec2( -0.81544232, -0.87912464 ),
   vec2( -0.38277543, 0.27676845 ), vec2( 0.97484398, 0.75648379 ),
   vec2( 0.44323325, -0.97511554 ), vec2( 0.53742981, -0.47373420 ),
   vec2( -0.26496911, -0.41893023 ), vec2( 0.79197514, 0.19090188 ),
   vec2( -0.24188840, 0.99706507 ), vec2( -0.81409955, 0.91437590 ),
   vec2( 0.19984126, 0.78641367 ), vec2( 0.14383161, -0.14100790 )
);

float CascadedShadowTest(vec3 fragWorldPos, vec3 normal, vec3 lightDir) {
    // Sélection de la cascade appropriée basée sur la profondeur clip-space du fragment
    int cascadeIndex = NUM_CASCADES - 1;
    for (int i = 0; i < NUM_CASCADES; ++i) {
        if (ClipSpaceZ < cascadePlaneDistances[i]) {
            cascadeIndex = i;
            break;
        }
    }

    // Projection dans l'espace lumière de cette cascade
    vec4 fragPosLightSpace = lightSpaceMatrices[cascadeIndex] * vec4(fragWorldPos, 1.0);
    vec3 coords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    coords = coords * 0.5 + 0.5;

    // Hors-frustum -> pas d'ombre
    if (coords.z > 1.0 || coords.x < 0.0 || coords.x > 1.0 || coords.y < 0.0 || coords.y > 1.0)
        return 0.0;

    // Bias adaptatif par cascade (les cascades éloignées ont plus de texels donc besoin de plus de bias)
    float baseBias = 0.00001;
    float bias = baseBias * float(cascadeIndex + 1);

    // Stratified Sampling (Rotation aléatoire du disque)
    float randomAngle = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453) * 6.283185;
    float s = sin(randomAngle);
    float c = cos(randomAngle);
    mat2 rot = mat2(c, -s, s, c);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMapArray, 0).xy);

    // Rayon ajusté par cascade : les cascades proches = plus nettes, éloignées = plus douces
    float diskRadius = 2.0 + float(cascadeIndex) * 0.5;

    // PCF 16 samples
    for(int i = 0; i < 16; ++i) {
        vec2 offset = rot * poissonDisk[i] * diskRadius;
        float pcfDepth = texture(shadowMapArray, vec3(coords.xy + offset * texelSize, float(cascadeIndex))).r;
        shadow += (coords.z - bias > pcfDepth) ? 1.0 : 0.0;
    }

    // Fondu entre la dernière cascade et "pas d'ombre" pour éviter une coupure nette
    float fadeFactor = 1.0;
    if (cascadeIndex == NUM_CASCADES - 1) {
        float edgeDist = max(
            max(abs(coords.x - 0.5), abs(coords.y - 0.5)),
            abs(coords.z - 0.5)
        );
        fadeFactor = smoothstep(0.5, 0.4, edgeDist);
    }

    return (shadow / 16.0) * fadeFactor;
}

// ============================================================================
// TONE MAPPING inline (ACES Filmic)
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

    // Ombre (Cascaded Soft Shadow)
    float shadow = dirLight.castsShadows ? CascadedShadowTest(FragPos, N, L) : 0.0;

    // Lumière directe
    vec3 radiance = dirLight.color * dirLight.intensity;
    vec3 direct = (kD * albedo / PI + spec) * radiance * NdotL * (1.0 - shadow);

    // Ambient hémisphérique
    float skyBlend = N.y * 0.5 + 0.5;
    vec3 skyIrradiance = mix(groundColor, mix(skyColorHorizon, skyColorZenith, skyBlend), skyBlend);
    vec3 ambient = skyIrradiance * albedo * ambientIntensity;

    // Tone mapping + sortie (gamma via GL_FRAMEBUFFER_SRGB)
    FragColor = vec4(ACESFilm((ambient + direct) * exposure), 1.0);
}