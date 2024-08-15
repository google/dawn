
RWByteAddressBuffer non_uniform_global : register(u0);
RWByteAddressBuffer output : register(u1);
static bool continue_execution = true;
void main() {
  if ((asint(non_uniform_global.Load(0u)) < 0)) {
    continue_execution = false;
  }
  output.Store(0u, asuint(ddx(1.0f)));
  if ((asfloat(output.Load(0u)) < 0.0f)) {
    int i = 0;
    {
      while(true) {
        float v = asfloat(output.Load(0u));
        if ((v > float(i))) {
          output.Store(0u, asuint(float(i)));
          if (!(continue_execution)) {
            discard;
          }
          return;
        }
        {
          i = (i + 1);
          if ((i == 5)) { break; }
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

