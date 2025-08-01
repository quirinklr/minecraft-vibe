#version 450
layout(location = 0) in  vec3 inPosition;
layout(location = 1) in  vec3 inTileOrigin;          
layout(location = 2) in  vec2 inBlockUV;

layout(std140, set = 0, binding = 0) uniform CameraUbo {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    vec3 skyColor;
    float time;
    int isUnderwater;
} cameraUbo;


layout(std140, set = 0, binding = 6) readonly buffer ModelMatrixSSBO {
    mat4 models[];
} modelData;

layout(location = 2) out vec3 fragWorldPos;
layout(location = 0) flat out vec2 tileOrigin;       
layout(location = 1)      out vec2 localUV;

void main() {
    mat4 modelMatrix = modelData.models[gl_InstanceIndex];
    vec3 worldPos = (modelMatrix * vec4(inPosition, 1.0)).xyz;
    
    float y_offset = -0.2;
    float freq1 = 0.4;
    float amp1 = 0.05;
    float speed1 = 0.45;
    y_offset += sin(worldPos.x * freq1 + cameraUbo.time * speed1) * amp1;
    float freq2 = 0.4;
    float amp2 = 0.015;
    float speed2 = 0.6;
    y_offset += sin(worldPos.z * freq2 + cameraUbo.time * speed2) * amp2;
    float freq3 = 1;
    float amp3 = 0.019;
    float speed3 = 0.5;
    y_offset += sin((worldPos.x + worldPos.z) * freq3 + cameraUbo.time * speed3) * amp3;
    worldPos.y += y_offset;
    
    gl_Position = cameraUbo.proj * cameraUbo.view * vec4(worldPos, 1.0);
    tileOrigin = inTileOrigin.xy;
    localUV    = inBlockUV;
    fragWorldPos = worldPos;
}