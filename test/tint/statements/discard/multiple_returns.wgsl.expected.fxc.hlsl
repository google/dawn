
RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);
static bool continue_execution = true;
void main() {
  if ((asint(non_uniform_global.Load(0u)) < int(0))) {
    continue_execution = false;
  }
  float v = ddx(1.0f);
  if (continue_execution) {
    output.Store(0u, asuint(v));
  }
  if ((asfloat(output.Load(0u)) < 0.0f)) {
    int i = int(0);
    {
      uint2 tint_loop_idx = (4294967295u).xx;
      while(true) {
        if (all((tint_loop_idx == (0u).xx))) {
          break;
        }
        float v_1 = asfloat(output.Load(0u));
        if ((v_1 > float(i))) {
          float v_2 = float(i);
          if (continue_execution) {
            output.Store(0u, asuint(v_2));
          }
          if (!(continue_execution)) {
            discard;
          }
          return;
        }
        {
          uint tint_low_inc = (tint_loop_idx.x - 1u);
          tint_loop_idx.x = tint_low_inc;
          uint tint_carry = uint((tint_low_inc == 4294967295u));
          tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
          i = (i + int(1));
          if ((i == int(5))) { break; }
        }
        continue;
      }
    }
    if (!(continue_execution)) {
      discard;
    }
    return;
  }
  if (!(continue_execution)) {
    discard;
  }
}

