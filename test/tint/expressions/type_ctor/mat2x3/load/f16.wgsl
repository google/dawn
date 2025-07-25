// flags:  --hlsl-shader-model 62
enable f16;

@group(0) @binding(0)
var<storage, read_write> out : mat2x3<f16>;

@compute @workgroup_size(1)
fn f() {
  var m = mat2x3<f16>();
  out = mat2x3(m);
}
