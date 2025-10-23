// flags:  --hlsl-shader-model 62
enable f16;
var<private> t : bool;
fn m() -> bool {
    t = true;
    return bool(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : f16 = f16(m());
}
