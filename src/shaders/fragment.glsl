#version 120

uniform vec3 camera_position;

varying vec3 v_position;
varying vec3 v_normal;
varying vec4 v_color;


void main() {
    vec3 light_position = vec3(100.0, 0.0, 0.0f);
    vec3 light_vector = normalize(light_position - v_position);
    light_vector = vec3(1.0, 0.0, 0.0);
    vec3 camera_vector = normalize(camera_position - v_position);
    vec4 color = v_color;//vec4(1.0, 1.0, 1.0, 1.0);

    if (color.r != 1.0) {
	gl_FragColor = vec4(0.0, 255.0, 0.0, 1.0);
	//return;
    }

    vec4 normal_color = vec4((v_normal + 1.0) / 2.0, 1.0);
    vec4 one = vec4(1.0);

    gl_FragColor = normal_color;//vec4(max(0.0, dot(v_normal, light_vector)));
    gl_FragColor.a = 1.0;
}