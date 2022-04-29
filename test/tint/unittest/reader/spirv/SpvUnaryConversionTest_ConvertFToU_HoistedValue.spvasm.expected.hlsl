SKIP: FAILED - see https://github.com/microsoft/DirectXShaderCompiler/issues/4422

void main_1() {
  [loop] while (true) {
    float x_600 = 0.0f;
    if (true) {
      break;
    }
    if (true) {
      x_600 = 50.0f;
    }
    break;
    {
      const uint x_82 = uint(x_600);
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
Internal compiler error: access violation. Attempted to read from address 0x0000000000000048

