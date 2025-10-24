#version 310 es
precision highp float;
precision highp int;

bool continue_execution = true;
void main() {
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      continue_execution = false;
      if (!(continue_execution)) {
        discard;
      }
      return;
    }
  }
  /* unreachable */
}
