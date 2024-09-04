SKIP: FAILED

#version 310 es

struct frexp_result_vec4_f32 {
  vec4 fract;
  ivec4 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_77af93() {
  frexp_result_vec4_f32 res = frexp_result_vec4_f32(vec4(0.5f), ivec4(1));
}
void main() {
  frexp_77af93();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_77af93();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_77af93();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct frexp_result_vec4_f32 {
  vec4 fract;
  ivec4 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_77af93() {
  frexp_result_vec4_f32 res = frexp_result_vec4_f32(vec4(0.5f), ivec4(1));
}
void main() {
  frexp_77af93();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_77af93();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_77af93();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'main' : function already has a body 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct frexp_result_vec4_f32 {
  vec4 fract;
  ivec4 exp;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void frexp_77af93() {
  frexp_result_vec4_f32 res = frexp_result_vec4_f32(vec4(0.5f), ivec4(1));
}
void main() {
  frexp_77af93();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_77af93();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  frexp_77af93();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:21: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
