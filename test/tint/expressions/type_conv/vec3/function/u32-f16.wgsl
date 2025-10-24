// flags:  --hlsl-shader-model 62
enable f16;
var<private> t : u32;
fn m() -> vec3<u32> {
    t = 1u;
    return vec3<u32>(t);
}

@compute @workgroup_size(1)
fn f() {
    var v : vec3<f16> = vec3<f16>(m());
}
