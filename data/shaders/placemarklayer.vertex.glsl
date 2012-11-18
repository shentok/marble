#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 rotationMatrix;
uniform mat4 projectionMatrix;

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

    // Calculate vertex position in screen space
    gl_Position = projectionMatrix * ( rotationMatrix * vec4(position, 1.0) - vec4(8, 8, 0, 0) + vec4(16, 16, 1, 1)*vec4(corner, 0.0, 0.0) );
}
//! [0]
