#version 310 es


struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};

layout(location = 0) flat out int vert_main_loc0_Output;
layout(location = 1) flat out uint vert_main_loc1_Output;
layout(location = 2) flat out ivec4 vert_main_loc2_Output;
layout(location = 3) flat out uvec4 vert_main_loc3_Output;
Interface vert_main_inner() {
  return Interface(0, 0u, ivec4(0), uvec4(0u), vec4(0.0f));
}
void main() {
  Interface v = vert_main_inner();
  vert_main_loc0_Output = v.i;
  vert_main_loc1_Output = v.u;
  vert_main_loc2_Output = v.vi;
  vert_main_loc3_Output = v.vu;
  gl_Position = v.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
#version 310 es
precision highp float;
precision highp int;


struct Interface {
  int i;
  uint u;
  ivec4 vi;
  uvec4 vu;
  vec4 pos;
};

layout(location = 0) flat in int frag_main_loc0_Input;
layout(location = 1) flat in uint frag_main_loc1_Input;
layout(location = 2) flat in ivec4 frag_main_loc2_Input;
layout(location = 3) flat in uvec4 frag_main_loc3_Input;
layout(location = 0) out int frag_main_loc0_Output;
int frag_main_inner(Interface inputs) {
  return inputs.i;
}
void main() {
  frag_main_loc0_Output = frag_main_inner(Interface(frag_main_loc0_Input, frag_main_loc1_Input, frag_main_loc2_Input, frag_main_loc3_Input, gl_FragCoord));
}
