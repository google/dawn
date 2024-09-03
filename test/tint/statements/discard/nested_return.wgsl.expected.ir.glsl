#version 310 es
precision highp float;
precision highp int;


int non_uniform_global;
bool continue_execution = true;
void main() {
  if ((non_uniform_global < 0)) {
    continue_execution = false;
  }
  if (!(continue_execution)) {
    discard;
  }
}
