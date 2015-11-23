#version 140

uniform sampler2D sampler;
uniform sampler2D corner;
in vec2 varyingTexCoords;
out vec4 fragColor;

void main()
{
    vec4 tex = texture(sampler, varyingTexCoords);
    vec4 texCorner = texture(corner, varyingTexCoords);
    tex.a = texCorner.a;
    fragColor = tex;
}
