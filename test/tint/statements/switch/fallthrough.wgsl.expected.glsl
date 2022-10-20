statements/switch/fallthrough.wgsl:6:13 warning: use of deprecated language feature: fallthrough is set to be removed from WGSL. Case can accept multiple selectors if the existing case bodies are empty. (e.g. `case 1, 2, 3:`) `default` is a valid case selector value. (e.g. `case 1, default:`)
            fallthrough;
            ^^^^^^^^^^^

#version 310 es

void f() {
  int i = 0;
  switch(i) {
    case 0: {
      /* fallthrough */
    }
    default: {
      break;
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
