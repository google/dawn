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
  var arg_1 = 0u;
  var res = 0u;
  arg_1 = 1u;
  let x_13 = atomicSub(&(sb_rw.arg_0), 1u);
  res = x_13;
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

Failed to generate: :15:5 error: unary: no matching overload for 'operator - (u32)'

2 candidate operators:
 • 'operator - (T  ✗ ) -> T' where:
      ✗  'T' is 'f32', 'i32' or 'f16'
 • 'operator - (vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32' or 'f16'

    %6:u32 = negation 1u
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
    %arg_1:ptr<function, u32, read_write> = var, 0u
    %res:ptr<function, u32, read_write> = var, 0u
    store %arg_1, 1u
    %5:ptr<function, u32, read_write> = var, 0u
    %6:u32 = negation 1u
    %7:u32 = convert 0u
    %8:void = %sb_rw.InterlockedAdd %7, %6, %5
    %9:u32 = load %5
    %x_13:u32 = let %9
    store %res, %x_13
    ret
  }
}
%fragment_main_1 = func():void {
  $B3: {
    %12:void = call %atomicAdd_8a199a
    ret
  }
}
%fragment_main = @fragment func():void {
  $B4: {
    %14:void = call %fragment_main_1
    ret
  }
}
%compute_main_1 = func():void {
  $B5: {
    %16:void = call %atomicAdd_8a199a
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B6: {
    %18:void = call %compute_main_1
    ret
  }
}


tint executable returned error: exit status 1
