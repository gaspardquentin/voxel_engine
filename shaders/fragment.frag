#version 410 core

uniform vec3 objectColor;
uniform vec3 outlineColor;
uniform float outlineThickness = 0.02; // Adjust thickness

in vec3 Normal;
in vec3 FragPos;

out vec4 fragColor;
void main()
{
	vec3 absNormal = abs(Normal);
	float edgeDist = max(absNormal.x, max(absNormal.y, absNormal.z));

	// Smoothstep for anti-aliased outline
	float outlineFactor = smoothstep(1.0 - outlineThickness, 1.0, edgeDist);

	// Blend between fill and outline
	vec3 finalColor = mix(objectColor, outlineColor, outlineFactor);

	//fragColor = vec4(finalColor, 1.0);
	fragColor = vec4(objectColor, 1.0);
}
