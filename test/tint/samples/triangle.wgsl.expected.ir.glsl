SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


vec4 main(uint VertexIndex) {
  return vec4(vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f), vec2(0.5f, -0.5f))[VertexIndex], 0.0f, 1.0f);
}
vec4 main() {
  return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'main' : function cannot take any parameter(s) 
ERROR: 0:6: 'float' :  entry point cannot return a value
ERROR: 0:6: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;


vec4 main(uint VertexIndex) {
  return vec4(vec2[3](vec2(0.0f, 0.5f), vec2(-0.5f), vec2(0.5f, -0.5f))[VertexIndex], 0.0f, 1.0f);
}
vec4 main() {
  return vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'main' : function cannot take any parameter(s) 
ERROR: 0:6: 'float' :  entry point cannot return a value
ERROR: 0:6: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
