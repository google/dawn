SKIP: FAILED

#version 310 es

struct vertexUniformBuffer1 {
  mat2 transform1;
};

struct vertexUniformBuffer2 {
  mat2 transform2;
};

uniform vertexUniformBuffer1 x_20;
uniform vertexUniformBuffer2 x_26;
vec4 main(uint tint_symbol_1) {
  vec2 indexable[3] = vec2[3](vec2(0.0f), vec2(0.0f), vec2(0.0f));
  mat2 x_23 = x_20.transform1;
  mat2 x_28 = x_26.transform2;
  uint x_46 = tint_symbol_1;
  indexable = vec2[3](vec2(-1.0f, 1.0f), vec2(1.0f), vec2(-1.0f));
  vec2 x_51 = indexable[x_46];
  vec2 x_52 = (mat2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])) * x_51);
  return vec4(x_52[0u], x_52[1u], 0.0f, 1.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'main' : function cannot take any parameter(s) 
ERROR: 0:13: 'float' :  entry point cannot return a value
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
