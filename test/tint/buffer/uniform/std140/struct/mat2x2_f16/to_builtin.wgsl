// flags:  --hlsl-shader-model 62
enable f16;

struct S {
  before : i32,
  m : mat2x2<f16>,
  @align(64) @size(16)
  after : i32,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

@compute @workgroup_size(1)
fn f() {
    let t = transpose(u[2].m);
    let l = length(u[0].m[1].yx);
    let a = abs(u[0].m[1].yx.x);
}
