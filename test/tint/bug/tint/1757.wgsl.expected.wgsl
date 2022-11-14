bug/tint/1757.wgsl:6:25 warning: use of deprecated language feature: 'sig' has been renamed to 'fract'
    let sig : f32 = res.sig;
                        ^^^

@compute @workgroup_size(1)
fn main() {
  let res = frexp(1.23);
  let exp : i32 = res.exp;
  let sig : f32 = res.sig;
}
