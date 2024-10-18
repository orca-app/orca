export fn run(n: i32) i32 {
    if (n < 2) {
        return 1;
    } else {
        var a = run(n - 1);
        var b = run(n - 2);
        return a + b;
    }
}
