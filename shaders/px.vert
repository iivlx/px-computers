#version 430 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform float xOffset;
uniform float yOffset;
uniform float scale;

void main() {
  vec2 position = aPos * scale + vec2(xOffset*scale, yOffset*scale);
  gl_Position = vec4(position, 0.0, 1.0);
  TexCoord = aTexCoord;
}
