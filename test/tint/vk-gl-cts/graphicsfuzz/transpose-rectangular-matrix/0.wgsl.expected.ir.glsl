SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  mat4x3 x_37 = mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
  mat4x3 x_38_phi = mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f));
  vec3 x_48_phi = vec3(0.0f);
  float x_32 = tint_symbol.y;
  if ((x_32 < 1.0f)) {
    x_38_phi = mat4x3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f));
  } else {
    x_37 = mat4x3(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f));
    x_38_phi = x_37;
  }
  mat4x3 x_38 = x_38_phi;
  float x_40 = transpose(x_38)[0u][1u];
  {
    while(true) {
      if ((x_40 > 1.0f)) {
        x_48_phi = vec3(0.0f);
        break;
      }
      x_48_phi = vec3(1.0f, 0.0f, 0.0f);
      break;
    }
  }
  vec3 x_48 = x_48_phi;
  x_GLF_color = vec4(x_48[0u], x_48[1u], x_48[2u], 1.0f);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
