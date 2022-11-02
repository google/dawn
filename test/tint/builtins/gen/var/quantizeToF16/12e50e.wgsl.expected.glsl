#version 310 es

float tint_quantizeToF16(float param_0) {
  return unpackHalf2x16(packHalf2x16(vec2(param_0))).x;
}


void quantizeToF16_12e50e() {
  float arg_0 = 1.0f;
  float res = tint_quantizeToF16(arg_0);
}

vec4 vertex_main() {
  quantizeToF16_12e50e();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

float tint_quantizeToF16(float param_0) {
  return unpackHalf2x16(packHalf2x16(vec2(param_0))).x;
}


void quantizeToF16_12e50e() {
  float arg_0 = 1.0f;
  float res = tint_quantizeToF16(arg_0);
}

void fragment_main() {
  quantizeToF16_12e50e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

float tint_quantizeToF16(float param_0) {
  return unpackHalf2x16(packHalf2x16(vec2(param_0))).x;
}


void quantizeToF16_12e50e() {
  float arg_0 = 1.0f;
  float res = tint_quantizeToF16(arg_0);
}

void compute_main() {
  quantizeToF16_12e50e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
