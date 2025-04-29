#version 460 core
out vec4 FragColor;

vec3 colorTopLeft = vec3(0.2, 0.18, 0.24);
vec3 colorTopRight = vec3(1.0, 0.13, 0.56);
vec3 colorBottomLeft = vec3(0.13, 1.0, 0.4);
vec3 colorBottomRight = vec3(1.0, 1.0, 0.59);
vec3 colorCenter = vec3(0.53, 0.46, 1.0);

in vec2 pointCoords;

uniform float maxX;
uniform float maxZ;
uniform bool line;

void main(){
	float fx = pointCoords.x / maxX;
	float fz = pointCoords.y / maxZ;
	if(line){
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}
	vec3 topColor = mix(colorTopLeft, colorTopRight, fx);
	vec3 bottomColor = mix(colorBottomLeft, colorBottomRight, fx);
	vec3 edgeColor = mix(bottomColor, topColor, fz);

	float center = (1.0 - abs(fx-0.5) - abs(fz - 0.5));
	vec3 color = mix(edgeColor, colorCenter, center);
	FragColor = vec4(color, 1.0);
}