#version 400

layout(triangles, equal_spacing, cw) in;
in vec3 tcPosition[];
out vec3 tePosition;
out vec3 tePatchDistance;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main()
{
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    tePosition = normalize(p0 + p1 + p2);
	tePatchDistance = gl_TessCoord;
    gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(tePosition, 1);
}
