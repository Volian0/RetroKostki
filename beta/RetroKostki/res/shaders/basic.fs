#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform vec3 afog_color;
uniform float afog_farz;

void main()
{
	float z = gl_FragCoord.z / gl_FragCoord.w / afog_farz;
	FragColor = mix(texture(texture1, TexCoord), vec4(afog_color, 1.0), z);
}