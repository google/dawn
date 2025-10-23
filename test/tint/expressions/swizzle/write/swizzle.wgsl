struct S {
    val: array<vec3f, 3>,
}

@compute @workgroup_size(1)
fn a() {
    var a = vec4();
    a.x = 1;
    a.z = 2;

    var d = S();
    d.val[2].y = 3;
}
