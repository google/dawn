bug/tint/1064.wgsl:12:9 warning: use of deprecated language feature: `break` must not be used to exit from a continuing block. Use break-if instead.
        break;
        ^^^^^

@fragment
fn main() {
  loop {
    if (false) {
    } else {
      break;
    }

    continuing {
      if (true) {
      } else {
        break;
      }
    }
  }
}
