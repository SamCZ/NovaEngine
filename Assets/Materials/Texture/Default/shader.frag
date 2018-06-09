#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

#ifdef USE_TEXTURE
uniform sampler2D Texture;
uniform vec2 Scale = vec2(1.0, 1.0);
uniform vec4 Tint;
#endif

#ifdef USE_COLOR
uniform vec4 Color;
#endif

void main() {
    FragColor = vec4(1.0);
#ifdef USE_TEXTURE
    FragColor = texture(Texture, TexCoords.st * Scale) * Tint;
#endif

#ifdef USE_COLOR
    FragColor = Color;
#endif
}