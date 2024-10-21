#version 310 es


struct Interface {
  float col1;
  float col2;
  vec4 pos;
};

layout(location = 1) out float vert_main_loc1_Output;
layout(location = 2) out float vert_main_loc2_Output;
Interface vert_main_inner() {
  return Interface(0.40000000596046447754f, 0.60000002384185791016f, vec4(0.0f));
}
void main() {
  Interface v = vert_main_inner();
  vert_main_loc1_Output = v.col1;
  vert_main_loc2_Output = v.col2;
  gl_Position = v.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
#version 310 es
precision highp float;
precision highp int;


struct Interface {
  float col1;
  float col2;
  vec4 pos;
};

layout(location = 1) in float frag_main_loc1_Input;
layout(location = 2) in float frag_main_loc2_Input;
void frag_main_inner(Interface colors) {
  float r = colors.col1;
  float g = colors.col2;
}
void main() {
  frag_main_inner(Interface(frag_main_loc1_Input, frag_main_loc2_Input, gl_FragCoord));
}
