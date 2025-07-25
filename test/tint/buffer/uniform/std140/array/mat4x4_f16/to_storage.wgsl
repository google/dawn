// flags:  --hlsl-shader-model 62
enable f16;

@group(0) @binding(0) var<uniform> u : array<mat4x4<f16>, 4>;
@group(0) @binding(1) var<storage, read_write> s : array<mat4x4<f16>, 4>;

@compute @workgroup_size(1)
fn f() {
    s = u;
    s[1] = u[2];
    s[1][0] = u[0][1].ywxz;
    s[1][0].x = u[0][1].x;
}
