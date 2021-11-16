#version 310 es
precision mediump float;

struct sspp962805860buildInformationS {
  vec4 footprint;
  vec4 offset;
  int essence;
  int orientation[6];
};

layout (binding = 2) buffer x_B4_BuildInformation_1 {
  sspp962805860buildInformationS passthru;
} sspp962805860buildInformation;

void main_1() {
  int orientation[6] = int[6](0, 0, 0, 0, 0, 0);
  int x_23[6] = sspp962805860buildInformation.passthru.orientation;
  orientation[0] = x_23[0u];
  orientation[1] = x_23[1u];
  orientation[2] = x_23[2u];
  orientation[3] = x_23[3u];
  orientation[4] = x_23[4u];
  orientation[5] = x_23[5u];
  return;
}

void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


