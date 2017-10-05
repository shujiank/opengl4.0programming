#version 400

layout(location=0) in vec3 in_Position;

out vec3 vPosition;

void main(void)
{
  vPosition = in_Position;
}