#version 310 es


struct In {
  uvec3 gid;
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

int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
void main_inner(In inp) {
  S s = S(0u, 0u, 0u);
  S v = s;
  int k = tint_f32_to_i32(float(v.a));
  switch(k) {
    case 2144158127:
    {
      S t = S(v.a, v.a, v.a);
      T u = T(t, t, t);
      break;
    }
    default:
    {
      break;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(In(gl_GlobalInvocationID));
}
