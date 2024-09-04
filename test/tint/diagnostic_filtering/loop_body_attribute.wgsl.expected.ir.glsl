SKIP: FAILED

<dawn>/test/tint/diagnostic_filtering/loop_body_attribute.wgsl:4:9 warning: 'dpdx' must only be called from uniform control flow
    _ = dpdx(1.0);
        ^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/loop_body_attribute.wgsl:6:7 note: control flow depends on possibly non-uniform value
      break if x > 0.0;
      ^^^^^

<dawn>/test/tint/diagnostic_filtering/loop_body_attribute.wgsl:6:16 note: user-defined input 'x' of 'main' may be non-uniform
      break if x > 0.0;
               ^

#version 310 es
precision highp float;
precision highp int;


void main(float x) {
  {
    while(true) {
      dFdx(1.0f);
      {
        if ((x > 0.0f)) { break; }
      }
      continue;
    }
  }
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'main' : function cannot take any parameter(s) 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
