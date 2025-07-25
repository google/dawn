// flags:  --hlsl-shader-model 62
enable f16;
var<private> m = mat4x4(0.0h, 1.0h, 2.0h, 3.0h,
                        4.0h, 5.0h, 6.0h, 7.0h,
                        8.0h, 9.0h, 10.0h, 11.0h,
                        12.0h, 13.0h, 14.0h, 15.0h);

@group(0) @binding(0)
var<storage, read_write> out : mat4x4<f16>;

@compute @workgroup_size(1)
fn f() {
  out = m;
}
