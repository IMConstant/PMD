#version 130

uniform vec3 camera_position;

uniform int u_render_type;
uniform vec3 u_light_position;

uniform sampler2D tex;

varying vec3 v_position;
varying vec3 v_normal;
varying vec4 v_color;
varying vec2 v_uv;


void main() {
    vec3 light_position = u_light_position;
    vec3 light_vector = normalize(light_position - v_position);
    vec3 camera_vector = normalize(camera_position - v_position);
    vec4 color = v_color;//vec4(1.0, 1.0, 1.0, 1.0);

    if (color.r != 1.0) {
	gl_FragColor = vec4(0.0, 255.0, 0.0, 1.0);
	//return;
    }

    if (u_render_type == 0) {
	gl_FragColor = v_color * vec4(max(0.0, dot(v_normal, light_vector)));
    }
    else if (u_render_type == 1) {
	vec4 normal_color = vec4((v_normal + 1.0) / 2.0, 1.0);
	gl_FragColor = normal_color;
    }
    else if (u_render_type == 2) {
	gl_FragColor = texture(tex, v_uv);
    }
    else if (u_render_type == 3) {
	gl_FragColor = vec4(1, 1, 1, 1) * vec4(max(0.0, dot(v_normal, light_vector)));
    }

    gl_FragColor.a = 1.0;
}
