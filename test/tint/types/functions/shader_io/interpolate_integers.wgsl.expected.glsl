#version 310 es

layout(location = 0) flat out int i_1;
layout(location = 1) flat out uint u_1;
layout(location = 2) flat out ivec4 vi_1;
layout(location = 3) flat out uvec4 vu_1;
struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};

Interface vert_main() {
  Interface tint_symbol = Interface(0, 0u, ivec4(0), uvec4(0u), vec4(0.0f));
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  Interface inner_result = vert_main();
  i_1 = inner_result.i;
  u_1 = inner_result.u;
  vi_1 = inner_result.vi;
  vu_1 = inner_result.vu;
  gl_Position = inner_result.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;

layout(location = 0) flat in int i_1;
layout(location = 1) flat in uint u_1;
layout(location = 2) flat in ivec4 vi_1;
layout(location = 3) flat in uvec4 vu_1;
layout(location = 0) out int value;
struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};

int frag_main(Interface inputs) {
  return inputs.i;
}

void main() {
  Interface tint_symbol = Interface(i_1, u_1, vi_1, vu_1, gl_FragCoord);
  int inner_result = frag_main(tint_symbol);
  value = inner_result;
  return;
}
