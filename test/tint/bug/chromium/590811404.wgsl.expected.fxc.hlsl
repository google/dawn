struct In {
  uint3 gid;
};

struct S {
  uint a;
  uint b;
  uint c;
};

struct T {
  S x;
  S y;
  S z;
};

struct main_inputs {
  uint3 In_gid : SV_DispatchThreadID;
};


int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

void main_inner(In inp) {
  S s = (S)0;
  S v = s;
  int k = tint_f32_to_i32(float(v.a));
  switch(k) {
    case int(2144158127):
    {
      S t = {v.a, v.a, v.a};
      T u = {t, t, t};
      break;
    }
    default:
    {
      break;
    }
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  In v_1 = {inputs.In_gid};
  main_inner(v_1);
}

