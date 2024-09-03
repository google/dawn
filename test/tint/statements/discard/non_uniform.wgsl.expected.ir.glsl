#version 310 es
precision highp float;
precision highp int;


int non_uniform_global;
float tint_symbol;
bool continue_execution = true;
void main() {
  if ((non_uniform_global < 0)) {
    continue_execution = false;
  }
  float v = dFdx(1.0f);
  if (continue_execution) {
    tint_symbol = v;
  }
  if (!(continue_execution)) {
    discard;
  }
}
