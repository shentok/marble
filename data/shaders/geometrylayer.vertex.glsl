#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

attribute vec3 position;
attribute vec4 color;

varying vec4 v_color;

uniform mat4 rotationMatrix;

void main()
{
    // Pass through the color to the fragment shader
    v_color = color;

    // Calculate the vertex position
    gl_Position = rotationMatrix * vec4( position, 1.0 );
}
