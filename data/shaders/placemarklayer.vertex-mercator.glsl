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

float gdInv(float x)
{
    const float a1 = 1.0/6.0;
    const float a2 = 1.0/24.0;
    const float a3 = 61.0/5040.;
    const float a4 = 277.0/72576.0;
    const float a5 = 50521.0/39916800.0;
    const float a6 = 41581.0/95800320.0;
    const float a7 = 199360981.0/1307674368000.0;
    const float a8 = 228135437.0/4184557977600.0;
    const float a9 = 2404879675441.0/121645100408832000.0;
    const float a10 = 14814847529501.0/2043637686868377600.0;
    const float a11 = 69348874393137901.0/25852016738884976640000.0;
    const float a12 = 238685140977801337.0/238634000666630553600000.0;
    const float a13 = 4087072509293123892361.0/10888869450418352160768000000.0;
    const float a14 = 454540704683713199807.0/3209350995912777478963200000.0;
    const float a15 = 441543893249023104553682821.0/8222838654177922817725562880000000.0;
    const float a16 = 2088463430347521052196056349.0/102156677868375135241390522368000000.0;

    float x2 = x * x;
    return x
        + x * x2 * (  a1
        + x2 * ( a2
        + x2 * ( a3
        + x2 * ( a4
        + x2 * ( a5
        + x2 * ( a6
        + x2 * ( a7
        + x2 * ( a8
        + x2 * ( a9
        + x2 * ( a10
        + x2 * ( a11
        + x2 * ( a12
        + x2 * ( a13
        + x2 * ( a14
        + x2 * ( a15
        + x2 * ( a16 ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) );
}

//! [0]
void main()
{
    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = texCoord;

    float rad2Pixel = 2.0 * radius / pi;
    float x = rad2Pixel * (position.x - centerLongitude);
    float y = - rad2Pixel * (gdInv(position.y) - gdInv(centerLatitude));

    // Calculate vertex position in screen space
    gl_Position = viewportMatrix * ( vec4(x, y, 1., 1.)  - vec4(8., 8., 0., 0.) + vec4(16., 16., 1., 1.)*vec4(corner, 0., 0.) );
}
//! [0]
