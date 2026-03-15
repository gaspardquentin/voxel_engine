#version 410 core

in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D uTexture;
uniform bool uHasTexture;
uniform vec4 uColor;

out vec4 FragColor;

void main() {
    if (uHasTexture) {
        vec4 texColor = texture(uTexture, fragTexCoord);
        if (texColor.a < 0.1)
            discard;
        FragColor = texColor;
    } else {
        FragColor = uColor;
    }
}
