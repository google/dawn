SKIP: FAILED

#version 310 es
precision mediump float;

void storageBarrier_d87211() {
  DeviceMemoryBarrierWithGroupSync();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  storageBarrier_d87211();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:5: 'DeviceMemoryBarrierWithGroupSync' : no matching overloaded function found 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



