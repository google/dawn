@compute @workgroup_size(16u, 16u, 1u)
fn main() {
  var v : u32;
  loop {
    let v_1 = 1u;

    continuing {
      v = v_1;
      break if !(true);
    }
  }
  let v_2 = v;
  {
    var v_3 : u32;
    v_3 = v_2;
    loop {
      _ = v_3;

      continuing {
        v_3 = 1u;
        break if !(true);
      }
    }
  }
}
