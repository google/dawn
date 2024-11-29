
Texture2D<float4> tint_symbol : register(t0);
[numthreads(6, 1, 1)]
void e() {
  {
    uint2 tint_loop_idx = (0u).xx;
    uint3 v = (0u).xxx;
    tint_symbol.GetDimensions(0u, v.x, v.y, v.z);
    uint level = v.z;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((level > 0u)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
      }
      continue;
    }
  }
}

