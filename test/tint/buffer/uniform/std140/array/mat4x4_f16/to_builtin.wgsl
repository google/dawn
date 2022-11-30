enable f16;

@group(0) @binding(0) var<uniform> u : array<mat4x4<f16>, 4>;

@compute @workgroup_size(1)
fn f() {
    let t = transpose(u[2]);
    let l = length(u[0][1].ywxz);
    let a = abs(u[0][1].ywxz.x);
}
