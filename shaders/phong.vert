
// These are the "input" to our shader.  They are read from the vertex
// arrays that we specified in the C++ code.
attribute vec3 positionIn;
attribute vec2 texcoordIn;
attribute vec3 normalIn;
attribute vec3 tangentIn;
attribute vec3 bitangentIn;
attribute float particleAgeIn;

// The model matrix, separate from the view matrix
uniform mat4 modelMatrix;

// The light matrix
uniform mat4 lightMatrix;

// Boolean telling us whether we're doing a "dead simple" shadow pass
uniform bool shadowPass;

// Boolean telling us whether we're rendering particles rather than triangles
uniform bool renderParticles;

// Which type of particle are we rendering?
// 1: fire
// 2: smoke
// 3: gun particles
uniform int particleType;

// The width of the viewport
uniform float viewportWidth;

// Outputs
varying vec2 texcoord;
varying vec3 normal;
varying vec3 tangent;
varying vec3 bitangent;
varying vec3 eyePosition;
varying vec3 lightspacePosition;
varying float particleAge;

float computeParticleBaseSize(float age) {

    // fire
    if (particleType == 1) {
        return 0.5 * (1.0 - age);
    }

    // Smoke
    if (particleType == 2) {
        return 0.8;
    }

    // gun
    else {
        return 0.3;
    }
}

void main() {

    /*
     * Shadow pass handling.
     *
     * For the shadow pass, we want to make the following vertex transformations:
     * 1 - Model matrix
     * 2 - Light View
     * 3 - Light Projection
     *
     * Since we have 1 as modelMatrix and 2+3 as lightMatrix, we just ignore the
     * modelview matrix here. Note that not setting this would normally be a
     * problem since gl_NormalMatrix depends on gl_ModelView, but we don't care
     * about normals for the shadow pass.
     */
    if (shadowPass) {

        // Transform the vertex by the light modelview and the light projection.
        gl_Position = lightMatrix * modelMatrix * vec4(positionIn, 1);

        // All done for the shadow pass
        return;
    }

    // Transform the vertex to get the eye-space position of the vertex
    vec4 eyeTemp = gl_ModelViewMatrix * vec4(positionIn, 1);
    eyePosition = eyeTemp.xyz;

    // Transform again to get the clip-space position.  The gl_Position
    // variable tells OpenGL where the vertex should go.
    gl_Position = gl_ProjectionMatrix * eyeTemp;

    // particle handling
    if (renderParticles) {
        float baseSize = computeParticleBaseSize(particleAgeIn);
        gl_PointSize = baseSize * (viewportWidth / (1.0 + length(eyePosition.xyz)));
        particleAge = particleAgeIn;
    }

    // Transform the normal and friends
    normal = gl_NormalMatrix * normalIn;
    tangent = gl_NormalMatrix * tangentIn;
    bitangent = gl_NormalMatrix * bitangentIn;

    // If we're rendering particles, use the gl texture coordinates. Otherwise
    // use the attributes.
    texcoord = texcoordIn;

    // Calculate the lightspace position. Bias all 3 coordinates into the
    // range [0, 1]
    lightspacePosition = (lightMatrix * modelMatrix * vec4(positionIn, 1)).xyz;
}
