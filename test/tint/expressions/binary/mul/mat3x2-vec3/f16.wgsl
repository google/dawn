// flags:  --hlsl-shader-model 62
enable f16;
struct S {
    matrix : mat3x2<f16>,
    vector : vec3<f16>,
};
@group(0) @binding(0) var<uniform> data: S;

@fragment
fn main() {
  let x = data.matrix * data.vector;
}
