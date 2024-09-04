SKIP: FAILED

#version 310 es

struct modf_result_vec3_f32 {
  vec3 fract;
  vec3 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_68d8ee() {
  modf_result_vec3_f32 res = modf_result_vec3_f32(vec3(-0.5f), vec3(-1.0f));
}
void main() {
  modf_68d8ee();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_68d8ee();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_68d8ee();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct modf_result_vec3_f32 {
  vec3 fract;
  vec3 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_68d8ee() {
  modf_result_vec3_f32 res = modf_result_vec3_f32(vec3(-0.5f), vec3(-1.0f));
}
void main() {
  modf_68d8ee();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_68d8ee();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_68d8ee();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'main' : function already has a body 
ERROR: 0:22: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct modf_result_vec3_f32 {
  vec3 fract;
  vec3 whole;
};
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void modf_68d8ee() {
  modf_result_vec3_f32 res = modf_result_vec3_f32(vec3(-0.5f), vec3(-1.0f));
}
void main() {
  modf_68d8ee();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_68d8ee();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  modf_68d8ee();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:21: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
