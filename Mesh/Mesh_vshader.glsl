#version 330 core 
in vec3 vpoint;
in vec3 vnormal;

//out vec3 fnormal_cam;
out vec2 uv;
out float height_center;
out float height_plus_u;
out float height_plus_v;

///--- Uploaded by Eigen in C++
uniform mat4 MODEL;
uniform mat4 VIEW;
uniform mat4 PROJ;

uniform float time;
uniform sampler2D height_map;

vec3 permute(vec3 x) {
	return mod(((x*34.0)+1.0)*x, 289.0); 
}

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);

  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 )) + i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 100.0 * dot(m, g);
}

// translate
mat4 T(float rate){
    mat4 T = mat4(1);
    //T[0][3] =  rate;
	T[1][3] =  rate;
	//T[2][3] =  rate;
    return T;
}

void main() {          
    //float displacement = snoise(uv);
	//float displacement = texture(tex, uv).r;

	//fnormal_cam = inverse( transpose( mat3(VIEW * MODEL) )) * vnormal; 
    
	// valley
	uv = (vpoint.xz + vec2(1.0f,1.0f)) / 2.0f;
	
	// island
	//uv = (vpoint.xz + vec2(sin(time),cos(time))) / 2.0f;

    float displacement = (texture(height_map, uv).r + snoise(uv)) /2;

	height_center = displacement;
	height_plus_u = (texture(height_map, uv + vec2(0.00001, 0)).r + snoise(uv + vec2(0.00001, 0))) /2; 
    height_plus_v = (texture(height_map, uv + vec2(0, 0.00001)).r + snoise(uv + vec2(0, 0.00001))) /2;

	if (displacement < -0.05) {
		displacement = displacement/10;
	}

	vec4 world = MODEL * vec4(vpoint + vec3(0, displacement, 0), 1.0);
    gl_Position = PROJ * VIEW * world;
}
