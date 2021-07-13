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
  g : vec2<i32>;
  h : vec2<i32>;
  i : mat2x3<f32>;
  j : mat3x2<f32>;
  [[align(16)]]
  k : Inner;
  [[align(16)]]
  l : [[stride(16)]] array<Inner, 4>;
};

[[binding(0), group(0)]] var<uniform> s : S;

[[stage(compute), workgroup_size(1)]]
fn main() {
  let a = s.a;
  let b = s.b;
  let c = s.c;
  let d = s.d;
  let e = s.e;
  let f = s.f;
  let g = s.g;
  let h = s.h;
  let i = s.i;
  let j = s.j;
  let k = s.k;
  let l = s.l;
}
