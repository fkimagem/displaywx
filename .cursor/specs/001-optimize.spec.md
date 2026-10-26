# 001 — Otimização de memória, heap e FreeRTOS no DisplayWX

Status geral: em andamento. Itens 1 a 10 feitos na branch `optimize/001-freertos`.

Como usar este documento:

- `implementar` aplica todos os itens com status `pendente`, na ordem numérica.
- `implemente item N` aplica só o item N.
- Cada item é independente: pode ser aceito, adiado ou descartado sem depender dos outros.
- Depois de implementar um item, trocar o status dele para `feito` e anotar o que mudou na seção "Registro".

Referência: [FreeRTOS (IDF) — ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/freertos_idf.html).

Regras desse manual que guiam os itens:

- No IDF FreeRTOS o tamanho de stack em `xTaskCreate*` é em **bytes**, não em words.
- Core 0 (`PRO_CPU`) costuma carregar Wi-Fi e Bluetooth. Core 1 (`APP_CPU`) fica para o restante da aplicação. Afinidade `tskNO_AFFINITY` deixa a task migrar entre os dois cores.
- `xTaskCreateStaticPinnedToCore` evita alocar TCB e stack no heap.
- `vTaskDelete` de uma task que está rodando no outro core é inseguro se ela segura mutex ou ainda não liberou memória. O caminho seguro é a própria task se encerrar com `vTaskDelete(NULL)` depois de limpar os recursos.
- Tasks devem bloquear em fila, notificação ou semáforo. Polling com `vTaskDelay(1)` mantém a task acordando o tempo todo.
- Acesso compartilhado entre cores precisa de primitiva de sincronização. Um mutex adquirido e liberado antes do trabalho protegido não sincroniza nada.

Como a biblioteca é usada:

Referência principal, sem câmera: `C:\Users\Anglo\Documents\Arduino\exemplo_completo_edge_placa`. ESP32-S3, ST7796, FT6336, 10 telas. Widgets: gauge, line chart, label, LED, slider, checkbox, circle/rect/text button, toggle, number box, spin box, radio, vertical bar, vertical analog, circular bar, thermometer e 20 imagens. `createTask(false, 3)` é chamado uma vez. O `loop()` chama `checkChangeScreen`, `startCustomDraw`, o comportamento da tela atual e `finishCustomDraw`, e termina com `delay(10)`. Esse `loop()` lê pots, escreve servo e LEDs e aplica `setValue` / `push`. Callbacks de toque só gravam flags e valores; a troca de tela acontece no `loop()`. No Arduino-ESP32 3.x esse `loopTask` nasce no core 1 (`CONFIG_ARDUINO_RUNNING_CORE`), prioridade 1, stack de 8192 bytes.

Câmera, quando existir:

- `examples/generic/ESP32P4/pico_camera_display` chama `createTask(false, 3)`, desenha widgets e faz blit da câmera na mesma task. Touch GT911 e SCCB da OV5647 compartilham GPIO 7/8.
- `examples/generic/ESP32P4/pico_camera_waveshare` captura frame no `loop()` do sketch, fora da task. Os dois caminhos (`loop` e `cameraTask`) podem chamar `captureFrame` ao mesmo tempo se o sketch também criar a task.

Escopo: `src/displaywx.cpp`, `src/displaywx.h`, `src/widgets/`, `src/touch/`, `src/camera/`. Examples só mudam se um item exigir um ajuste mínimo de chamada.

Fora de escopo: reescrever widgets por estilo, trocar a API pública dos sketches, e otimizar fontes em flash.

---

## Item 1 — Uma única task de UI, fora do core do `loop()`

Status: feito.

Onde: `DisplayWX::createTask` em `src/displaywx.cpp`.

Problema: no exemplo completo o `loop()` do sketch é quem alimenta os widgets e os periféricos (pot, servo, LED, troca de tela). No ESP32-S3 esse `loopTask` está no core 1 com prioridade 1. Uma task de UI no mesmo core com prioridade maior segura esse `loop()` durante cada redraw. O core 0 continua sendo o core de Wi-Fi/Bluetooth; a task de UI fica nele com prioridade 1, abaixo das tarefas de rede.

Mudança:

1. Se já existir `m_hndTaskEventoTouch`, retornar sem criar outra task.
2. Com `portNUM_PROCESSORS > 1`, fixar a task no core oposto a `ARDUINO_RUNNING_CORE`. No S3 isso é o core 0. Em alvo single-core, usar `tskNO_AFFINITY`.
3. Prioridade 1, a mesma do `loopTask`, para os dois lados seguirem quando compartilham um core.
4. Guardar o handle em `m_hndTaskEventoTouch`.

Aceite: uma segunda chamada a `createTask` não cria task nem fila nova. No ESP32-S3 a task aparece no core 0 e o `loop()` do exemplo completo continua no core 1. O sketch continua chamando `createTask` como hoje.

---

## Item 2 — Stack em bytes, alocação estática, medição de folga

Status: feito.

Onde: mesma criação de task do item 1. `configMINIMAL_STACK_SIZE + 3048` hoje soma cerca de 4 KB.

Problema: no IDF o argumento de stack é em bytes. O loop da task entra em `print`, PPA, `dtostrf` e callbacks do usuário. 4 KB estoura fácil no caminho da câmera e fragmenta o heap, porque TCB e stack saem de `pvPortMalloc`.

Mudança:

1. Definir duas constantes em bytes: `DFK_UI_STACK_BYTES` (8192) e `DFK_UI_STACK_CAMERA_BYTES` (12288) quando `HAS_CAMERA_MIPI` e `DFK_CAMERA_MIPI` estão ativos.
2. Alocar stack e TCB com buffers estáticos (`StaticTask_t` + `StackType_t[]`) e criar com `xTaskCreateStaticPinnedToCore`.
3. A cada 5 s, se o log de debug estiver ligado, imprimir `uxTaskGetStackHighWaterMark(NULL)`.
4. Se a folga ficar abaixo de 512 bytes, emitir `ESP_LOGW` uma vez.

Aceite: a task não nasce com `new`/`pvPortMalloc` de stack. O high water mark aparece no log de debug. No exemplo completo, sem câmera no sketch, o tamanho usado é 8192 bytes quando `HAS_CAMERA_MIPI` e `DFK_CAMERA_MIPI` estão desligados. Com esses defines, o tamanho é 12288. Compila nos dois casos.

---

## Item 3 — Watchdog só desta task

Status: feito.

Onde: `DisplayWX::createTask`, ramo `m_enableWTD`. Os examples P4 chamam `createTask(false, 3)`.

Problema: com watchdog desligado o código chama `esp_task_wdt_delete(NULL)` e `esp_task_wdt_deinit()`. Isso remove o TWDT do sistema inteiro, inclusive das idles. Com watchdog ligado, o timeout é refeito para o chip todo (`m_timeoutWTD * 1000` ms) e a task é adicionada com `esp_task_wdt_add(NULL)`.

Mudança:

1. Parar de chamar `esp_task_wdt_deinit` neste fluxo.
2. Se `enableWatchdog` for falso, apenas não registrar esta task. Deixar o TWDT que o Arduino já criou.
3. Se for verdadeiro, `esp_task_wdt_add` no handle da task depois que ela existir, e `esp_task_wdt_reset` só nesse handle (`RESET_WDT` precisa receber o handle, não `NULL` se o reset for feito de outro contexto).
4. O reset continua no fim de `TaskEventoTouch` e nos laços longos que ainda existirem até o item 7.

Aceite: `createTask(false, 3)` não desliga o watchdog das outras tasks. `createTask(true, 3)` só observa `TaskEventoTouch`.

---

## Item 4 — A task se encerra sozinha

Status: feito.

Onde: `DisplayWX::~DisplayWX` e `cleanupFreeRTOSResources`.

Problema: o destrutor apaga semáforos e buffers enquanto `TaskEventoTouch` continua no `for(;;)`. O manual do IDF desaconselha `vTaskDelete` de uma task que roda no outro core, porque ela pode estar com mutex ou memória ainda em uso. Hoje a task nem é apagada: o handle vaza e o próximo acesso aos semáforos é use-after-free.

Mudança:

1. Criar um flag `m_stopTask` (`volatile` ou `std::atomic<bool>`).
2. `cleanupFreeRTOSResources` liga o flag e espera, no máximo 500 ms, `ulTaskNotifyTake` de volta ou o handle zerar.
3. O `for(;;)` de `TaskEventoTouch` testa o flag no início de cada volta. Se estiver ligado: solta mutex que esteja segurando, sai dos laços de teclado, chama `vTaskDelete(NULL)`.
4. A task zera `m_hndTaskEventoTouch` antes de se apagar (gravar `nullptr` numa variável que o destrutor lê; o handle em si pode ser lido depois do delete, então usar um flag `m_taskStopped`).
5. Só então o destrutor apaga semáforos, filas e o preview da câmera.

Aceite: destruir `DisplayWX` não deixa task órfã. Não há `vTaskDelete` chamado de outro core sobre essa task.

---

## Item 5 — Mutex cobre o desenho de verdade

Status: feito.

Onde: `displaywxTask`, `startCustomDraw`, `finishCustomDraw`.

Problema: `displaywxTask` toma `m_transactionSemaphore` por 1 ms, lê `m_runningTransaction` e solta o mutex antes de desenhar. `startCustomDraw` faz o mesmo para gravar o flag e solta o mutex na hora. Entre cores do SMP o flag fica exposto e o sketch pode escrever no TFT ao mesmo tempo que a task. `portMAX_DELAY` no sketch trava para sempre se a task morrer segurando o mutex.

Mudança:

1. `startCustomDraw` toma o mutex com timeout de 100 ms e só devolve em `finishCustomDraw`.
2. `displaywxTask` tenta o mutex com timeout 0. Se não conseguir, volta na próxima iteração sem desenhar.
3. Segurar o mutex durante `updateWidgets` e `cameraTask` (blit no framebuffer).
4. Trocar `m_runningTransaction` por "mutex ocupado pelo sketch". Remover a janela em que o flag é true e o mutex já foi devolvido.
5. Documentar no header: callbacks de widget rodam fora desse mutex (item 6), para o callback poder chamar `startCustomDraw` sem deadlock.

Aceite: desenho do sketch e desenho da task não se intercalam. Um timeout estourado registra `ESP_LOGE` e segue, sem `portMAX_DELAY`.

---

## Item 6 — Fila de callback sem bloquear quem a consome

Status: feito.

Onde: `WidgetBase::addCallback` (`xQueueSend` com 50 ms) e `DisplayWX::processCallback` (`xQueueReceive` com 1 ms). A fila nasce com profundidade 5 em `DisplayWX::setup`.

Problema: quem enfileira o callback no toque é a própria `TaskEventoTouch`, e só ela consome a fila. Com a fila cheia, `xQueueSend(..., 50 ms)` bloqueia o único consumidor. `xQueueReceive` com 1 ms, com a fila vazia, dorme um tick em toda volta, somado ao `vTaskDelay(1)` do fim da task. `setup()` chama `xQueueCreate` de novo se `createTask` repetir, e a fila anterior vaza.

Mudança:

1. Criar `xFilaCallback` uma vez. Profundidade 16.
2. `addCallback` envia com timeout 0. Se a fila estiver cheia, descartar e contar em um `uint32_t` de estouros (logar a cada 32 perdas, não a cada uma).
3. `processCallback` drena com `xQueueReceive(..., 0)` até esvaziar, no máximo 8 callbacks por volta, para um callback lento não monopolizar o frame.
4. Executar o callback depois de soltar o mutex do item 5.
5. No destrutor, `vQueueDelete` depois que a task parou (item 4).

Aceite: uma volta sem toque não dorme dentro de `processCallback`. Oito toques seguidos não bloqueiam a task por 50 ms. A fila não é recriada.

---

## Item 7 — Teclado e numpad sem laço infinito dentro da task

Status: feito.

Onde: `processTextBoxTouch` e `processNumberBoxTouch`. Enquanto `WidgetBase::usingKeyboard` é true, a função faz `getTouch` e `vTaskDelay(10)` e só retorna ao fechar.

Problema: nesse intervalo a task não atualiza os outros widgets, não faz blit da câmera e não drena callbacks. O `RESET_WDT` esconde o bloqueio. O manual pede task em loop que bloqueia em eventos curtos, não um segundo loop aninhado.

Mudança:

1. Extrair o miolo dos dois `while` para `serviceKeyboard()` e `serviceNumpad()`, chamados uma vez por volta de `displaywxTask`.
2. `detectTouch` do textbox/numberbox só abre o teclado e marca `usingKeyboard`.
3. Cada volta lê o touch uma vez, repassa ao teclado ou ao numpad, e segue para `updateWidgets`.
4. Fechar com RETURN/ESC enfileira o callback (item 6) e limpa `usingKeyboard`.

Aceite: com o teclado aberto, `cameraTask` e os widgets visíveis continuam na mesma cadência da task. Não resta `while (WidgetBase::usingKeyboard)` dentro do processamento de toque.

---

## Item 8 — Câmera fora da volta de UI

Status: feito.

Onde: `TaskEventoTouch` chama `displaywxTask()` e em seguida `cameraTask()`. `cameraTask` faz `captureFrame`, blit PPA em modo bloqueante e `releaseFrame`.

Problema: um frame MIPI atrasado segura touch e redraw. O example `pico_camera_waveshare` ainda captura no `loop()` do sketch; se os dois caminhos rodarem, duas tasks chamam o mesmo `CameraController` sem exclusão.

Mudança:

1. Criar `m_cameraBusy` (`std::atomic<bool>` ou mutex curto só em `CameraController`). `captureFrame` / `releaseFrame` recusam reentrância.
2. Mover `cameraTask` para uma task própria, stack estática (item 2, tamanho de câmera), core 1, prioridade 2 (abaixo da UI).
3. A task de câmera bloqueia em notificação (`ulTaskNotifyTake`) com timeout de 1 frame (33 ms a 30 fps), não em `vTaskDelay(1)` junto da UI.
4. `TaskEventoTouch` deixa de chamar `cameraTask`.
5. Se `HAS_CAMERA_MIPI` não estiver definido, essa task não existe.

Aceite: um `captureFrame` lento não atrasa `getTouch`. Duas chamadas simultâneas não entram no driver de vídeo ao mesmo tempo.

---

## Item 9 — Um mutex para o I2C compartilhado

Status: feito.

Onde: `src/touch/share_i2c.cpp` (`TouchI2cMaster`) e o begin do SCCB em `src/camera/cameracontroller/cameracontroller.cpp`. No Pico, touch e câmera usam GPIO 7 e 8.

Problema: `TouchI2cMaster` reutiliza o barramento, mas cada `write` / `requestFrom` segue sem mutex. A task de touch e a task de câmera (item 8), ou o `loop()` do example, podem iniciar transações juntas no mesmo `i2c_master_bus_handle_t`. O array `devices[128]` também ocupa espaço fixo para handles que quase nunca existem; isso é secundário.

Mudança:

1. Criar um mutex no `TouchI2cMaster` (`xSemaphoreCreateMutexStatic` para não usar heap).
2. `beginTransmission` até `endTransmission`, e `requestFrom` até o fim da leitura, seguram esse mutex com timeout de 50 ms (`kTimeoutMs` já existe).
3. `CameraController::begin` toma o mesmo mutex ao configurar o SCCB quando os pinos coincidem com o touch. Expor `TouchI2c.lock()` / `unlock()` só para esse uso, ou uma função `withBus`.
4. Trocar `devices[128]` por um mapa pequeno (até 4 endereços: GT911, auxiliar do painel, SCCB). Realocar handle se o endereço mudar.

Aceite: touch e SCCB no mesmo par de pinos serializam as transações. Timeout de I2C não trava a task.

---

## Item 10 — Touch por evento, com poll lento como reserva

Status: feito.

Onde: `TaskEventoTouch` termina com `vTaskDelay(pdMS_TO_TICKS(1))`. `TouchScreen` guarda `m_pinINT`, e o GT911 tem o `attachInterrupt` comentado em `TAMC_GT911.cpp`. Cada volta chama `getTouch`, que faz transação I2C.

Problema: acordar a cada 1 ms para ler I2C gasta CPU e barramento mesmo com a tela parada. O pino INT já é passado na API (`startTouchGT911`, FT6336, CST820, etc.).

Mudança:

1. Se `m_pinINT` for válido, instalar ISR que só faz `vTaskNotifyGiveFromISR` no handle da task de UI. ISR sem `printf`, sem I2C, sem malloc.
2. A task espera `ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(30))`. Ao acordar por notificação, lê o touch uma vez.
3. O timeout de 30 ms continua lendo o touch para controladores sem INT e para não perder um toque se a IRQ falhar.
4. Remover o `vTaskDelay(1)` fixo do fim da task. O sono passa a ser o `ulTaskNotifyTake`.
5. Deixar o caminho sem pino INT igual ao timeout de 30 ms, sem ISR.

Aceite: com INT ligado, o analisador de CPU mostra a task bloqueada entre toques. Um toque ainda chama `processTouchStatus` na mesma volta em que a notificação chega.

---

## Item 11 — Não percorrer widget limpo e não logar o hot path

Status: pendente.

Onde: `updateWidgets` e cada `update*` em `displaywx.cpp`. `redraw()` dos widgets começa com macros `CHECK_*`. Com `LOG_CHECK_ERRORS` em 1, `CHECK_DEBOUNCE_REDRAW_VOID` emite `ESP_LOGW` a cada rejeição. Vários `redraw()` também logam `ESP_LOGD` no caminho normal (`CameraMipi::detectTouch`, `WidgetBase::setPressed`).

Problema: a cada volta a task caminha todos os arrays (gauge, label, imagem, câmera, …) e entra em `redraw()` só para sair em `CHECK_SHOULDREDRAW`. Log no hot path formata string e pode alocar.

Mudança:

1. Em cada `update*`, continuar se `!widget->shouldRedraw()` (método já existente ou leitura do flag via função pública já usada por `forceUpdate`).
2. Manter `CHECK_SHOULDREDRAW` dentro de `redraw()` como rede de segurança.
3. Deixar `LOG_CHECK_ERRORS` em 0 no código que roda por frame. Não adicionar `ESP_LOG*` novos dentro de `redraw`, `detectTouch` e `displaywxTask`.
4. `CameraMipi::detectTouch` deixa de logar "INSIDE BOUNDS" / "OUTSIDE BOUNDS" em todo toque.

Aceite: com a tela estática, `redraw()` de label/gauge/botão não é chamado. O log serial fica quieto durante um arraste.

---

## Item 12 — Escrita agrupada no display

Status: pendente.

Onde: `DisplayWX::drawPng` (pixel a pixel), `WidgetBase::drawRotatedImageOptimized` (pixel a pixel com `writePixel`), `Label::redraw` / `printText`.

Problema: cada pixel é uma transação com o painel. `drawRotatedImageOptimized` já abre `startWrite`/`endWrite`, mas ainda emite um `writePixel` por ponto. `drawPng` não agrupa. Vários widgets no mesmo frame abrem e fecham a transação SPI/DSI várias vezes.

Mudança:

1. `drawPng` copia a linha visível para um buffer RGB565 na stack (largura máxima 320) ou em PSRAM se a largura passar disso, e chama `draw16bitRGBBitmap` / `writeFastHLine` equivalente da GFX.
2. Na rotação, calcular a linha de destino e escrever com `writeFastHLine` ou bitmap de uma linha, dentro do `startWrite` que já existe.
3. `updateWidgets` abre um `startWrite` em volta dos `redraw()` daquela volta e fecha no fim, se o driver GFX estiver ativo. Widgets que já chamam `startWrite` precisam detectar transação aberta ou remover o par interno para não aninhar.
4. Não agrupar quando `m_runningTransaction` do sketch estiver ativo (item 5): o sketch já é o dono do barramento.

Aceite: um PNG 100×100 não gera 10 mil `drawPixel`. Um frame com vários labels abre uma transação de escrita, não uma por widget.

---

## Item 13 — Preview da câmera alocado uma vez, sync só do retângulo

Status: pendente.

Onde: `ensureCameraPreview`, `blitCameraDmaFullScreen`, `blitCameraDmaCropped`, `syncCameraFramebufferCache`.

Problema: o preview pede primeiro `MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL`. Dois widgets 200×100 e 300×300 (example `pico_camera_display`) pedem até ~180 KB de RAM interna; se o bloco cresce, o anterior é `heap_caps_free` e o heap fragmenta. `syncCameraFramebufferCache` faz `esp_cache_msync` do framebuffer inteiro (720×1280×2 ≈ 1,8 MB no painel de 7").

Mudança:

1. Preferir `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT` alinhado a 128 bytes para o preview. Usar RAM interna só se o PPA rejeitar o buffer.
2. Na configuração dos `CameraMipi`, calcular o maior `width * height` e alocar esse bloco uma vez. `ensureCameraPreview` reutiliza se `m_cameraPreviewFbBytes >= bytes`.
3. `esp_cache_msync` cobre só o retângulo escrito (x, y, w, h), alinhado à linha de cache de 128 bytes, não `native_w * native_h`.
4. O destrutor continua com um único `heap_caps_free` (já existe).

Aceite: dois previews não provocam realloc a cada frame. O sync de cache escala com a área do widget, não com a tela toda.

---

## Item 14 — PPA sem prender a task de câmera no modo bloqueante o tempo todo

Status: pendente.

Onde: `ppa_srm_oper_config_t::mode = PPA_TRANS_MODE_BLOCKING` nos dois blits.

Problema: `ppa_do_scale_rotate_mirror` em modo bloqueante ocupa a task até o DMA acabar. Com a task separada (item 8) a UI segue, mas a captura do próximo frame espera o blit. Em tela cheia isso limita o fps e segura o buffer de vídeo.

Mudança:

1. Usar o modo não bloqueante do PPA quando o IDF do alvo expuser callback de conclusão.
2. A task de câmera espera a conclusão com `ulTaskNotifyTake` e timeout de 50 ms, depois `releaseFrame`.
3. Se o IDF da build não tiver modo não bloqueante, manter o bloqueante e registrar isso num comentário de uma linha no blit. Não inventar API.

Aceite: `releaseFrame` só corre depois do PPA terminar. Um timeout de 50 ms libera o frame e conta um erro, sem laço infinito.

---

## Item 15 — Gauge sem `new[]` para título e faixas

Status: pendente.

Onde: `GaugeSuper::setup` em `src/widgets/gauge/wgaugesuper.cpp`. Limites já existem: `MAX_TITLE_LENGTH` 20 e `MAX_SERIES` 10.

Problema: cada `setup` faz `new char[titleLen + 1]`, `new int[]` e `new uint16_t[]`. O tamanho é fixo e pequeno. `new` no heap interno fragmenta e o caminho de falha parcial já existe (alocou um array e falhou o outro).

Mudança:

1. Trocar os ponteiros por `char m_title[MAX_TITLE_LENGTH + 1]`, `int m_intervals[MAX_SERIES]`, `uint16_t m_colors[MAX_SERIES]`.
2. `setup` só copia com `strncpy` / laço limitado por `MAX_SERIES`.
3. Apagar `cleanupMemory` desses três blocos. O destrutor deixa de dar `delete[]` neles.

Aceite: configurar o gauge não chama `new`/`delete`. Título maior que 20 caracteres continua truncado.

---

## Item 16 — Imagem grande em PSRAM

Status: pendente.

Onde: `Image` em `src/widgets/image/wimage.cpp` (`new pixel_t[bytesOfColor]`, `new uint8_t[maskLen]`, `new uint8_t[lineSize]`).

Problema: uma imagem 480×320 RGB565 são ~300 KB via `new`, que cai no heap interno. O `lineBuffer` já é liberado. O buffer de pixels permanece e compete com stack, filas e DMA.

Mudança:

1. Alocar `m_pixels` e `m_maskAlpha` com `heap_caps_malloc(..., MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)` quando `psramFound()` for true. Sem PSRAM, manter `heap_caps_malloc` com `MALLOC_CAP_8BIT`.
2. Liberar com `heap_caps_free` nos mesmos pontos em que hoje há `delete[]` (`clearBuffers` e falhas de leitura).
3. Manter o `lineBuffer` pequeno com `new`/`delete[]` ou um array automático se `lineSize <= 1024`.
4. `m_ownsMemory` continua decidindo se este objeto libera o bloco.

Aceite: imagem carregada de arquivo em placa com PSRAM não reduz o heap interno no tamanho do bitmap. Falha no meio da leitura não vaza o bloco.

---

## Item 17 — LineChart: pool em PSRAM e mutex com timeout

Status: pendente.

Onde: `LineChart::allocBuffers`, `initMutex`, `redraw` em `src/widgets/linechart/wlinechart.cpp`.

Problema: o pool de amostras usa `malloc` (heap interno). O mutex é `xSemaphoreCreateMutex` no heap e vários pontos usam `xSemaphoreTake(..., portMAX_DELAY)`. Se o redraw da task de UI segurar o mutex e o `loop()` do sketch também esperar para sempre, os dois lados param. `vSemaphoreDelete` no destrutor, com a task ainda em `redraw`, é o caso que o manual cita (mutex preso, task apagada ou semáforo apagado em uso).

Mudança:

1. `heap_caps_malloc` do pool com PSRAM se existir; senão `MALLOC_CAP_8BIT`.
2. Mutex estático (`StaticSemaphore_t` + `xSemaphoreCreateMutexStatic`).
3. `xSemaphoreTake` com timeout de 20 ms. Se falhar, `redraw` e `push` retornam sem desenhar e sem escrever amostra. A amostra perdida pode sobrescrever a última posição numa variável `m_droppedSamples`.
4. `destroyMutex` só depois que a task de UI parou, ou no destrutor do widget se a task não estiver em `redraw` (trancar com timeout 0 e, se ocupado, não deletar; logar o erro).

Aceite: adicionar ponto e redesenhar não usam `portMAX_DELAY`. O pool grande sai da RAM interna quando há PSRAM.

---

## Item 18 — Log fora do frame

Status: pendente.

Onde: `addLog`, `processLogQueue`, `bufferLog[LOG_LENGTH]`, fila `xFilaLog` de 10 × `logMessage_t` (64 bytes copiados por valor). `processLogQueue` faz `Serial.printf` dentro de `displaywxTask`.

Problema: a task de UI bloqueia na UART. A fila e o `bufferLog` guardam o mesmo texto duas vezes. `Serial` também é usado pelo sketch sem mutex.

Mudança:

1. `processLogQueue` sai de `displaywxTask`.
2. Uma task de log, prioridade 1, stack estática de 2048 bytes, bloqueia em `xQueueReceive(..., portMAX_DELAY)` e só então escreve na UART.
3. `addLog` continua com `xQueueSend(..., 0)`. Fila cheia descarta.
4. Remover a cópia para `bufferLog` se nenhum leitor externo usar esse array. Se o header público expõe `bufferLog`, manter o array e copiar na task de log, não na task de UI.
5. Criar a fila uma vez, junto da fila de callback (item 6).

Aceite: `displaywxTask` não chama `Serial.printf`. Um log por segundo não atrasa o redraw.

---

## Item 19 — Medir texto por busca binária

Status: pendente.

Onde: `WidgetBase::getLastLettersForSpace` e `getFirstLettersForSpace` em `src/widgets/widgetbase.cpp`.

Problema: os dois métodos chamam `getTextBounds` uma vez por comprimento possível. Um rótulo de 64 caracteres mede o texto dezenas de vezes por `redraw` de textbox/label quando o texto não cabe. `getTextBounds` percorre a fonte.

Mudança:

1. Busca binária no comprimento que ainda cabe na largura pedida.
2. Escrever o recorte no buffer `out` uma vez, no fim.
3. Manter a assinatura atual.

Aceite: para um texto de N caracteres, o número de `getTextBounds` fica em torno de log2(N), não N. O recorte visível permanece o mesmo para os casos já usados pelos examples de textbox.

---

## Item 20 — Filas e flag de instância criados uma vez

Status: pendente.

Onde: `DisplayWX::setup` (`xQueueCreate` da fila de log e da fila de callback) e o construtor (`DisplayWX::instance = this`). `createTask` chama `setup()` sempre.

Problema: a segunda chamada vaza as duas filas. Um segundo objeto `DisplayWX` no mesmo sketch sobrescreve `instance`, e a task antiga continua chamando o objeto novo. Isso aparece se o sketch declara o display em escopo que reconstrói.

Mudança:

1. `setup` retorna imediatamente se as filas já existem.
2. O construtor, se `instance` já aponta para outro objeto vivo, não sobrescreve e registra `ESP_LOGE`. Uma instância por processo.
3. O destrutor zera `instance` só se `instance == this`, e só depois do item 4.

Aceite: chamar `createTask` duas vezes não aumenta o número de filas no heap. Dois objetos `DisplayWX` não compartilham o ponteiro estático em silêncio.

---

## Ordem sugerida

Itens seguros de fazer primeiro, com pouco efeito colateral: 11, 15, 19, 20, 6, 3.

Itens que mudam concorrência e precisam de teste no hardware: 1, 2, 4, 5, 7, 8, 9, 10.

Itens de cópia de pixels e PSRAM: 12, 13, 14, 16, 17, 18.

## Registro

| Item | Status | Nota |
| --- | --- | --- |
| 1 | feito | Revisto com o exemplo completo. Uma task só. Core oposto a `ARDUINO_RUNNING_CORE` (core 0 no S3, onde o `loop()` está no core 1). Prioridade 1. Single-core usa `tskNO_AFFINITY`. |
| 2 | feito | Sem mudança de código depois do exemplo completo. Stack estática de 8192 bytes sem câmera e 12288 com câmera MIPI. Medição a cada 5 s. |
| 3 | feito | `createTask` não chama `esp_task_wdt_deinit` nem reconfigura o TWDT. `createTask(false, 3)` não registra a task. `createTask(true, 3)` só faz `esp_task_wdt_add` no handle da UI. `RESET_WDT` só reseta se a task atual for `TaskEventoTouch`. |
| 4 | feito | `m_stopTask` / `m_taskStopped`. O destrutor espera até 500 ms a task chamar `vTaskDelete(NULL)`. Semáforos, filas e o preview da câmera só são liberados depois disso. Timeout deixa esses recursos no lugar. |
| 5 | feito | `startCustomDraw` segura o mutex até `finishCustomDraw`, com timeout de 100 ms. A task só desenha widgets, troca de tela e faz blit da câmera com timeout 0. Callbacks ficam fora do mutex. |
| 6 | feito | Fila de callback criada uma vez, profundidade 16. Envio com timeout 0; a cada 32 perdas sai um `ESP_LOGW`. `processCallback` drena no máximo 8 itens com timeout 0, fora do mutex. O destrutor apaga a fila depois que a task para. |
| 7 | feito | O toque no textbox/numberbox só abre o teclado. `serviceKeyboard` e `serviceNumpad` rodam uma vez por volta, com o toque já lido. Só o dedo que abriu o campo é ignorado, até soltar. Com o teclado aberto, `cameraTask`, `loadScreen` e `updateWidgets` não desenham. RETURN/ESC enfileira o callback e fecha. Não resta `while (usingKeyboard)` no processamento de toque. |
| 8 | feito | `TaskCamera` própria, stack estática de 12288 bytes, core 1, prioridade 2. Acorda com `ulTaskNotifyTake` de 33 ms. `TaskEventoTouch` não chama mais `cameraTask`. `captureFrame` segura um mutex até `releaseFrame`; a outra chamada volta sem entrar no driver. O destrutor espera as duas tasks pararem. |
| 9 | feito | Mutex estático no `TouchI2c`. `beginTransmission` até `endTransmission`, e `requestFrom` até o fim da leitura, esperam no máximo 50 ms. `CameraController::begin` usa o mesmo mutex quando SDA/SCL coincidem com o toque. Quatro slots de device no lugar de `devices[128]`; slot cheio remove o handle antigo e cria o endereço novo. |
| 10 | feito | Com `m_pinINT` válido, a ISR só faz `vTaskNotifyGiveFromISR` na task de UI. A task dorme em `ulTaskNotifyTake` de 30 ms e lê o toque ao acordar, por IRQ ou por timeout. Sem pino INT não há ISR. O `vTaskDelay(1)` do fim da task saiu. |
| 11 | pendente | |
| 12 | pendente | |
| 13 | pendente | |
| 14 | pendente | |
| 15 | pendente | |
| 16 | pendente | |
| 17 | pendente | |
| 18 | pendente | |
| 19 | pendente | |
| 20 | pendente | |
