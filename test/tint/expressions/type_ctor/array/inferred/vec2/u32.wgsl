var<private> arr = array(vec2<u32>(1u), vec2<u32>(2u));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
