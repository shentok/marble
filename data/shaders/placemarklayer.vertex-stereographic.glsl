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

//! [0]
void main()
{
    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = texCoord;
  
    float lambda = position.x;
    float phi = position.y;
    float lambdaPrime = centerLongitude;
    float phi1 = centerLatitude;

    float cosC = sin( phi1 ) * sin( phi ) + cos( phi1 ) * cos( phi ) * cos( lambda - lambdaPrime );

    if ( cosC <= 0. ) {
        return;
    }

    float k = 1. / (1. + cosC);

    // Let (x, y) be the position on the screen of the placemark..
    float x = ( cos( phi ) * sin( lambda - lambdaPrime ) ) / k;
    float y = ( cos( phi1 ) * sin( phi ) - sin( phi1 ) * cos( phi ) * cos( lambda - lambdaPrime ) ) / k;

    x = x * radius / 2.;
    y = - y * radius / 2.;

    // Calculate vertex position in screen space
    gl_Position = viewportMatrix * ( vec4(x, y, 1., 1.0)  - vec4(8., 8., 0., 0.) + vec4(16., 16., 1., 1.)*vec4(corner, 0., 0.) );
}
//! [0]
