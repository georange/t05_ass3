#version 330 core
out vec4 color;
//in vec3 fnormal_cam;

in vec2 uv;
in float height_center;
in float height_plus_u;
in float height_plus_v;

uniform sampler2D height_map;
//uniform sampler2D diffuse_map;

uniform sampler2D water;
uniform sampler2D sand;
uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D snow;

vec3 R = vec3(1,0,0);
vec3 G = vec3(0,1,0);
vec3 B = vec3(0,0,1);

vec3 getTex(float height, float slope, float tiling_amount) {
	vec3 water_tex = texture(water, uv * tiling_amount).rgb;
	vec3 sand_tex = texture(sand, uv * tiling_amount).rgb;
	vec3 grass_tex = texture(grass, uv * tiling_amount).rgb;
	vec3 rock_tex = texture(rock, uv * tiling_amount).rgb;
	vec3 snow_tex = texture(snow, uv * tiling_amount).rgb;

	float water_weight = 0;
	float sand_weight = 0;
	float grass_weight = 0;
	float rock_weight = 0;
	float snow_weight = 0;

	height = height * 100;
	
	if (height < -5.0) {
		water_weight = 1.0;
	} else if (height < -2.0) {
		sand_weight = (height + 5.0) / 3.0;
		water_weight = 1.0 - sand_weight;

	} else if (height < 0.0) {
		sand_weight = 1.0;
	} else if (height < 5.0) {
		grass_weight = (height + 0.0) / 5.0;
		sand_weight = 1.0 - grass_weight;

	} else if (height < 10.0) {
		grass_weight = 1.0;
	} else if (height < 15.0) {
		rock_weight = (height - 10.0) / 5.0;
		grass_weight = 1.0 - rock_weight;

	} else if (height < 16.0 || slope > 0.7) {
		rock_weight = 1.0;
	} else if (height < 20.0 || slope > 0.7) {
		snow_weight = (height - 16.0) / 4.0;
		rock_weight = 1.0 - snow_weight;

	} else {	
		snow_weight = 1.0;	
	}

	vec3 weighted = (water_tex * water_weight) + (sand_tex * sand_weight) + (grass_tex * grass_weight) + (rock_tex * rock_weight) + (snow_tex * snow_weight);
	return weighted;
}

void main() {
    float tiling_amount = 3;

	float eps = 0.00001;
    //float height_center = texture(height_map, uv).r;
    //float height_plus_u = texture(height_map, uv + vec2(eps, 0)).r; // get offset height values
    //float height_plus_v = texture(height_map, uv + vec2(0, eps)).r;

    //float slope = height_plus_v/height_plus_u; 

	vec3 X = vec3(eps * 2.0f, height_plus_u, 0);
	vec3 Y = vec3(0, height_plus_v, eps * 2.0f);
	vec3 C = vec3(0, height_center, 0);

	vec3 dx = normalize(X - C);
	vec3 dy = normalize(Y - C);

    vec3 N = normalize(cross(dy, dx)); // Calculate normal from height differences
    vec3 light = normalize(vec3(1,3,0));
    vec3 ambient = vec3(0.1, 0.1, 0.2);

	float slope = 1.0f - N.y;

	vec3 weighted_tex = getTex(height_center, slope, tiling_amount);
    vec3 diffuse = weighted_tex * clamp(dot(N, light), 0, 1);


    // Optional TODO: add specular term
    // Hint: you will need the world space position of each pixel.
    // You can easily get this from the vertex shader where it is
    // already calculated
        
    color = vec4(ambient + diffuse,1); 
}
