# vm-manager – Implementação 7 (Memória Virtual)

Simulador de gerenciador de memória virtual em C, baseado no projeto
"Designing a Virtual Memory Manager" 
TLB de 16 entradas, memória física de 128 frames, substituição FIFO/LRU de página e de TLB.

## Compilar
```
make
make clean
```

## Executar
```
./vm addresses.txt <fifo|lru> <fifo|lru>   # 1º: substituição de página, 2º: substituição da TLB
```
Gera `correct.txt` na pasta atual. Precisa do `BACKING_STORE.bin` na mesma pasta.

## Status
Em desenvolvimento.
