# vm-manager – Implementação 7 (Memória Virtual)

Simulador de gerenciador de memória virtual em C, baseado no projeto "Designing a Virtual Memory Manager"

O programa lê um arquivo de endereços lógicos de 16 bits, traduz cada um para endereço físico
usando TLB e tabela de páginas, trata page faults lendo a página do `BACKING_STORE.bin`
(paginação sob demanda) e grava o resultado em `correct.txt`.

| Parâmetro | Valor |
|---|---|
| Espaço lógico | 2^16 = 65.536 bytes (página de 8 bits + offset de 8 bits) |
| Tamanho da página / frame | 256 bytes |
| Tabela de páginas | 256 entradas |
| TLB | 16 entradas |
| Memória física | 128 frames (padrão), preenchidos do 0 ao 127 |
| Substituição de página | FIFO ou LRU |
| Substituição na TLB | FIFO ou LRU |
| Busca na TLB | uma thread por entrada (16 threads) |

## Requisitos
- Linux, Unix ou macOS (testado no WSL2/Ubuntu)
- `gcc` e `make`
- `BACKING_STORE.bin` na pasta de onde o programa é executado

## Compilar
```
make          # gera o executável vm (128 frames, com TLB, busca com threads)
make clean    # remove vm e correct.txt
```

Opções de compilação (rode `make clean` antes de trocar de opção):

| Comando | Fluxo |
|---|---|
| `make FRAMES=256 TLB=0` | sem TLB, 256 frames (sem substituição) |
| `make TLB=0` | sem TLB, 128 frames (FIFO ou LRU pelo 2º argumento) |
| `make THREADS=0` | com TLB, busca sem threads |
| `make` | com TLB, busca com uma thread por entrada |

## Executar
```
./vm <arquivo_de_enderecos> <fifo|lru> <fifo|lru>
```
- 1º argumento: arquivo com um endereço lógico (inteiro de 0 a 65535) por linha
- 2º argumento: algoritmo de substituição de **página**
- 3º argumento: algoritmo de substituição da **TLB**

Exemplo:
```
./vm addresses.txt lru fifo
```

## Saída (`correct.txt`)
Uma linha por endereço, seguida das estatísticas:
```
Virtual address: 16916 TLB: 0 Physical address: 20 Value: 0
...
Number of Translated Addresses = 1000
Page Faults = 539
Page Fault Rate = 0.539
TLB Hits = 54
TLB Hit Rate = 0.054
```
`TLB:` é o índice (0–15) da entrada da TLB que resolveu o endereço (hit) ou que recebeu a página (miss); sem TLB, vale -1.

## Tratamento de erros
O programa valida tudo antes de traduzir e encerra com mensagem e código de saída 1 quando:
- o número de argumentos é diferente de 3, ou o algoritmo não é `fifo` nem `lru`;
- o arquivo de endereços não existe, não pode ser aberto ou está vazio;
- alguma linha não é um inteiro entre 0 e 65535 (linha vazia, texto, negativo, valor acima de 65535, lixo depois do número);
- o `BACKING_STORE.bin` não pode ser aberto ou lido;
- a criação de uma thread da TLB falha.

Arquivos com fim de linha do Windows (CRLF) são aceitos.

## Resultados com `addresses.txt` (1000 endereços)
| Configuração | Page faults | TLB hits |
|---|---|---|
| 256 frames, sem TLB | 244 | 0 |
| 128 frames, LRU, sem TLB | 539 | 0 |
| 128 frames, FIFO, sem TLB | 538 | 0 |
| 128 frames, página LRU, TLB FIFO | 539 | 54 (igual ao `correct.txt` de referência) |
| 128 frames, página LRU, TLB LRU | 539 | 55 |

## Arquivos
- `vm.c` – código-fonte
- `Makefile` – compilação e limpeza
- `evidencias.log` – registro das sessões de terminal (`script`)
