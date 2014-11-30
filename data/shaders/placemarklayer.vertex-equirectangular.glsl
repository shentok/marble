#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform float centerLongitude;
uniform float centerLatitude;
uniform float radius;
uniform mat4 viewportMatrix;

attribute vec3 position;
attribute vec2 texCoord;
attribute vec2 corner;

varying vec2 v_texcoord;

const float pi = 3.14159265358979323846264;

//! [0]
void main()
{
    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = texCoord;

    float x =   2.0 * (position.x - centerLongitude) / pi * radius;
    float y = - 2.0 * (position.y - centerLatitude) / pi * radius;

    // Calculate vertex position in screen space
    gl_Position = viewportMatrix * ( vec4(x, y, 1., 1.) - vec4(8., 8., 0., 0.) + vec4(16., 16., 1., 1.)*vec4(corner, 0., 0.) );
}
//! [0]
