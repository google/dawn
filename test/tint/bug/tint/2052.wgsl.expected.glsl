#version 310 es
precision highp float;
precision highp int;

bool continue_execution = true;
void main() {
  continue_execution = false;
  if (!(continue_execution)) {
    discard;
  }
}
