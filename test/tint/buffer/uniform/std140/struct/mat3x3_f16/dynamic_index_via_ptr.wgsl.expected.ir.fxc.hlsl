SKIP: FAILED


enable f16;

struct Inner {
  @size(64)
  m : mat3x3<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat3x3<f16> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec3<f16> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f16 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :40:24 error: binary: %26 is not in scope
    %25:u32 = add %24, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:60:5 note: %26 declared here
    %26:u32 = mul %48, 2u
    ^^^^^^^

:45:24 error: binary: %26 is not in scope
    %32:u32 = add %31, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:60:5 note: %26 declared here
    %26:u32 = mul %48, 2u
    ^^^^^^^

:60:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %26:u32 = mul %48, 2u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat3x3<f16> @offset(0)
}

Outer = struct @align(8) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 8u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:u32 = add %10, %13
    %24:u32 = add %23, %16
    %25:u32 = add %24, %26
    %27:array<Inner, 4> = call %28, %25
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:u32 = add %30, %16
    %32:u32 = add %31, %26
    %33:Inner = call %34, %32
    %l_a_i_a_i:Inner = let %33
    %36:u32 = add %10, %13
    %37:mat3x3<f16> = call %38, %36
    %l_a_i_a_i_m:mat3x3<f16> = let %37
    %40:u32 = add %10, %13
    %41:u32 = add %40, %16
    %42:u32 = div %41, 16u
    %43:ptr<uniform, vec4<u32>, read> = access %a, %42
    %44:vec4<u32> = load %43
    %45:vec4<f16> = bitcast %44
    %46:vec3<f16> = swizzle %45, xyz
    %l_a_i_a_i_m_i:vec3<f16> = let %46
    %48:i32 = call %i
    %26:u32 = mul %48, 2u
    %49:u32 = add %10, %13
    %50:u32 = add %49, %16
    %51:u32 = add %50, %26
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %a, %52
    %54:u32 = mod %51, 16u
    %55:u32 = div %54, 4u
    %56:u32 = load_vector_element %53, %55
    %57:u32 = mod %51, 4u
    %58:bool = eq %57, 0u
    %59:u32 = hlsl.ternary 16u, 0u, %58
    %60:u32 = shr %56, %59
    %61:f32 = hlsl.f16tof32 %60
    %62:f16 = convert %61
    %l_a_i_a_i_m_i_i:f16 = let %62
    ret
  }
}
%21 = func(%start_byte_offset:u32):Outer {
  $B4: {
    %65:array<Inner, 4> = call %28, %start_byte_offset
    %66:Outer = construct %65
    ret %66
  }
}
%28 = func(%start_byte_offset_1:u32):array<Inner, 4> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x3<f16>(vec3<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B6, b: $B7, c: $B8] {  # loop_1
      $B6: {  # initializer
        next_iteration 0u  # -> $B7
      }
      $B7 (%idx:u32): {  # body
        %70:bool = gte %idx, 4u
        if %70 [t: $B9] {  # if_1
          $B9: {  # true
            exit_loop  # loop_1
          }
        }
        %71:u32 = mul %idx, 64u
        %72:u32 = add %start_byte_offset_1, %71
        %73:ptr<function, Inner, read_write> = access %a_1, %idx
        %74:Inner = call %34, %72
        store %73, %74
        continue  # -> $B8
      }
      $B8: {  # continuing
        %75:u32 = add %idx, 1u
        next_iteration %75  # -> $B7
      }
    }
    %76:array<Inner, 4> = load %a_1
    ret %76
  }
}
%34 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %78:mat3x3<f16> = call %38, %start_byte_offset_2
    %79:Inner = construct %78
    ret %79
  }
}
%38 = func(%start_byte_offset_3:u32):mat3x3<f16> {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %81:u32 = div %start_byte_offset_3, 16u
    %82:ptr<uniform, vec4<u32>, read> = access %a, %81
    %83:vec4<u32> = load %82
    %84:vec4<f16> = bitcast %83
    %85:vec3<f16> = swizzle %84, xyz
    %86:u32 = add 8u, %start_byte_offset_3
    %87:u32 = div %86, 16u
    %88:ptr<uniform, vec4<u32>, read> = access %a, %87
    %89:vec4<u32> = load %88
    %90:vec4<f16> = bitcast %89
    %91:vec3<f16> = swizzle %90, xyz
    %92:u32 = add 16u, %start_byte_offset_3
    %93:u32 = div %92, 16u
    %94:ptr<uniform, vec4<u32>, read> = access %a, %93
    %95:vec4<u32> = load %94
    %96:vec4<f16> = bitcast %95
    %97:vec3<f16> = swizzle %96, xyz
    %98:mat3x3<f16> = construct %85, %91, %97
    ret %98
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat3x3<f16>(vec3<f16>(0.0h))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %102:bool = gte %idx_1, 4u
        if %102 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %103:u32 = mul %idx_1, 256u
        %104:u32 = add %start_byte_offset_4, %103
        %105:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %106:Outer = call %21, %104
        store %105, %106
        continue  # -> $B15
      }
      $B15: {  # continuing
        %107:u32 = add %idx_1, 1u
        next_iteration %107  # -> $B14
      }
    }
    %108:array<Outer, 4> = load %a_2
    ret %108
  }
}

