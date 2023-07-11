function fatorial(n) {
    if (n == 0) {
        return 1;
    }
    else {
        return n * fatorial(n - 1);
    }
}

console.time("JS");
// console.log(fibonacci(100000));
for (let i = 0; i < 1000; i++)
    console.log(fatorial(10));
console.timeEnd("JS");