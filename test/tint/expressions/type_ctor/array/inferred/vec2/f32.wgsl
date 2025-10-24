var<private> arr = array(vec2<f32>(1f), vec2<f32>(2f));

@compute @workgroup_size(1)
fn f() {
    var v = arr;
}
