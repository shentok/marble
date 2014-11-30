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

varying vec2 v_texcoord;

const float pi = 3.14159265358979323846264;
const float EARTH_RADIUS = 6378000.0;
const float DEG2RAD = 180. / pi;

//! [0]
void main()
{
    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = texCoord;

    float P = 1.5 + 3. * 1000. * 0.4 / radius / tan(0.5 * 110. * DEG2RAD);
//    m_altitudeToPixel = radius / (EARTH_RADIUS * qSqrt((m_P-1)/(m_P+1)));
//    m_perspectiveRadius = radius / qSqrt((m_P-1)/(m_P+1));
//    m_pPfactor = (m_P+1)/(m_perspectiveRadius*m_perspectiveRadius*(m_P-1));
    float deltaLambda = position.x - centerLongitude;
    float phi = position.y;
    float phi1 = centerLatitude;

    float cosC = sin( phi1 ) * sin( phi ) + cos( phi1 ) * cos( phi ) * cos( deltaLambda );

    // Don't display placemarks that are below 10km altitude and
    // are on the Earth's backside (where cosC < 1/P)
    if (cosC < 1./P && position.z < 10000.) {
        return;
    }

    // Let (x, y) be the position on the screen of the placemark ..
    // First determine the position in unit coordinates:
    float k = (P - 1.) / (P - cosC); // scale factor
    float x = ( cos( phi ) * sin( deltaLambda ) ) * k;
    float y = ( cos( phi1 ) * sin( phi ) - sin( phi1 ) * cos( phi ) * cos( deltaLambda ) ) * k;

    // Transform to screen coordinates
    float pixelAltitude = (position.z + EARTH_RADIUS) * radius / (EARTH_RADIUS * sqrt((P-1.)/(P+1.)));
    x *= pixelAltitude;
    y *= pixelAltitude;

    y = - y;

    // Calculate vertex position in screen space
    gl_Position = viewportMatrix * vec4(x, y, 1., 1.0);
}
//! [0]
