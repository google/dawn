struct Inner {
  x : i32;
};

[[block]]
struct S {
  a : vec3<i32>;
  b : i32;
  c : vec3<u32>;
  d : u32;
  e : vec3<f32>;
  f : f32;
  g : mat2x3<f32>;
  h : mat3x2<f32>;
  i : Inner;
  j : [[stride(16)]] array<Inner, 4>;
};

[[binding(0), group(0)]] var<storage, write> s : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
  s.a = vec3<i32>();
  s.b = i32();
  s.c = vec3<u32>();
  s.d = u32();
  s.e = vec3<f32>();
  s.f = f32();
  s.g = mat2x3<f32>();
  s.h = mat3x2<f32>();
  s.i = Inner();
  s.j = [[stride(16)]] array<Inner, 4>();
}
