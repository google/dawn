statements/switch/fallthrough.wgsl:6:13 warning: use of deprecated language feature: fallthrough is set to be removed from WGSL. Case can accept multiple selectors if the existing case bodies are empty. (e.g. `case 1, 2, 3:`) `default` is a valid case selector value. (e.g. `case 1, default:`)
            fallthrough;
            ^^^^^^^^^^^

[numthreads(1, 1, 1)]
void f() {
  int i = 0;
  switch(i) {
    case 0: {
      /* fallthrough */
      {
        break;
      }
    }
    default: {
      break;
    }
  }
  return;
}
