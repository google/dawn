
static bool continue_execution = true;
void f() {
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
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

