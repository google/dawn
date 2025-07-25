// flags:  --hlsl-shader-model 62
enable f16;
var<private> m = mat3x4<f16>(vec4<f16>(0.0h, 1.0h, 2.0h, 3.0h),
                             vec4<f16>(4.0h, 5.0h, 6.0h, 7.0h),
                             vec4<f16>(8.0h, 9.0h, 10.0h, 11.0h));

@group(0) @binding(0)
var<storage, read_write> out : mat3x4<f16>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
