# chess-engine

Motor de xadrez (chess engine) escrito em C++, com busca, avaliação e um pipeline de tuning automático de parâmetros.

Repositório: [github.com/jpedrosousa/chess-engine](https://github.com/jpedrosousa/chess-engine)

## Visão geral

O projeto implementa um motor de xadrez completo do zero: representação de tabuleiro, geração de movimentos legais, busca com poda alfa-beta e uma função de avaliação ajustável por meio de tuning automático (Texel tuning).

## Funcionalidades

### Representação e regras
- Representação de tabuleiro em mailbox
- Parsing e geração de FEN (round-trip validado)
- Geração de movimentos legais, validada por perft (20 / 400 / 8902 nas profundidades 1–3)

### Busca
- Negamax com poda alfa-beta
- Iterative deepening
- Quiescence search
- Null-move pruning (R=2, profundidade mínima 3, com guarda de zugzwang)
- Move ordering por MVV-LVA
- Killer moves (2 por ply) e history heuristic
- Zobrist hashing e tabela de transposição (flags EXACT / LOWERBOUND / UPPERBOUND, com ajuste de mate score por ply)

### Avaliação
- Piece-square tables
- Valores de material clássicos com fatores de escala das PSTs aprendidos via tuning (em vez de rede neural)
- `loadParams()` para carregar `tuned_params.txt` gerado pelo pipeline de tuning

### Pipeline de tuning (Texel tuning)
- `generate_data.cpp`: geração dos dados de treino
- `tune.cpp`: otimização por coordinate descent
- Saída: `tuned_params.txt`, carregado pelo motor em tempo de execução

## Estrutura do projeto

```
.
├── main.cpp
├── board.cpp
├── movegen.cpp
├── evaluation.cpp
├── search.cpp
├── play.cpp
├── zobrist.cpp
├── tt.cpp
├── generate_data.cpp
├── tune.cpp
├── CMakeLists.txt
└── include/
    └── (headers)
```

## Build

O projeto usa CMake:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Uso

### Modo interativo
Execute o binário gerado (`chess_engine`) para jogar de forma interativa.

### Testes
O motor possui uma suíte de testes embutida:

```bash
./chess_engine --test
```

Cobre: round-trip de FEN, perft (20/400/8902), detecção de xeque-mate, busca, tabela de transposição, quiescence search, `loadParams()` e a guarda de zugzwang do null-move pruning.

### Gerando e aplicando parâmetros tunados
```bash
./generate_data   # gera os dados de treino
./tune            # roda o coordinate descent e produz tuned_params.txt
```
O `tuned_params.txt` resultante é carregado automaticamente pelo motor via `loadParams()`.

## Status

Projeto completo e validado ponta a ponta: compilação limpa (sem erros/warnings), suíte de testes passando integralmente e pipeline `generate_data` → `tune` → `tuned_params.txt` → motor testado de ponta a ponta.