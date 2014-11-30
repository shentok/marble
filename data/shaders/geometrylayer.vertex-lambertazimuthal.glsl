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

    float lambda = position.x;
    float phi = position.y;
    float lambdaPrime = centerLongitude;
    float phi1 = centerLatitude;

    float cosC = sin( phi1 ) * sin( phi ) + cos( phi1 ) * cos( phi ) * cos( lambda - lambdaPrime );
    // Prevent division by zero
    if (cosC <= 0.) {
        return;
    }

    float k = sqrt(2. / (1. + cosC));

    // Let (x, y) be the position on the screen of the placemark..
    float x = ( cos( phi ) * sin( lambda - lambdaPrime ) ) * k;
    float y = ( cos( phi1 ) * sin( phi ) - sin( phi1 ) * cos( phi ) * cos( lambda - lambdaPrime ) ) * k;

    x *= radius / sqrt(2.);
    y *= - radius / sqrt(2.);

    // Calculate vertex position in screen space
    gl_Position = viewportMatrix * vec4(x, y, 1., 1.);
}
//! [0]
