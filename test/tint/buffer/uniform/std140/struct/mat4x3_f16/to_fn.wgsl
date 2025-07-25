// flags:  --hlsl-shader-model 62
enable f16;

struct S {
  before : i32,
  m : mat4x3<f16>,
  @align(64) @size(16)
  after : i32,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

fn a(a : array<S, 4>) {}
fn b(s : S) {}
fn c(m : mat4x3<f16>) {}
fn d(v : vec3<f16>) {}
fn e(f : f16) {}

@compute @workgroup_size(1)
fn f() {
    a(u);
    b(u[2]);
    c(u[2].m);
    d(u[0].m[1].zxy);
    e(u[0].m[1].zxy.x);
}
