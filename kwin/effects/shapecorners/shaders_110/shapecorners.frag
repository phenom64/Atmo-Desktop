#version 110

uniform sampler2D sampler;
uniform sampler2D corner;

varying vec2 varyingTexCoords;

void main()
{
    vec4 tex = texture2D(sampler, varyingTexCoords);
    vec4 texCorner = texture2D(corner, varyingTexCoords);
    tex.a = texCorner.a;
    gl_FragColor = tex;
}
