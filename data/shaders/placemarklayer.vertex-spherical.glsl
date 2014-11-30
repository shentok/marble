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

    vec4 cv = vec4( cos(-position.y) * sin(position.x) * radius,
                    sin(-position.y) * radius,
                    cos(-position.y) * cos(position.x) * radius,
                    1.0 );

    float cPhi = cos(0.5 * centerLatitude);
    float cThe = cos(0.5 * centerLongitude);
    float cPsi = cos(0.5 * 0.);

    float sPhi = sin(0.5 * centerLatitude);
    float sThe = sin(0.5 * centerLongitude);
    float sPsi = sin(0.5 * 0.);

    float w = cPhi * cThe * cPsi + sPhi * sThe * sPsi;
    float x = sPhi * cThe * cPsi - cPhi * sThe * sPsi;
    float y = cPhi * sThe * cPsi + sPhi * cThe * sPsi;
    float z = cPhi * cThe * sPsi - sPhi * sThe * cPsi;

    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;
    float yy = y * y;
    float yz = y * z;
    float yw = y * w;
    float zz = z * z;
    float zw = z * w;

    float m00 = 1.0 - 2.0 * (yy + zz);
    float m01 =       2.0 * (xy + zw);
    float m02 =       2.0 * (xz - yw);

    float m10 =       2.0 * (xy - zw);
    float m11 = 1.0 - 2.0 * (xx + zz);
    float m12 =       2.0 * (yz + xw);

    float m20 =       2.0 * (xz + yw);
    float m21 =       2.0 * (yz - xw);
    float m22 = 1.0 - 2.0 * (xx + yy);

    mat4 rotationMatrix = mat4( m00, m10, m20, 0.,
                                m01, m11, m21, 0.,
                                m02, m12, m22, 0.,
                                 0.,  0.,  0., 1. );

    vec4 vec = rotationMatrix * cv;

    // Calculate vertex position in screen space
    gl_Position = viewportMatrix * ( vec  - vec4(8., 8., 0., 0.) + vec4(16., 16., 1., 1.)*vec4(corner, 0., 0.) );
}
//! [0]
