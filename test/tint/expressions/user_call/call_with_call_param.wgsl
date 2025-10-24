fn b(i: i32) -> f32 {
    return 2.3f;
}

fn c(u: u32) -> i32 {
    return 1;
}

@compute @workgroup_size(1)
fn a() {
    var a = b(c(2));
}
