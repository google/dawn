SKIP: FAILED


struct SB_RW_atomic {
  /* @offset(0) */
  arg_0 : atomic<u32>,
}

struct SB_RW {
  /* @offset(0) */
  arg_0 : u32,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicAdd_8a199a() {
  var res = 0u;
  let x_9 = atomicSub(&(sb_rw.arg_0), 1u);
  res = x_9;
  return;
}

fn fragment_main_1() {
  atomicAdd_8a199a();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  atomicAdd_8a199a();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}

Failed to generate: :13:5 error: unary: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'

    %5:u32 = negation 1u
    ^^^^^^^^^^^^^^^^^^^^

:10:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
SB_RW_atomic = struct @align(4) {
  arg_0:atomic<u32> @offset(0)
}

$B1: {  # root
  %sb_rw:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
}

%atomicAdd_8a199a = func():void {
  $B2: {
    %res:ptr<function, u32, read_write> = var, 0u
    %4:ptr<function, u32, read_write> = var, 0u
    %5:u32 = negation 1u
    %6:u32 = convert 0u
    %7:void = %sb_rw.InterlockedAdd %6, %5, %4
    %8:u32 = load %4
    %x_9:u32 = let %8
    store %res, %x_9
    ret
  }
}
%fragment_main_1 = func():void {
  $B3: {
    %11:void = call %atomicAdd_8a199a
    ret
  }
}
%fragment_main = @fragment func():void {
  $B4: {
    %13:void = call %fragment_main_1
    ret
  }
}
%compute_main_1 = func():void {
  $B5: {
    %15:void = call %atomicAdd_8a199a
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B6: {
    %17:void = call %compute_main_1
    ret
  }
}


tint executable returned error: exit status 1
