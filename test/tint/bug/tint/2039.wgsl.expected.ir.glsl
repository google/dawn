#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint tint_symbol_1 = 0u;
  {
    while(true) {
      switch(2) {
        case 1:
        {
          {
            if (true) { break; }
          }
          continue;
        }
        default:
        {
          break;
        }
      }
      tint_symbol_1 = (tint_symbol_1 + 1u);
      {
        if (true) { break; }
      }
      continue;
    }
  }
}
