[[stage(compute)]]
fn main() {
    var exponent : i32;
    let significand : f32 = frexp(1.23, &exponent);
}
