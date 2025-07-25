// flags:  --hlsl-shader-model 62
enable f16;

@group(0) @binding(0) var<uniform> u : mat2x3<f16>;

@compute @workgroup_size(1)
fn f() {
    let t = transpose(u);
    let l = length(u[1]);
    let a = abs(u[0].zxy.x);
}
