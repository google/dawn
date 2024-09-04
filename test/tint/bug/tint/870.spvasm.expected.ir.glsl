SKIP: FAILED

#version 310 es

struct sspp962805860buildInformationS {
  vec4 footprint;
  vec4 offset;
  int essence;
  int orientation[6];
};

struct x_B4_BuildInformation {
  sspp962805860buildInformationS passthru;
};
precision highp float;
precision highp int;


x_B4_BuildInformation sspp962805860buildInformation;
void main_1() {
  int orientation[6] = int[6](0, 0, 0, 0, 0, 0);
  int x_23[6] = sspp962805860buildInformation.passthru.orientation;
  orientation[0] = x_23[0u];
  orientation[1] = x_23[1u];
  orientation[2] = x_23[2u];
  orientation[3] = x_23[3u];
  orientation[4] = x_23[4u];
  orientation[5] = x_23[5u];
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
