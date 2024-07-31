SKIP: FAILED


struct Inner {
  @size(64)
  m : mat3x3<f32>,
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
  let l_a_i_a_i_m : mat3x3<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec3<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :40:24 error: binary: %26 is not in scope
    %25:u32 = add %24, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %26 declared here
    %26:u32 = mul %51, 4u
    ^^^^^^^

:45:24 error: binary: %26 is not in scope
    %32:u32 = add %31, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %26 declared here
    %26:u32 = mul %51, 4u
    ^^^^^^^

:50:24 error: binary: %26 is not in scope
    %38:u32 = add %37, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %26 declared here
    %26:u32 = mul %51, 4u
    ^^^^^^^

:55:24 error: binary: %26 is not in scope
    %44:u32 = add %43, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %26 declared here
    %26:u32 = mul %51, 4u
    ^^^^^^^

:63:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %26:u32 = mul %51, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(16) {
  m:mat3x3<f32> @offset(0)
}

Outer = struct @align(16) {
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
    %16:u32 = mul 16u, %15
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
    %37:u32 = add %36, %16
    %38:u32 = add %37, %26
    %39:mat3x3<f32> = call %40, %38
    %l_a_i_a_i_m:mat3x3<f32> = let %39
    %42:u32 = add %10, %13
    %43:u32 = add %42, %16
    %44:u32 = add %43, %26
    %45:u32 = div %44, 16u
    %46:ptr<uniform, vec4<u32>, read> = access %a, %45
    %47:vec4<u32> = load %46
    %48:vec3<u32> = swizzle %47, xyz
    %49:vec3<f32> = bitcast %48
    %l_a_i_a_i_m_i:vec3<f32> = let %49
    %51:i32 = call %i
    %26:u32 = mul %51, 4u
    %52:u32 = add %10, %13
    %53:u32 = add %52, %16
    %54:u32 = add %53, %26
    %55:u32 = div %54, 16u
    %56:ptr<uniform, vec4<u32>, read> = access %a, %55
    %57:u32 = mod %54, 16u
    %58:u32 = div %57, 4u
    %59:u32 = load_vector_element %56, %58
    %60:f32 = bitcast %59
    %l_a_i_a_i_m_i_i:f32 = let %60
    ret
  }
}
%21 = func(%start_byte_offset:u32):Outer {
  $B4: {
    %63:array<Inner, 4> = call %28, %start_byte_offset
    %64:Outer = construct %63
    ret %64
  }
}
%28 = func(%start_byte_offset_1:u32):array<Inner, 4> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x3<f32>(vec3<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B6, b: $B7, c: $B8] {  # loop_1
      $B6: {  # initializer
        next_iteration 0u  # -> $B7
      }
      $B7 (%idx:u32): {  # body
        %68:bool = gte %idx, 4u
        if %68 [t: $B9] {  # if_1
          $B9: {  # true
            exit_loop  # loop_1
          }
        }
        %69:u32 = mul %idx, 64u
        %70:u32 = add %start_byte_offset_1, %69
        %71:ptr<function, Inner, read_write> = access %a_1, %idx
        %72:Inner = call %34, %70
        store %71, %72
        continue  # -> $B8
      }
      $B8: {  # continuing
        %73:u32 = add %idx, 1u
        next_iteration %73  # -> $B7
      }
    }
    %74:array<Inner, 4> = load %a_1
    ret %74
  }
}
%34 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %76:mat3x3<f32> = call %40, %start_byte_offset_2
    %77:Inner = construct %76
    ret %77
  }
}
%40 = func(%start_byte_offset_3:u32):mat3x3<f32> {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %79:u32 = div %start_byte_offset_3, 16u
    %80:ptr<uniform, vec4<u32>, read> = access %a, %79
    %81:vec4<u32> = load %80
    %82:vec3<u32> = swizzle %81, xyz
    %83:vec3<f32> = bitcast %82
    %84:u32 = add 16u, %start_byte_offset_3
    %85:u32 = div %84, 16u
    %86:ptr<uniform, vec4<u32>, read> = access %a, %85
    %87:vec4<u32> = load %86
    %88:vec3<u32> = swizzle %87, xyz
    %89:vec3<f32> = bitcast %88
    %90:u32 = add 32u, %start_byte_offset_3
    %91:u32 = div %90, 16u
    %92:ptr<uniform, vec4<u32>, read> = access %a, %91
    %93:vec4<u32> = load %92
    %94:vec3<u32> = swizzle %93, xyz
    %95:vec3<f32> = bitcast %94
    %96:mat3x3<f32> = construct %83, %89, %95
    ret %96
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat3x3<f32>(vec3<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %100:bool = gte %idx_1, 4u
        if %100 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %101:u32 = mul %idx_1, 256u
        %102:u32 = add %start_byte_offset_4, %101
        %103:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %104:Outer = call %21, %102
        store %103, %104
        continue  # -> $B15
      }
      $B15: {  # continuing
        %105:u32 = add %idx_1, 1u
        next_iteration %105  # -> $B14
      }
    }
    %106:array<Outer, 4> = load %a_2
    ret %106
  }
}

