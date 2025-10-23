struct S {
    val: array<vec3f, 3>,
}

@compute @workgroup_size(1)
fn a() {
    var a = vec4();
    let b = a.x;
    let c = a.zzyy;

    var d = S();
    let e = d.val[2].yzx;
}
