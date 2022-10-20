statements/switch/fallthrough.wgsl:6:13 warning: use of deprecated language feature: fallthrough is set to be removed from WGSL. Case can accept multiple selectors if the existing case bodies are empty. (e.g. `case 1, 2, 3:`) `default` is a valid case selector value. (e.g. `case 1, default:`)
            fallthrough;
            ^^^^^^^^^^^

@compute @workgroup_size(1)
fn f() {
  var i : i32;
  switch(i) {
    case 0: {
      fallthrough;
    }
    default: {
      break;
    }
  }
}
