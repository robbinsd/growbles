// This is a texture sampler.  It lets you sample textures!  The keyword
// "uniform" means constant - sort of.  The uniform variables are the same
// for all fragments in an object, but they can change in between objects.

uniform sampler2D spriteMap; // Set to texture sampler 0
uniform sampler2D diffuseMap; // Set to texture sampler 1
uniform sampler2D specularMap; // Set to texture sampler 2
uniform sampler2D normalMap; // Set to texture sampler 3
uniform sampler2D shadowMap; // Set to texture sampler 4
uniform samplerCube envMap; // Set to texture sampler 5

// Diffuse, ambient, and specular materials.  These are also uniform.
uniform vec3 Kd;
uniform vec3 Ks;
uniform vec3 Ka;
uniform float alpha;

// We need the inverse view matrix for environment mapping
uniform mat3 inverseViewMatrix;

// Are we doing the shadow pass?
uniform bool shadowPass;

// Boolean telling us whether we're rendering particles rather than triangles
uniform bool renderParticles;

// Which type of particle are we rendering?
// 1: fire
// 2: smoke
// 3: gun particles
uniform int particleType;

// Are we doing normal mapping?
uniform bool mapNormals;

// Are we doing environment mapping?
uniform bool mapEnvironment;

varying vec2 texcoord;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;
varying vec3 eyePosition;
varying vec3 lightspacePosition;
varying float particleAge;

vec3 mappedNormal() {

    // Sample the normal
    vec3 normSamp = texture2D(normalMap, texcoord).rgb;

    // Decompress
    vec3 decompressed = normSamp * 2.0 - 1.0;

    // Transform first by the TBN matrix, then by the lighting matrix
    mat3 tbn = mat3(tangent, bitangent, normal);
    return tbn * decompressed;
}

vec3 shadeFromLight(in int lightNum, in vec3 N, in vec3 L, in vec3 V) {

    // Calculate parameters
    float Rd = max(0.0, dot(L, N));
    vec3 R = reflect(-L, N);
    float Rs = pow(max(0.0, dot(V, R)), alpha);

    // Sample textures
    vec3 Td, Ts, Ta;
    if (mapEnvironment) {
        vec3 cubeCoords = vec3(inverseViewMatrix * reflect(-V, N));
        Td = Ts = Ta = textureCube(envMap, cubeCoords).rgb;
    }
    else {
        Td = texture2D(diffuseMap, texcoord).rgb;
        Ts = texture2D(specularMap, texcoord).rgb;
        Ta = vec3(1, 1, 1);
    }

    // Calculate the diffuse color coefficient, and sample the diffuse texture
    vec3 diffuse = Rd * Kd * Td * gl_LightSource[lightNum].diffuse.rgb;

    // Calculate the specular coefficient
    vec3 specular = Rs * Ks * Ts * gl_LightSource[lightNum].specular.rgb;

    // Ambient is easy
    vec3 ambient = Ka * Ta * gl_LightSource[lightNum].ambient.rgb;

    return diffuse + specular + ambient;
}

float computeParticleAlpha(float age) {

    // Fire
    if (particleType == 1) {
        return (0.3) * (1.0 - age);
    }

    // Smoke
    if (particleType == 2) {

        // Alpha increases quickly after birth...
        if (age < 0.1)
            return 7.0 * age;

        // ...then decreases slowly to zero.
        return (1.0 - age) * 0.7;
    }

    // Gun
    else {
        return 1.0;
    }
}

void main() {

    // If we're doing the shadow pass, all we care about is the depth buffer,
    // which is taken care of automatically. Just write a solid color and be
    // done.
    if (shadowPass) {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        return;
    }

    // Particle rendering
    if (renderParticles) {
        vec4 texSample = texture2D(spriteMap,  gl_TexCoord[0].xy);
        gl_FragColor = vec4(texSample.rgb, texSample.a * computeParticleAlpha(particleAge));
        return;
    }

    // Do normal mapping, if enabled
    vec3 N;
    if (mapNormals)
        N = normalize(mappedNormal());
    else
        N = normalize(normal);

    // Calculate the view vector
    vec3 V = normalize(-eyePosition);

    vec3 L0 = normalize(-gl_LightSource[0].position.xyz);
    vec3 L1 = normalize(gl_LightSource[1].position.xyz - eyePosition);

    // Determine if the vertex is in shadow for the directional light
    vec2 shadowCoord = vec2(lightspacePosition.x * 0.5 + 0.5,
                            lightspacePosition.y * 0.5 + 0.5);
    float shadowZ = texture2D(shadowMap, shadowCoord).r * 2.0 - 1.0;
    bool inShadow = (shadowZ < lightspacePosition.z - 0.01);

    // The contributions of lights are additive
    vec3 light0Contrib = inShadow ? vec3(0.0, 0.0, 0.0) : shadeFromLight(0, N, L0, V);
    vec3 light1Contrib = shadeFromLight(1, N, L1, V);

    // This actually writes to the frame buffer
    gl_FragColor = vec4(light0Contrib + light1Contrib, 1);
}
