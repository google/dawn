SKIP: FAILED

#version 310 es

struct Uniforms {
  mat4 modelViewProjectionMatrix;
};

struct VertexOutput {
  vec4 vtxFragColor;
  vec4 Position;
};

struct VertexInput {
  vec4 cur_position;
  vec4 color;
};
precision highp float;
precision highp int;


uniform Uniforms uniforms;
VertexOutput main(VertexInput tint_symbol) {
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol_1.Position = (uniforms.modelViewProjectionMatrix * tint_symbol.cur_position);
  tint_symbol_1.vtxFragColor = tint_symbol.color;
  return tint_symbol_1;
}
vec4 main(vec4 fragColor) {
  return fragColor;
}
error: Error parsing GLSL shader:
ERROR: 0:21: 'main' : function cannot take any parameter(s) 
ERROR: 0:21: 'structure' :  entry point cannot return a value
ERROR: 0:21: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es

struct Uniforms {
  mat4 modelViewProjectionMatrix;
};

struct VertexOutput {
  vec4 vtxFragColor;
  vec4 Position;
};

struct VertexInput {
  vec4 cur_position;
  vec4 color;
};
precision highp float;
precision highp int;


uniform Uniforms uniforms;
VertexOutput main(VertexInput tint_symbol) {
  VertexOutput tint_symbol_1 = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol_1.Position = (uniforms.modelViewProjectionMatrix * tint_symbol.cur_position);
  tint_symbol_1.vtxFragColor = tint_symbol.color;
  return tint_symbol_1;
}
vec4 main(vec4 fragColor) {
  return fragColor;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
