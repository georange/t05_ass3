#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 uv;

uniform mat4 VIEW;
uniform mat4 PROJ;
uniform mat4 M;

uniform float time;

mat4 R(float degrees){
    mat3 R = mat3(1);
    float alpha = radians(degrees);
    R[0][0] =  cos(alpha);
    R[0][1] =  sin(alpha);
    R[1][0] = -sin(alpha);
    R[1][1] =  cos(alpha);
    return mat4(R);
}

void main() {
    uv = aPos;
    vec4 pos = PROJ * VIEW * M * vec4(aPos, 1.0);
    gl_Position = pos.xyww;

	//uv = aPos;
    //gl_Position = PROJ * VIEW * vec4(aPos, 1.0);
}  