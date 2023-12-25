//vertex shader
#version 330 core

layout (location = 0) in vec3 attribPos;
layout(location = 1) in vec2 texCoord;

uniform mat4 uMvp = mat4(1.0);
uniform vec2 uResolution;

out vec2 uv;
out vec4 screenPos;

void main() {
    gl_Position = uMvp * vec4(attribPos, 1.0);
    uv = texCoord;
    screenPos = gl_Position;
};

//fragment shader
#version 330 core

uniform float uTime;
uniform sampler2D uTex0;
uniform vec2 uResolution;
uniform vec2 uMouse;
in vec2 uv;
in vec4 screenPos;
out vec4 FragColor;


//Noise functions----------------------
float rand(in vec2 uv) {
    return fract(sin(dot(uv,  vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(in vec2 pos) {
    vec2 localPos = fract(pos);
    float a        = rand(floor(pos));
    float b        = rand(floor(pos + vec2(1.0, 0.0)));
    float c        = rand(floor(pos + vec2(1.0, 1.0)));
    float d        = rand(floor(pos + vec2(0.0, 1.0)));
    vec2 xy = localPos*localPos*(3.0 - 2.0*localPos);
    return mix(mix(a, b, xy.x), mix(d, c, xy.x), xy.y);   
    return mix(mix(a, b, xy.x), mix(d, c, xy.x), xy.y);   
}

float fbm(in vec2 pos) {
   float ret  = 0.0;
   float freq = 1.0;
   
   ret += noise(pos);
   ret += 0.5     * noise(2.0   * pos);
   ret += 0.25    * noise(4.0   * pos);
   ret += 0.125   * noise(8.0   * pos);
   ret += 0.0625  * noise(16.0  * pos);
   ret += 0.03125 * noise(32.0  * pos);
   
   return ret;
}
//--------------------------------------------



uniform float OCEAN_LEVEL      = 0.45f;
uniform float OCEAN_EDGE       = 0.1;
uniform float MOUNTAIN_LEVEL   = 0.85;
uniform float MOUNTAIN_LEVEL_2 = 0.95;
uniform float LOD               = 1.0;

uniform int   APPLY_BORDER       =  0;
uniform float BORDER_LEVEL       = -0.4;

uniform vec3 OCEAN_COLOR       = vec3(0.0, 0.0, 1.0);
uniform vec3 PLAIN_COLOR       = vec3(0.0, 0.5,0.0);
uniform vec3 MOUNTAIN_COLOR    = vec3(0.8);



float getHeight(in vec2 uv) {
    float temp      = fbm(vec2(fbm(vec2(fbm(uv*10.0), fbm(uv*5.0))), fbm(uv * 15.0)));
    return temp;
}

float worldEdge(in vec2 uv, float worldHeight, float edgeHeight, in vec2 downLeft) {
    float temp;
    temp = mix(worldHeight, edgeHeight, clamp( (uv.x - 1.0 + downLeft.x ) / downLeft.x ,0.0, 1.0));    
    temp = mix(temp, edgeHeight, clamp( (uv.y - 1.0 + downLeft.y) /downLeft.y ,0.0, 1.0)); 
    temp = mix(temp, edgeHeight, clamp( (downLeft.x - uv.x)     / downLeft.x ,0.0, 1.0)); 
    temp = mix(temp, edgeHeight, clamp( (downLeft.y - uv.y)     / downLeft.y ,0.0, 1.0)); 
    return temp;
}


void colorize(float height, out vec3 outputCol) {
    vec3 temp = vec3(height);
    if (height < OCEAN_LEVEL)
        outputCol = OCEAN_COLOR * (OCEAN_LEVEL - 0.1 + height);
    else if (height < OCEAN_LEVEL + OCEAN_EDGE){
        float x = (height - OCEAN_LEVEL) / OCEAN_EDGE;
        outputCol     = mix(OCEAN_COLOR, temp, x);
    }
    else if (height < MOUNTAIN_LEVEL)
        outputCol = temp * PLAIN_COLOR;
    else if (height < MOUNTAIN_LEVEL_2)
        outputCol = temp * temp * temp * MOUNTAIN_COLOR;
    else 
        outputCol = temp * MOUNTAIN_COLOR;
}

void main() {
    float height    = getHeight(uv * LOD);
    if (APPLY_BORDER != 0)
        height = worldEdge(uv, height, BORDER_LEVEL, vec2(0.3));
    vec3 col = vec3(height);
    colorize(height, col);
    FragColor = vec4(col, 1.0);
}
