//vertex shader
#version 330 core

layout (location = 0) in vec3 attribPos;
layout(location = 1) in vec2 texCoord;

uniform mat4 uMvp = mat4(1.0);
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
in vec2 uv;
in vec4 screenPos;
out vec4 FragColor;


//Noise functions----------------------
#define PI 3.1415926


float rand(in vec2 p) {
    return fract(sin(dot(p,  vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(in vec2 pos) {
    vec2 localPos = fract(pos);
    float a        = rand(floor(pos));
    float b        = rand(floor(pos + vec2(1.0, 0.0)));
    float c        = rand(floor(pos + vec2(1.0, 1.0)));
    float d        = rand(floor(pos + vec2(0.0, 1.0)));
    vec2 xy = localPos*localPos*(3.0 - 2.0*localPos);
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

bool solveQuadratic(float a, float b, float c, inout float s1, inout float s2) {
    float delta = (b*b - 4.0*a*c);
    if (delta < 0.0) 
        return false;
    float sDelta= sqrt(delta);
    s1          = (-b - sDelta)/(2.0*a);
    s2          = (-b + sDelta)/(2.0*a);
    return true;
}

vec3 tonemap(vec3 color)
{
   float gamma = 1.45;
   float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma / (1. + luma);
	color *= toneMappedLuma / luma;
	color = pow(color, vec3(1. / gamma));
	return color;
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

uniform int CHEAP_NOISE   = 1;
uniform int CHEAP_NORMALS = 0;
uniform vec2 PLANET_ROTATION;
uniform float MAX_HEIGHT = 0.001;
uniform vec3 LIGHT_POSITION;
uniform vec2 uRes;

//-----------------------Noise Map----------------------------------

float getHeight(in vec2 p) {
    float temp;
    if (CHEAP_NOISE == 1) 
        temp      = fbm(p*10.0);
    else 
        temp      = fbm(vec2(fbm(vec2(fbm(p*10.0), fbm(p*5.0))), fbm(p * 15.0)));
    return temp;
}

float worldEdge(in vec2 p, float worldHeight, float edgeHeight, in vec2 downLeft) {
    float temp;
    temp = mix(worldHeight, edgeHeight, clamp( (p.x - 1.0 + downLeft.x ) / downLeft.x ,0.0, 1.0));    
    temp = mix(temp, edgeHeight, clamp( (p.y - 1.0 + downLeft.y) /downLeft.y ,0.0, 1.0)); 
    temp = mix(temp, edgeHeight, clamp( (downLeft.x - p.x)     / downLeft.x ,0.0, 1.0)); 
    temp = mix(temp, edgeHeight, clamp( (downLeft.y - p.y)     / downLeft.y ,0.0, 1.0)); 
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

void noiseMap(in vec2 p, out vec4 outputC) {
    vec2 pp;
    pp.x = mod(p.x, 2.0);
    pp.y = mod(p.y, 2.0);
    float height    = getHeight(pp * LOD);
    if (APPLY_BORDER == 1)
        height = worldEdge(pp, height, BORDER_LEVEL, vec2(0.3));
    vec3 col;
    colorize(height, col);
    outputC = vec4(col, height);
}

//--------------------------Space Background------------------------------------------
//Need to refactor this, its an old shader that's being used
#define Nebula 10.0
#define NebIntensity 40.0
#define stars 700.0
#define colIntensity vec3(1.,0.5,1.0)
//Standard random function and noise to build perlin noise function with addition of octaves



void bg(in vec2 p, out vec3 outputC ) {
    
    float r = (fbm(uv * 10.0))*colIntensity.r; //arbitrary values in perlin function to generate "galatical" colors
    float g = (fbm(uv * 20.0))*colIntensity.g;
    float b = (fbm(uv * 300.0))*colIntensity.b;
    float fade = pow(1.0 - min(fbm(uv * Nebula), 1.0), NebIntensity);
    vec3 col = vec3(r,g,b);
    vec3 sky = vec3(pow(rand(uv), stars));
    sky += col * fade;
    outputC = vec3(sky * 0.5);
}



//--------------------------------------------Raymarching-----------------------------------------------------


int   gMaxRaySteps = 30;
float gMinForInter = 0.0001;
vec3  gBgCol       = vec3(0.05, 0.01, 0.01);
uniform vec3  gAmbientLight= vec3(0.04);

vec2  gMouse;

vec3 nullVec3;

vec2 uv0;

uniform vec3  atmCol                  = vec3(0.88,0.88, 1.3);
uniform float atmThickness            = 0.095;
uniform float atmContrib              = 3.6; //From 1.0 to higher values
float maxAtmInteriorThickness = 100.0;

struct Camera {
    vec3 position;
    vec3 rotation;
    float nearPlane;
    float farPlane;
};

struct Sphere {
    vec3 position;
    float radius;
    vec3 rotation;
    vec3 color;
};
struct Light {
    vec3 position;
    vec3 color;
};

struct HitInfo {
    vec3   color;
    Sphere sphere;
    float  height;
};


//get if ray point->light.position intersect with the sphere s
bool sphereIntersect(in vec3 p, inout Sphere s, inout Light l) {
    float r2   = s.radius * s.radius; //radius squared
    vec3  ray  = normalize(l.position - p);
    p         += ray * 2.0 * gMinForInter;
    float a    = dot(ray, ray);
    float b    = 2.0 * dot(p - s.position, ray);
    float c    = dot(p - s.position, p-s.position) - r2;	
    float s1;
    float s2;
    if (!solveQuadratic(a,b,c, s1, s2))
        return false;
        
	return (s1 > 0.2) || (s2 > 0.2);
}

float sdfSphere(vec3 pos, inout HitInfo hit) {
    vec2 sphereUV;
    
    Sphere sphere     = hit.sphere;
    
    vec3 n     = (pos - sphere.position) / sphere.radius;
    
    sphereUV.x =  atan(n.x, - n.z) / (PI*2.0) + 0.5;

    vec2 rot = PLANET_ROTATION;
    rot.x = mod(PLANET_ROTATION.x, 2.0);
    rot.y = mod(PLANET_ROTATION.y, 2.0);

    sphereUV.y =  0.5 + asin(n.y)/PI + rot.y;
    sphereUV.x += 0.1 + rot.x;
    vec4 sampleBuff = vec4(1.0);
    noiseMap(sphereUV, sampleBuff);
    hit.color = sampleBuff.xyz;
    
    hit.height = clamp(sampleBuff.w, 0.0, 1.0); 
   
    return distance(pos, sphere.position) - sphere.radius - hit.height * hit.height * MAX_HEIGHT;
}

vec3 normalSphere(in vec3 pos, in HitInfo sphere) {
    //Cheap, no height
    if (CHEAP_NORMALS == 1) {
        return normalize(pos - sphere.sphere.position);
    }
    float dis = sdfSphere(pos, sphere);
    vec2  eps = vec2(0.0001, 0.0);
    return normalize( dis - vec3(
                        sdfSphere(pos - eps.xyy, sphere),
                        sdfSphere(pos - eps.yxy, sphere),
                        sdfSphere(pos - eps.yyx, sphere)
                    ));
}

void rayMarch(in Camera cam, in vec2 p, out vec3 col, out vec3 interPos) {
    vec3 currentPosition = vec3(0.0);
    float steps          = 0.0;
    
    
    Sphere s;
    Light  l;
    s.position = vec3(-0.0, -0., 1.3);
    s.radius   = 0.5;
    s.color    = vec3(0.0, 0.0, .0);
    
    l.position = vec3(10.0,10.0, -2.);
    #ifdef LIGHT_MOUSE
    l.position = vec3(gMouse.x * 13.0, gMouse.y * 13.0, 0.3);
    #endif
    l.position = LIGHT_POSITION;
    l.color    = vec3(1.0);
    
    HitInfo hit;
    HitInfo hit0;
    
    hit.sphere  = s;
    hit0.sphere = s;
    
    float minNear     = 10000.0;
    float nearestStep = 10000.0;

    vec3 rayDir = normalize(vec3(p.x, p.y, cam.nearPlane));
    //-------------------No hit on atmosphere or sphere nor atmosphere--------------
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(-s.position, rayDir);
    float raymarchingRadius = s.radius + max(atmThickness, MAX_HEIGHT);
    float c = dot(s.position, s.position) - (raymarchingRadius) * (raymarchingRadius);
    float s0, s1 = 0.0;
    bool doesHit = solveQuadratic(a,b,c,s0,s1);
    if (!doesHit)  {
        bg(uv0, col);
        return;
    }
    
    steps += min(s0, s1);
    currentPosition = steps * rayDir;

    //------------------Single Step for planet interior, multiple for atmosphere--------------
    
    for (int i = 0; i < gMaxRaySteps; i++) {
       
        float nearest;
        nearest         = sdfSphere(currentPosition, hit);
        steps          += nearest;
        currentPosition = steps * rayDir;
        nearestStep     = min(nearestStep, steps);
        //minNear       = min(minNear, nearest); //temp for atmosphere
        minNear         = min(minNear, distance(currentPosition, s.position) - s.radius);
        //We are sure not to hit the sphere
        if (distance(currentPosition, s.position) > raymarchingRadius ) {
            break;
        }

        if (nearest > gMinForInter) {
            continue;
        }
        
         
        
        //Intersection occured
        
     
        vec3 projection =  normalize(-currentPosition + l.position);
        vec3 normal = normalSphere(currentPosition, hit0);
        
        vec3 reflection    = reflect(-projection, normal);
        vec3 viewDir      = normalize(cam.position - currentPosition );
        float p         =  (hit.height  < OCEAN_LEVEL + OCEAN_EDGE) ? 16.0 : 1.0;
        
        float specular  = pow(max(0.0,dot(viewDir, reflection)), p);
        float diffuse   = max(0.0, dot(projection, normal));
        vec3 result     = (specular * diffuse +diffuse+gAmbientLight)*l.color; 

        //Add atmosphere color
        vec3 snormal = normalize(currentPosition - s.position);
        float atmInt= min(1.0, abs(dot(snormal, normalize(cam.position - s.position))));
        
        col = hit.color;
        col = mix(col, atmCol, min(atmThickness + (atmContrib - atmInt) * atmThickness, maxAtmInteriorThickness));
        col*= result;
        return;
       
        //-----------
    }
    //Calculate atmosphere
    vec3 atmPos = nearestStep * rayDir; //Point in atmosphere
    float temp = 0.0;
    
    float tempThick  = max(0.0, dot(normalize(atmPos - s.position), normalize(l.position - s.position )));
    float atmThicknessTmp     = tempThick * atmThickness;
    
    if ((minNear < atmThicknessTmp) && atmThicknessTmp > 0.00001) {
        temp   = 1.0 - (atmThicknessTmp - minNear) / atmThicknessTmp;
        temp   = exp(-5.0*temp); 
        temp   = clamp(temp, 0.0, 1.0);
    }
    bg(uv0, col);
    col = mix(col, atmCol, temp);    
}

void main()
{

    uv0      = uv;
    vec2  tmp;

    if (uRes.x < uRes.y) {
        tmp.x = uv.x - 0.5;
        tmp.y = (uv.y - 0.5) * (uRes.y / uRes.x);
    }

    else {
        tmp.y = uv.y - 0.5;
        tmp.x = (uv.x - 0.5) * (uRes.x / uRes.y);
    }

    Camera cam;
    cam.position = vec3(0.0);
    cam.nearPlane = 0.8;
    vec3 temp;
    vec3 col;
    rayMarch(cam, tmp, col, temp);
    FragColor = vec4((col),1.0);
}