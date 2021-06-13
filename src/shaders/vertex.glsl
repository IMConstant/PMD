attribute vec3 position;
attribute vec3 normal;
attribute vec4 color;
attribute vec2 uv;

uniform mat4 u_model_view_projection;

varying vec3 v_position;
varying vec3 v_normal;
varying vec4 v_color;
varying vec2 v_uv;

void main() {
    vec4 pos = u_model_view_projection * vec4(position, 1.0);

    v_normal = normal;
    v_position = position;
    v_color = color;
    v_uv = uv;

    gl_Position = pos;
}
