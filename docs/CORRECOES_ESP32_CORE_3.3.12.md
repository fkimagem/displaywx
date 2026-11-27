# Correções da DisplayWX — ESP32 core 3.3.12

Branch: `codex/correcoes-esp32-core-3.3.12`. Base: `e7981ada00930911c7cd910dbec6ce11230bec26`.

Este documento acompanha a implementação dos 36 itens da [revisão original](REVISAO_TECNICA_ESP32_CORE_3.3.12.md). A revisão original permanece como histórico; seus números de linha são anteriores às correções.

## Alterações implementadas

| Item | Correção |
|---|---|
| DWX-01 | RadioGroup inicializa o ponteiro, mantém um único dono dos botões e rejeita origem nula. |
| DWX-02 | XPT2046 registra propriedade do SPI; `end()` preserva o barramento emprestado. |
| DWX-03 | Calibração usa vetor fixo de 4 ou 9 pontos; reinicialização encerra o driver anterior. CST816 preserva sua ISR própria e a desarma antes da destruição. |
| DWX-04 | GaugeSuper verifica o índice antes de acessar intervalos e valida ordenação, quantidade e limites. |
| DWX-05 | Formatação limitada por capacidade, sem multiplicação de inteiros para medir texto; floats extremos usam notação compacta quando necessário. |
| DWX-06 | GSL3680 verifica erros de leitura, limita os pontos decodificados e respeita a capacidade da saída. |
| DWX-07 | GT911 rejeita contagem e resolução inválidas; checksum e estado de leitura são inicializados. |
| DWX-08 | LineChart valida geometria antes de subtrair margens, exige pelo menos dois pontos e usa contadores que não dão volta em 255. |
| DWX-09 | `end(timeoutMs)` solicita parada, espera os workers suspensos e fora dos núcleos, libera callers de `blockLoopTask()` e só então destrói recursos. |
| DWX-10 | CameraController aceita apenas frames RGB565 com dimensões e capacidade válidas. O fallback RAW foi retirado da apresentação. |
| DWX-11 | Auto-click usa prazo atendido pela tarefa de UI; não existe mais timer FreeRTOS com ponteiro para o objeto. |
| DWX-12 | Desenho e estado dos widgets usam mutex recursivo comum; `startCustomDraw()` informa sucesso e `finishCustomDraw()` verifica o dono. |
| DWX-13 | Setters e desenho são serializados; mudança de tela usa troca atômica. TextBox e NumberBox oferecem `copyValue()` para leitura concorrente. |
| DWX-14 | I2C compartilhado mantém o lock durante transmissão e consumo da resposta; `endRead()` descarta uma resposta não consumida. |
| DWX-15 | Operações legadas do SD permanecem dentro do escopo do guard. |
| DWX-16 | LineChart só fica pronto após alocação válida; redraw usa troca atômica e versão dos dados para preservar atualizações concorrentes. |
| DWX-17 | Cópia da câmera usa estados de propriedade. Timeout não libera nem reutiliza um buffer enquanto o produtor escreve. Não há notificação para um chamador expirado. |
| DWX-18 | Buffers próprios do PPA têm endereço e capacidade alinhados a 128 bytes. O caminho direto DSI exige capacidade existente alinhada; caso contrário usa staging. |
| DWX-19 | Setup reutiliza filas existentes, usa alocações sem exceção e verifica falhas; teclados fazem rollback se o setup falhar. |
| DWX-20 | Worker separado grava logs e tenta novamente lotes pendentes; só remove registros confirmados. `droppedLogCount()` informa descartes. |
| DWX-21 | Câmera encerra captura, frame, vídeo, PPA e XCLK; falha no início do XCLK libera a alocação. |
| DWX-22 | GSL3680 mantém handles por objeto e faz rollback dos estágios criados. TwoWire administra o barramento próprio e seus pinos. |
| DWX-23 | Objetos donos de recursos têm cópia desabilitada: DisplayWX, TouchScreen, XPT2046, GSL3680, TouchI2cMaster, RadioGroup, GaugeSuper, LineChart, Image e CameraController. |
| DWX-24 | Geometria degenerada é rejeitada; mapeamento usa intermediários largos e saturação; ângulos não finitos são rejeitados e normalização não usa laço. |
| DWX-25 | Métricas de texto estão em C: fallback válido, tabela vazia segura, acumuladores largos, dimensões saturadas e índice opcional fornecido pelo chamador. |
| DWX-26 | Somente uma instância DisplayWX pode possuir o contexto global. Uma segunda instância retorna `isValid() == false`. |
| DWX-27 | Diagnósticos periódicos da UI dependem de `DFK_ENABLE_DIAGNOSTICS`. Logs de alta frequência do XPT ficam desativados por padrão. |
| DWX-28 | A biblioteca registra apenas sua tarefa no watchdog existente; não altera a política global da aplicação. |
| DWX-29 | Snapshot do gráfico usa até dois `memcpy` por série; tabelas de ponteiros têm capacidade fixa, sem três alocações extras. |
| DWX-30 | Captura adquire o frame DMA uma vez; snapshot escrito pela CPU é publicado com writeback; PPA invalida a saída antes de leitura pela CPU. |
| DWX-31 | Gráfico, imagens e câmera têm limites configuráveis. Imagens e previews preferem PSRAM; snapshot e preview são reutilizados. |
| DWX-32 | Imagem rotacionada usa linhas de até 128 pixels, recorte prévio e incremento das coordenadas. Rotações cardinais usam seno/cosseno exatos. |
| DWX-33 | Formatações numéricas de GaugeSuper, SpinBox e VAnalog usam buffers C, evitando `String` temporária nos trechos alterados. |
| DWX-34 | Widgets opcionais podem ser excluídos; fontes têm seleção por família, catálogo compacto/customizado e dados Roboto `inline const` compartilhados pelo linker. |
| DWX-35 | Callbacks têm orçamento entre chamadas; logs saíram da UI e abertura do editor de câmera tornou-se assíncrona. |
| DWX-36 | Dependência padrão fixada, ambiente documentado e scripts de compilação/testes adicionados. |

Os algoritmos sem dependência de Arduino ficam em `src/extras/dwx_safe.c` e `src/extras/textmetrics.c`, com cabeçalhos C/C++. A camada de widgets permanece C++ para integração com Arduino, guards e atômicos.

## Contratos que o sketch deve respeitar

- **Vida dos objetos:** pare produtores externos, chame `end()` e confirme `true` antes de destruir a DisplayWX. `false` significa que ainda existem recursos em uso; mantenha o objeto vivo e tente novamente. Não destrua a DisplayWX de seu próprio callback/worker, nem com uma transação de desenho aberta. O destrutor espera o encerramento, sem liberar recursos ainda usados.
- **Widgets e buffers emprestados:** a biblioteca não passa a possuir os arrays de widgets, fontes, textos de configuração e imagens emprestados. Eles precisam continuar válidos durante o uso. Não destrua/reconfigure um LineChart enquanto outra tarefa chama `push()`.
- **Concorrência:** chamadas públicas de widgets e desenho adquirem o mutex comum. `LineChart::push()` usa somente seu mutex de dados. Um desenho demorado pode atrasar outros setters. Não chame essas APIs de uma ISR. Acesso direto a membros globais públicos, ao backend ou a ponteiros retornados continua exigindo `WidgetStateGuard` ou uma transação de desenho.
- **Desenho manual:** use `if (display.startCustomDraw(timeoutMs)) { /* desenhar */ display.finishCustomDraw(); }`. O padrão espera a aquisição; timeout explícito pode retornar `false`.
- **Textos:** `getValue()` de TextBox e `getValueChar()` de NumberBox devolvem ponteiros emprestados. Para outro task, prefira `copyValue(buffer, capacidade)`, que faz uma cópia protegida e retorna o tamanho original, no contrato de `strlcpy`.
- **Snapshots e cópias:** `Image::getMetrics()` e `Numpad::getPressedKey()` devolvem cópias protegidas. Use `auto valor = ...` ou `const auto& valor = ...`, sem referência mutável ao estado interno. Widgets com estado atômico/recursos próprios não podem ser copiados; mantenha endereços estáveis durante o registro na UI. `appendLog()` passou a retornar sucesso/falha.
- **Editor de câmera:** `beginCameraConfig()` agora retorna se o pedido foi aceito. A UI abre o editor quando a cópia termina; falha/timeout abandona o pedido com log. O widget deve permanecer válido enquanto o pedido estiver pendente. Snapshot pode continuar alocado para reutilização até `end()`.
- **Auto-click:** permanece um disparo único; a precisão acompanha o ciclo da UI. Não há serviço de timer separado.
- **Watchdog:** configure a política global no sketch. `createTask(true)` registra somente a tarefa de UI; `timeout_s` foi mantido na assinatura por compatibilidade e não reconfigura o watchdog.
- **Callbacks:** `DFK_CALLBACK_BUDGET_US` vale entre callbacks, não interrompe uma função já executando. Mova rede, espera e processamento longo para uma tarefa da aplicação.
- **Logs:** fila cheia e descarte no encerramento incrementam o contador. Falha/remoção do SD mantém registros pendentes para tentativa posterior. Erro parcial de escrita ou de flush pode repetir conteúdo na tentativa seguinte; não há garantia transacional de escrita única. O timeout de `end()` não consegue interromper uma chamada de SD que já está bloqueada.
- **Nome do log:** `getLogFileName()` retorna armazenamento emprestado que muda na rotação. Mantenha um `displaywx::storage::SdGuard` válido durante a leitura/cópia. A chamada direta ao helper legado `generateNameFile()` também exige esse guard. O mutex de desenho não substitui o mutex do armazenamento.
- **I2C compartilhado:** todos os consumidores do mesmo barramento, inclusive acessos diretos feitos pelo sketch, precisam participar do mesmo lock. Depois de `requestFrom()`, consuma os bytes na mesma tarefa ou chame `endRead()`.

## Configuração e dependências

Ambiente de referência: Arduino ESP32 **3.3.12**, ESP-IDF **5.5.5**, GCC **14.2.0**, GNU C++20/GNU C17 e otimização `-Os`. `library.properties` fixa a dependência padrão **GFX Library for Arduino 1.6.5**. A versão do core é selecionada no gerenciador de placas; `depends` não instala/pina cores.

| Caminho opcional | Pacote local de referência |
|---|---|
| Adafruit GFX | 1.12.1 |
| SSD1306 | Adafruit SSD1306 2.5.15 |
| PCD8544 | Adafruit PCD8544 Nokia 5110 LCD library 2.0.3 |
| ST7565 | ST7565 LCD 1.0.2 |
| U8g2 | 2.36.12 |
| CST816 | CST816S 1.1.1 |
| FS, SD, SPI, Wire, ESP_Video | Versões fornecidas pelo core 3.3.12 |

Use `user_setup.h` ou `DISPLAYWX_CONFIG_HEADER` global, como descrito no README. Uma macro definida somente no `.ino` não configura os arquivos `.cpp` da biblioteca.

`DFK_DISABLE_` aceita: `TOUCHAREA`, `CIRCLEBTN`, `RECTBTN`, `LED`, `VBAR`, `TOGGLE`, `HSLIDER`, `TEXTBOX`, `CHECKBOX`, `RADIO`, `NUMBERBOX`, `VANALOG`, `GAUGE`, `LINECHART`, `LABEL`, `IMAGE`, `SD`, `SPINBOX`, `TEXTBUTTON`, `CIRCULARBAR`, `THERMOMETER`, `CAMERA_MIPI`, `LOGGING`, `SPIFFS`, `FATFS`. Label permanece disponível como dependência interna de outros widgets mesmo quando seu registro automático é desativado.

| Limite | Padrão | Escopo |
|---|---:|---|
| `DFK_LINECHART_MAX_BYTES` | 512 KiB | Pool de cada gráfico |
| `DFK_IMAGE_MAX_BYTES` | 4 MiB | Pixels + máscara de cada imagem carregada |
| `DFK_CAMERA_MAX_BYTES` | 4 MiB | Cada buffer próprio de preview/snapshot |
| `DFK_CALLBACK_BUDGET_US` | 2.000 µs | Trabalho entre callbacks, máximo de 8 por ciclo |
| `DFK_TEXT_MAX_BYTES` | 65.535 bytes | Texto examinado pelo medidor C |

Os limites não reservam RAM e não substituem a verificação de alocação. Durante crescimento de buffer pode existir temporariamente o buffer antigo junto com o novo. Ajuste os limites à PSRAM/RAM da placa. O logger adiciona uma tarefa com pilha estática de 4 KiB quando habilitado.

Para métricas C, inicialize `replacementIndex = -1` se quiser procurar `?`. `dwx_text_index()` recebe os vetores do chamador; mantenha-os vivos e imutáveis enquanto o medidor os usar. Só marque `pairsSorted` quando os pares estiverem ordenados por `(left, right)`.

## Validação automatizada

Os scripts gravam objetos e logs apenas em `tmp/`. Não fazem upload:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/run_host_tests.ps1
python tests/display_backend_contract_test.py
python tests/compile_matrix.py --target esp32 --touch XPT2046
python tests/compile_matrix.py --target esp32s3
python tests/compile_matrix.py --target esp32p4 --touch GSL3680 --camera OV5647
python tests/compile_matrix.py --backend SSD1306 --touch GT911
python tests/compile_matrix.py --minimal
powershell -NoProfile -ExecutionPolicy Bypass -File tests/run_arduino_build.ps1
```

`compile_matrix.py` aceita `--packages`, `--backend`, `--touch`, `--camera`, `--full-fonts`, `--calibration` e `--filter` (termos separados por vírgula). Compila os fontes reais com as flags do core instalado, sem substituição por mocks. O teste de contratos dos backends verifica nomes/assinaturas de chamadas, mas não substitui compilação. `tests/sketches/compile_smoke` permite compilar e linkar pela Arduino IDE.

Resultados verificados em 25/11/2026:

| Verificação | Resultado |
|---|---|
| Algoritmos C/C++, limites numéricos, sentinelas de buffers, ring e métricas de texto | Passou |
| Contratos de armazenamento SD | Passou |
| Contratos de 5 backends, 50 configurações e 9.918 chamadas ativas | Passou |
| ESP32 + Arduino GFX + XPT2046 | 58/58 fontes compilados |
| ESP32-P4 + Arduino GFX + OV5647 + GSL3680 | 58/58 fontes compilados |
| ESP32-S3 + SSD1306 + GT911 | 58/58 fontes compilados |
| ESP32-S3 + PCD8544 | 58/58 fontes compilados |
| ESP32-S3 + ST7565 | 58/58 fontes compilados |
| ESP32-S3 + U8g2, recursos mínimos | 58/58 fontes compilados |
| ESP32-S3 + U8g2, imagens habilitadas | 58/58 fontes compilados |
| ESP32-S3 + Arduino GFX, recursos mínimos | 58/58 fontes compilados |
| Ramos de FT6336, CST816, GT911, GT911 Waveshare, CST820, GSL3680 e XPT2046/calibração de 4 pontos | Compilados |
| Compilação e link completos pelo builder da Arduino IDE 1.8.19, ESP32-S3 com PSRAM OPI e core 3.3.12 | Passou; arquivos ELF e BIN gerados |

Os fontes alterados na revisão final também são recompilados nas configurações afetadas. As compilações por arquivo usam diagnóstico nível 4 para verificar caminhos de logging. O build da IDE usa o padrão do sketch.

O sketch `compile_smoke`, com a configuração padrão da biblioteca, ocupou **490.896 bytes de flash (37%)** e **41.360 bytes de variáveis globais (12%)**. Esses números são do teste de compilação/link; não medem o heap durante uso nem representam o consumo do sketch real da aplicação. Nenhum firmware foi enviado à placa.

O log do build completo fica em `tmp/arduino-smoke.log`; os resultados por configuração ficam em `tmp/correcoes-build/`.

## Roteiro para o teste pessoal

1. Compile seu sketch real na branch com o core 3.3.12 e a configuração da sua placa. Verifique touch, teclados, troca de telas, botões e callbacks.
2. Teste calibração/reinicialização do touch e SPI compartilhado com display/SD. Depois de encerrar o touch, confirme que os demais dispositivos continuam funcionando.
3. Atualize textos e gráfico a partir de outra tarefa. Teste escalas negativas, `INT_MIN`/`INT_MAX`, faixas sem intervalos e dimensões inválidas; observe responsividade e integridade do desenho.
4. Confira imagens mascaradas e rotações 0°, 90°, 180°, 270° e intermediárias, especialmente bordas e imagens de dimensões ímpares.
5. Se usar câmera P4: teste os quatro sentidos, zoom, overlays, abrir/fechar ROI repetidamente, formatos não suportados e ausência do sensor. Verifique preview de 66×66 e outros tamanhos não múltiplos da linha de cache.
6. Remova/recoloque o SD durante logs; provoque fila cheia e confira o contador. Repita início/encerramento por pelo menos 100 ciclos e compare heap livre, maior bloco livre e número de tarefas após estabilizar.

Ainda dependem da placa: ganho de FPS/latência, consumo real de heap/PSRAM, integridade de cache/DMA e comportamento elétrico dos drivers. Não foram estimados percentuais de ganho.
