#version 410 core

//uniform vec3 objectColor;
//uniform vec3 outlineColor;
//uniform float outlineThickness = 0.02; // Adjust thickness

in vec3 Normal;
in vec3 FragPos;
in vec3 TexCoord;

uniform sampler2DArray uTextures; 

out vec4 fragColor;

void main()
{
	vec4 texColor = texture(uTextures, TexCoord);

	if (texColor.a < 0.1)
		discard;

	// Directional sunlight
	vec3 lightDir = normalize(vec3(0.4, 1.0, 0.3));  // sun direction (top-right-ish)
	vec3 norm = normalize(Normal);
	float diffuse = max(dot(norm, lightDir), 0.0);

	// Ambient + diffuse
	float ambient = 0.35;
	float lighting = ambient + (1.0 - ambient) * diffuse;

	fragColor = vec4(texColor.rgb * lighting, texColor.a);
}
