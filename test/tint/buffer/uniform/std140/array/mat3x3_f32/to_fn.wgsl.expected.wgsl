@group(0) @binding(0) var<uniform> u : array<mat3x3<f32>, 4>;

fn a(a : array<mat3x3<f32>, 4>) {
}

fn b(m : mat3x3<f32>) {
}

fn c(v : vec3<f32>) {
}

fn d(f : f32) {
}

@compute @workgroup_size(1)
fn f() {
  a(u);
  b(u[1]);
  c(u[1][0].zxy);
  d(u[1][0].zxy.x);
}
