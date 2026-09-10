#version 410 core

in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D uTexture;
uniform bool uHasTexture;
uniform vec4 uColor;

out vec4 FragColor;

void main() {
    // Directional sunlight (same as voxel shader)
    vec3 lightDir = normalize(vec3(0.4, 1.0, 0.3));
    vec3 norm = normalize(fragNormal);
    float diffuse = max(dot(norm, lightDir), 0.0);

    float ambient = 0.35;
    float lighting = ambient + (1.0 - ambient) * diffuse;

    if (uHasTexture) {
        vec4 texColor = texture(uTexture, fragTexCoord);
        if (texColor.a < 0.1)
            discard;
        FragColor = vec4(texColor.rgb * lighting, texColor.a);
    } else {
        FragColor = vec4(uColor.rgb * lighting, uColor.a);
    }
}
