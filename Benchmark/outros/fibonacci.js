function fibonacci(n) {
    let a = 0;
    let b = 1;
    let result = 0;
    for (let i = 2; i <= n; i++) {
        result = a + b;
        a = b;
        b = result;
    }
    return result;
}

console.time("JS");
// console.log(fibonacci(100000));
console.log(fibonacci(10000));
console.timeEnd("JS");