#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

varying vec4 v_color;

void main()
{
    // The v_color variable is interpolated to this fragment's position
    // for us by OpenGL.
    gl_FragColor = v_color;
}
