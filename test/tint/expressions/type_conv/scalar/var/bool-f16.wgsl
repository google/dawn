// flags:  --hlsl-shader-model 62
enable f16;
var<private> u = bool(true);
fn f() {
    let v : f16 = f16(u);
}