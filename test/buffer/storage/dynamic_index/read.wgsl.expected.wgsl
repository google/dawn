struct Inner {
  a : vec3<i32>;
  b : i32;
  c : vec3<u32>;
  d : u32;
  e : vec3<f32>;
  f : f32;
  g : mat2x3<f32>;
  h : mat3x2<f32>;
  i : [[stride(16)]] array<vec4<i32>, 4>;
};

[[block]]
struct S {
  arr : array<Inner>;
};

[[binding(0), group(0)]] var<storage, read> s : S;

[[stage(compute), workgroup_size(1)]]
fn main([[builtin(local_invocation_index)]] idx : u32) {
  let a = s.arr[idx].a;
  let b = s.arr[idx].b;
  let c = s.arr[idx].c;
  let d = s.arr[idx].d;
  let e = s.arr[idx].e;
  let f = s.arr[idx].f;
  let g = s.arr[idx].g;
  let h = s.arr[idx].h;
  let i = s.arr[idx].i;
}
