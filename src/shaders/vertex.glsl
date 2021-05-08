attribute vec3 position;
attribute vec3 normal;
attribute vec4 color;

uniform mat4 u_model_view_projection;

varying vec3 v_position;
varying vec3 v_normal;
varying vec4 v_color;

void main() {
    vec4 pos = u_model_view_projection * vec4(position, 1.0);

    v_normal = normal;
    v_position = position;
    v_color = color;

    gl_Position = pos;
}