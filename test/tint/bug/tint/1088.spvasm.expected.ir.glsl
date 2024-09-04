SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct LeftOver {
  mat4 worldViewProjection;
  float time;
  mat4 test2[2];
  strided_arr test[4];
};

struct main_out {
  vec4 tint_symbol;
  vec2 vUV_1;
};

vec3 position_1 = vec3(0.0f);
uniform LeftOver x_14;
vec2 vUV = vec2(0.0f);
vec2 uv = vec2(0.0f);
vec3 normal = vec3(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  vec4 q = vec4(0.0f);
  vec3 p = vec3(0.0f);
  q = vec4(position_1.x, position_1.y, position_1.z, 1.0f);
  p = q.xyz;
  float v = p.x;
  p[0u] = (v + sin(((x_14.test[0].el * position_1.y) + x_14.time)));
  float v_1 = p.y;
  p[1u] = (v_1 + sin((x_14.time + 4.0f)));
  mat4 v_2 = x_14.worldViewProjection;
  tint_symbol = (v_2 * vec4(p.x, p.y, p.z, 1.0f));
  vUV = uv;
  tint_symbol[1u] = (tint_symbol.y * -1.0f);
}
main_out main(vec3 position_1_param, vec2 uv_param, vec3 normal_param) {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  return main_out(tint_symbol, vUV);
}
error: Error parsing GLSL shader:
ERROR: 0:39: 'main' : function cannot take any parameter(s) 
ERROR: 0:39: 'structure' :  entry point cannot return a value
ERROR: 0:39: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
