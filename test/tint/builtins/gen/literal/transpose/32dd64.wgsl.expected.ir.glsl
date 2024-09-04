SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void transpose_32dd64() {
  mat4x3 res = mat4x3(vec3(1.0f), vec3(1.0f), vec3(1.0f), vec3(1.0f));
}
void main() {
  transpose_32dd64();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  transpose_32dd64();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  transpose_32dd64();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void transpose_32dd64() {
  mat4x3 res = mat4x3(vec3(1.0f), vec3(1.0f), vec3(1.0f), vec3(1.0f));
}
void main() {
  transpose_32dd64();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  transpose_32dd64();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  transpose_32dd64();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'main' : function already has a body 
ERROR: 0:17: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


struct VertexOutput {
  vec4 pos;
};

void transpose_32dd64() {
  mat4x3 res = mat4x3(vec3(1.0f), vec3(1.0f), vec3(1.0f), vec3(1.0f));
}
void main() {
  transpose_32dd64();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  transpose_32dd64();
}
VertexOutput main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  transpose_32dd64();
  return tint_symbol;
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'local_size_x' : there is no such layout identifier for this stage taking an assigned value 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
