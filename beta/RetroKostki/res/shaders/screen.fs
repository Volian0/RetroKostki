#version 330 core

#ifdef GL_ES
   precision highp float;
#endif

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform vec2 curvature = vec2(3, 3);

uniform vec4 fade_color = vec4(0.0, 0.0, 0.0, 0.0);

vec4 scanLineIntensity(float uv, float resolution, float opacity)
{
	float intensity = sin(uv * resolution * 3.141 * 2.0);
	intensity = ((0.5 * intensity) + 0.5) * 0.9 + 0.1;
	return vec4(vec3(pow(intensity, opacity)), 1.0);
}
 
vec2 curveRemapUV(vec2 uv)
{
	uv = uv * 2.0 - 1.0;
	vec2 offset = abs(uv.yx) / vec2(curvature.x, curvature.y);
	uv = uv + uv * offset * offset;
	uv = uv * 0.5 + 0.5;
	return uv;
}

void main()
{ 
	vec2 remappedUV = curveRemapUV(vec2(TexCoords.x, TexCoords.y));
	vec4 baseColor = mix(texture(screenTexture, remappedUV), vec4(fade_color.r, fade_color.g, fade_color.b, 1.0), fade_color.a);

	baseColor *= scanLineIntensity(remappedUV.x, 480, 0.5);
	baseColor *= scanLineIntensity(remappedUV.y, 480, 0.5);

	if (remappedUV.x < 0.0 || remappedUV.y < 0.0 || remappedUV.x > 1.0 || remappedUV.y > 1.0) {
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	} else {
		gl_FragColor = baseColor;
	}
}