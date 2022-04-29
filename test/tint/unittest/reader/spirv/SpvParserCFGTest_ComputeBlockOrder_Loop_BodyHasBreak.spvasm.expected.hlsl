SKIP: FAILED - see https://github.com/microsoft/DirectXShaderCompiler/issues/4422

static uint var_1 = 0u;

void main_1() {
  [loop] while (true) {
    if (false) {
    } else {
      break;
    }
    break;
  }
  return;
}

void main() {
  main_1();
  return;
}
Internal compiler error: access violation. Attempted to read from address 0x0000000000000048

