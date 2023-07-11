var Benchmark = require('./node_modules/benchmark');
const fs = require('fs');

const QUANT = 1000000;

const memory = new WebAssembly.Memory({ initial: 100 });
const array = new Uint32Array(memory.buffer);

const wasmModule = new WebAssembly.Module(fs.readFileSync('./sumArray.wasm'));
const wasmInstance = new WebAssembly.Instance(wasmModule, { js: { mem: memory } })

const wasmO4Module = new WebAssembly.Module(fs.readFileSync('./sumArrayO4.wasm'));
const wasmO4Instance = new WebAssembly.Instance(wasmO4Module, { js: { mem: memory } })

function sum(array) {
    let sum = 0;
    for (let i = 0; i < array.length; i++) {
        sum += array[i];
    }
    return sum;
}

for (let i = 0; i < QUANT; i++) {
    array[i] = 1;
}

const sumWA = wasmInstance.exports.sum;
const sumWAO4 = wasmO4Instance.exports.sum;

console.log("\n===================")
console.log("Teste de Velocidade")
console.log("===================\n")
console.log(`Informção da Plataforma: ${Benchmark.platform.description}\n`);

console.log("Primeira execução:")
let inicio = performance.now()
sum(array);
let fim = performance.now();
console.log(`Somador em JS: ${(fim - inicio).toFixed(3)} ms`)

inicio = performance.now()
sumWA(QUANT);
fim = performance.now();
console.log(`Somador em WA: ${(fim - inicio).toFixed(3)} ms`)

inicio = performance.now()
sumWAO4(QUANT);
fim = performance.now();
console.log(`Somador em WA com otimização: ${(fim - inicio).toFixed(3)} ms\n`)

const suite = new Benchmark.Suite;
suite.add('Somador em JS', function () {
    sum(array);
}).add('Somador em WA', function () {
    sumWA(QUANT);
}).add('Somador em WA com otimização', function () {
    sumWAO4(QUANT);
}).on('complete', function () {
    console.log(this[0].toString());
    console.log(`Média: ${this[0].stats.mean * 1000} ms`);
    console.log(`Desvio Padrão: ${this[0].stats.variance * 1000000} µs\n`);
    console.log(this[1].toString());
    console.log(`Média: ${this[1].stats.mean * 1000} ms`);
    console.log(`Desvio Padrão: ${this[1].stats.variance * 1000000} µs\n`);
    console.log(this[2].toString());
    console.log(`Média: ${this[2].stats.mean * 1000} ms`);
    console.log(`Desvio Padrão: ${this[2].stats.variance * 1000000} µs\n`);
}).run({ 'async': true });
