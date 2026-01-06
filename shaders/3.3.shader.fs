#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 objectColor;
uniform sampler2D texture_diffuse1;
uniform bool useTexture;

uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void main()
{
    vec4 baseColor;
    if (useTexture)
        baseColor = texture(texture_diffuse1, TexCoords);
    else
        baseColor = vec4(objectColor, 1.0);

    FragColor = baseColor;
}
