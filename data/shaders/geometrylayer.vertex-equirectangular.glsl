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
attribute vec4 color;

varying vec4 v_color;

const float pi = 3.14159265358979323846264;

//! [0]
void main()
{
    // Pass through the color to the fragment shader
    v_color = color;

    float x =   2.0 * (position.x - centerLongitude) / pi * radius;
    float y = - 2.0 * (position.y - centerLatitude) / pi * radius;

    // Calculate vertex position in screen space
    gl_Position = viewportMatrix * vec4(x, y, 1., 1.);
}
//! [0]
