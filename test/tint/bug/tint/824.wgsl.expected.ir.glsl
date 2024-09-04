SKIP: FAILED

#version 310 es

struct Output {
  vec4 Position;
  vec4 color;
};

Output main(uint VertexIndex, uint InstanceIndex) {
  vec2 zv[4] = vec2[4](vec2(0.20000000298023223877f), vec2(0.30000001192092895508f), vec2(-0.10000000149011611938f), vec2(1.10000002384185791016f));
  float z = zv[InstanceIndex][0u];
  Output tint_symbol_1 = Output(vec4(0.0f), vec4(0.0f));
  tint_symbol_1.Position = vec4(0.5f, 0.5f, z, 1.0f);
  vec4 colors[4] = vec4[4](vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f));
  tint_symbol_1.color = colors[InstanceIndex];
  return tint_symbol_1;
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'main' : function cannot take any parameter(s) 
ERROR: 0:8: 'structure' :  entry point cannot return a value
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
