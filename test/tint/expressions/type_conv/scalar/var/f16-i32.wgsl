// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = f16(1.0h);

@compute @workgroup_size(1)
fn f() {
    let v : i32 = i32(u);
}
