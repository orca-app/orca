(module $test.wasm
  (type (;0;) (func (result i32)))
  (func $start (type 0) (result i32)
    (local i32)
    (block (result i32)
        (i32.const 42)
        (i32.const 0)
        (br_if 0)
        (local.set 0)
        (i32.const 54)
        )
    return)
  (memory (;0;) 2)
  (export "memory" (memory 0))
  (export "start" (func $start)))