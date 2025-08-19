#version 410 core

//uniform vec3 objectColor;
//uniform vec3 outlineColor;
//uniform float outlineThickness = 0.02; // Adjust thickness

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D atlas; //for now just a voxel texture

out vec4 fragColor;
void main()
{
	//vec3 absNormal = abs(Normal);
	//float edgeDist = max(absNormal.x, max(absNormal.y, absNormal.z));

	// Smoothstep for anti-aliased outline
	//float outlineFactor = smoothstep(1.0 - outlineThickness, 1.0, edgeDist);

	// Blend between fill and outline
	//vec3 finalColor = mix(objectColor, outlineColor, outlineFactor);

	//fragColor = vec4(finalColor, 1.0);

	vec4 texColor = texture(atlas, TexCoord);

	if (texColor.a < 0.1)   // optional: discard transparent texels
		discard;

	fragColor = texColor;//vec4(objectColor, 1.0);
}
