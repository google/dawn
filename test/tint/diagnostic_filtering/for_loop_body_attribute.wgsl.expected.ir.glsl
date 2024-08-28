SKIP: FAILED

<dawn>/test/tint/diagnostic_filtering/for_loop_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_body_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  for (; x > v.x; ) @diagnostic(warning, derivative_uniformity) {
  ^^^

<dawn>/test/tint/diagnostic_filtering/for_loop_body_attribute.wgsl:8:9 note: return value of 'textureSample' may be non-uniform
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/src/tint/lang/glsl/writer/printer/printer.cc:350 internal compiler error: Switch() matched no cases. Type: tint::core::type::SampledTexture
********************************************************************
*  The tint shader compiler has encountered an unexpected error.   *
*                                                                  *
*  Please help us fix this issue by submitting a bug report at     *
*  crbug.com/tint with the source program that triggered the bug.  *
********************************************************************

tint executable returned error: signal: illegal instruction
