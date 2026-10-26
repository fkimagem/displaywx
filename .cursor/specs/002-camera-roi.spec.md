# 002 — ROI, fps e configuração da câmera MIPI

Status geral: feito na branch `camera/002-roi`.

Como usar este documento:

- `implementar` aplica os itens com status `pendente`, na ordem numérica.
- `implemente item N` aplica só o item N. Os itens 2 em diante assumem os anteriores já feitos.
- Depois de implementar um item, trocar o status dele para `feito` e anotar o que mudou na seção "Registro".

Não muda a resolução do sensor. O modo continua o que `startCamera()` escolheu. O recorte é feito no frame já capturado, no PPA, como `blitCameraDmaCropped()` faz hoje.

Contexto do código hoje:

- Uma task, `TaskCamera`, espera `kCameraFramePeriodMs` (33 ms) e chama `cameraTask()`. Há um sensor e um frame por vez. Vários `CameraMipi` desenham esse mesmo frame.
- `cameraTask()` volta sem capturar quando `WidgetBase::usingKeyboard` está ligado.
- `captureFrame()` segura o mutex da câmera até `releaseFrame()`. O buffer volta ao driver. Não fica cópia do último frame.
- O toque no widget liga `m_fullScreen` e `s_hasFullScreen`. `updateCameraMipi()` desenha só esse widget, no retângulo da tela, com `getZoomLevel()`. `updateWidgets()` não desenha o resto enquanto houver tela cheia.
- `setZoomLevel()` aceita 1, 2 ou 4. O 4 desenha o texto "3x". O zoom limita o crop central (`cam / zoom`) e o PPA amplia até o widget. A escala do PPA trunca para `n/16`; `cameraPpaExactCrop()` procura um crop em que `crop * n / 16` seja exatamente o destino.
- O numpad é o modelo da tela modal: `open()` no widget, `usingKeyboard`, toque tratado em `serviceNumpad()`, RETURN confirma, ESC cancela, e a task da câmera fica parada enquanto o teclado está aberto.

Fora de escopo: crop no registrador do sensor, um fps por widget, gravar a ROI em NVS, gesto de arrastar na miniatura, e trocar o modo de resolução em tempo de execução.

---

## Item 1 — Estado da ROI e fps de pré-visualização

Status: feito.

Onde: `CameraMipiConfig` e `CameraMipi` em `src/widgets/cameramipi/wcameramipi.h` e `wcameramipi.cpp`. Fps em `DisplayFK`, `src/displayfk.h` e `src/displayfk.cpp`.

Problema: o widget só conhece largura, altura e um zoom de três valores. Não há centro do recorte, nem a escolha do que a tela cheia mostra, nem um ritmo de miniatura separado do ritmo máximo.

Mudança:

1. Acrescentar em `CameraMipiConfig`:
   - `centerPointX` e `centerPointY`, `int16_t`, padrão `-1`. Valor negativo significa o centro do frame capturado, calculado quando a largura e a altura do frame forem conhecidas.
   - `keepZoomOnMaximized`, `bool`, padrão `false`.
2. Guardar no widget o centro efetivo, o nível de zoom (`uint8_t`, padrão 1) e o valor do checkbox. `setup()` copia a config. Centro negativo permanece “ainda não resolvido” até o primeiro frame.
3. `setZoomLevel(level)` passa a aceitar qualquer nível `>= 1`. Nível `0` é rejeitado com `ESP_LOGW`. Os níveis 1, 2 e 4 usados em `examples/generic/ESP32P4/cameras/p4_eye/p4_eye.ino` continuam válidos.
4. `DisplayFK::setCameraPreviewFps(uint8_t fps)` grava o fps das miniaturas. Valor fora de 5–24 é limitado a esse intervalo, com `ESP_LOGW`. O padrão é 5. `cameraPreviewFps()` devolve o valor guardado.
5. Getters no widget: centro efetivo, nível de zoom, `keepZoomOnMaximized()`, e um setter `setKeepZoomOnMaximized(bool)` para o sketch. O item 6 é quem altera esses campos pela tela de configuração.

Aceite: um sketch que não preenche os campos novos continua compilando e começa com centro no meio do frame, zoom 1 e checkbox desligado. `setCameraPreviewFps(3)` guarda 5. `setCameraPreviewFps(30)` guarda 24. `setZoomLevel(2)` continua aceito. Nenhum pixel muda ainda.

---

## Item 2 — ROI proporcional ao widget na miniatura

Status: feito.

Onde: `cameraZoomCrop()`, `blitCameraDmaCropped()` e `updateCameraMipi()` em `src/displayfk.cpp`. Leitura do centro e do zoom em `CameraMipi`.

Problema: o crop é sempre central e o tamanho sai só de 1, 2 ou 4. A miniatura precisa mostrar a região escolhida para aquele widget, com a mesma proporção do widget, ampliada até encher o retângulo do widget.

Mudança:

1. Calcular o crop máximo, par, com a proporção `getWidthPlot() / getHeightPlot()`, o maior que ainda cabe no frame. Zoom 1 usa esse crop. Zoom `z` divide a largura e a altura por `z`, mantém par, e não passa desse máximo.
2. O centro do crop é `centerPoint` do widget. Se ainda for negativo, usar o centro do frame e gravar esse centro no widget. Deslocar o centro o mínimo necessário para o crop inteiro ficar dentro do frame.
3. Ajustar o crop para um tamanho par em que a escala `n/16` do PPA caia exatamente em `getWidthPlot()` × `getHeightPlot()`, com a mesma escala nos dois eixos. Se o nível pedido passar do fator máximo do PPA no IDF ligado ao build, reduzir o nível até o blit ser aceito e registrar `ESP_LOGW` uma vez por widget.
4. `updateCameraMipi()`, no ramo da miniatura, continua chamando `blitCameraDmaCropped()` com a posição e o tamanho do widget. O crop e a escala vêm da ROI desse widget, não de um zoom global.
5. Rotação e espelho continuam os de `setRotationCamera()`. O centro e o crop estão no espaço do frame capturado, antes da rotação do PPA.
6. O texto do zoom, quando `showZoomLevel()` for verdadeiro, mostra `1x`, `2x`, `4x` conforme o nível. O nível 4 deixa de imprimir `3x`.

Aceite: dois widgets no mesmo frame podem ter centros e zooms diferentes e cada um desenha o seu recorte. Zoom 1 mostra o maior recorte com a proporção do widget. Aumentar o zoom encolhe o recorte em volta do centro e o widget continua preenchido. `p4_eye` com `setZoomLevel(1|2|4)` muda o tamanho desse recorte. O sensor permanece no tamanho passado a `startCamera()`.

---

## Item 3 — Tela cheia: frame inteiro ou a mesma ROI

Status: feito.

Onde: ramo `onlyFullScreen` de `updateCameraMipi()` e `blitCameraDmaCropped()` / `blitCameraDmaFullScreen()` em `src/displayfk.cpp`.

Problema: a tela cheia corta um pedaço do tamanho do display em escala 1 quando o frame é maior que o painel. O uso pedido é outro: ver a imagem completa encaixada na tela, ou ver só a ROI da miniatura, em tempo real.

Mudança:

1. O toque no widget continua abrindo a tela cheia, como `CameraMipi::detectTouch()` faz hoje. Sair da tela cheia continua por `exitFullScreen()` / `leaveFullScreen()`.
2. Com `keepZoomOnMaximized == false`, o centro e o zoom são os da miniatura. O recorte usa a proporção da tela e a escala enche o display inteiro, sem faixa preta e sem o texto de fps. O rótulo de zoom continua só se `showZoomLevel()` estiver ligado.
3. Com `keepZoomOnMaximized == true`, a origem é a mesma ROI da miniatura daquele widget. A mesma regra de encaixe: a ROI inteira visível na tela, faixas pretas se a proporção da ROI e a do display divergirem. Sem esticar.
4. O encaixe também respeita `n/16` e o fator máximo do PPA. Se a ROI não puder encher a área útil, usar o maior destino par que a escala aceite e centrar, com faixa preta no resto.
5. Enquanto `hasFullScreen()` for verdadeiro, os outros widgets de câmera não recebem blit. `updateWidgets()` continua retornando cedo nesse estado.

Aceite: checkbox desligado cobre a tela inteira com o mesmo centro e o mesmo zoom da miniatura, sem fps. Checkbox ligado mostra a mesma região da miniatura, maior, sem mostrar o resto da cena. Voltar da tela cheia redesenha a tela de widgets que estava por baixo.

---

## Item 4 — 5–24 fps na miniatura e o máximo na tela cheia

Status: feito.

Onde: `TaskCamera` e `kCameraFramePeriodMs` em `src/displayfk.cpp`. Rótulo em `drawCameraZoomLabel()` ou numa função ao lado, chamada por `updateCameraMipi()`.

Problema: a espera da task é 33 ms fixos, na miniatura e na tela cheia. A miniatura da esteira precisa ir devagar. A tela cheia precisa acompanhar o frame o mais rápido que a captura e o PPA permitirem.

Mudança:

1. Com nenhum widget em tela cheia, `ulTaskNotifyTake` espera `1000 / cameraPreviewFps()` milissegundos. O padrão 5 espera 200 ms. O sensor não é reprogramado; `captureBuffer()` entrega o frame disponível e os intermediários se perdem.
2. Com `hasFullScreen()`, a espera extra é 1 ms. O intervalo real fica o de `captureBuffer()` mais o blit.
3. Na miniatura, desenhar no canto do widget o fps configurado, no formato `5 fps`, em texto branco. Isso independe de `showZoomLevel()`.
4. Na tela cheia, desenhar no canto o fps medido entre dois blits concluídos daquele modo, atualizado uma vez por segundo, no formato `28 fps`.
5. `setCameraPreviewFps()` durante a tela cheia só muda a espera usada na volta às miniaturas.

Aceite: sem tela cheia, o intervalo entre capturas fica na faixa do fps configurado (5 fps cerca de 200 ms entre um `captureFrame` e o próximo). Ao abrir a tela cheia, a espera de 200 ms some e o número do canto passa a ser a taxa medida. Ao fechar, a espera da miniatura volta. Vários widgets na mesma tela não disparam capturas extras: uma captura, um blit por widget visível.

---

## Item 5 — Cópia do último frame

Status: feito.

Onde: `CameraController::captureFrame()` / `releaseFrame()` e a task da câmera. Buffer novo ao lado de `m_cameraPreviewFb` em `DisplayFK`.

Problema: `openConfig()` precisa desenhar o último frame parado. O buffer do driver só existe entre `captureFrame()` e `releaseFrame()`, e `cameraTask()` solta o frame no fim de cada volta.

Mudança:

1. Alocar um buffer alinhado a 128 bytes, `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT`, com `largura * altura * 2` do frame. Alocar na hora do pedido e liberar ao fechar a configuração. Se a alocação falhar, o pedido falha com `ESP_LOGE` e a câmera ao vivo segue.
2. O pedido nasce na task de UI ou no `loop()` do sketch. A cópia nasce na task da câmera, com o mutex da câmera ainda seguro, antes de `releaseFrame()`. `memcpy` do RGB565. Um `TaskNotify` avisa quem pediu.
3. Quem pediu espera no máximo 500 ms. Estouro de tempo libera o buffer, registra `ESP_LOGW` e não abre a tela.
4. A cópia guarda também largura e altura. Ela não é atualizada de novo enquanto a tela de configuração estiver aberta.
5. O destrutor de `DisplayFK` libera esse buffer depois que a task da câmera parou, no mesmo ponto em que `m_cameraPreviewFb` já é liberado.

Aceite: um pedido com a câmera a capturar produz um buffer na PSRAM do tamanho do frame e a task segue soltando os frames seguintes. Sem PSRAM livre, nada é aberto e a miniatura continua. O sensor não muda de modo.

---

## Item 6 — `openConfig()` no modelo do teclado numérico

Status: feito.

Onde: classe nova ao lado do widget, no estilo de `Numpad` em relação a `NumberBox`. Sugestão de arquivos: `src/widgets/cameramipi/cameraroi.h` e `cameraroi.cpp`. `CameraMipi::openConfig()` e o despacho de toque em `DisplayFK`, no mesmo lugar em que `serviceNumpad()` trata o numpad. Sem desenho de tecla dentro de `displayfk.cpp`.

Problema: o enquadramento da esteira é escolhido olhando a cena, não por número. A tela cobre o display, mostra o frame parado, um retângulo com a proporção do widget, e controles para zoom, posição e o checkbox.

Mudança:

1. `CameraMipi::openConfig()` pede a cópia do item 5 para aquele widget. Se a tela cheia desse ou de outro widget estiver ativa, fecha a tela cheia antes, para a task poder entregar um frame e a UI voltar a desenhar. Sem cópia, retorna `false`.
2. Com a cópia pronta, ligar `WidgetBase::usingKeyboard`, guardar o widget editado e um rascunho de centro, zoom e `keepZoomOnMaximized`. A task da câmera, que já retorna cedo com `usingKeyboard`, deixa de capturar. O frame na tela é só a cópia.
3. Desenhar a cópia com a mesma rotação e o mesmo espelho da visão ao vivo, encaixada na área acima de uma faixa de botões. Faixas pretas se a proporção do frame e a dessa área divergirem. O retângulo da ROI é o crop do item 2 mapeado por essa mesma escala e essa mesma origem. A proporção do retângulo é a do widget.
4. Faixa de botões: `-`, `+`, setas para cima, baixo, esquerda e direita, o checkbox com o texto `keep zoom on maximized`, `OK` e `ESC`.
5. `+` aumenta o nível de zoom do rascunho. `-` diminui, mínimo 1. Cada seta move o centro em passos pares de pelo menos 2 px e cerca de 1/16 do lado correspondente da ROI, com o crop inteiro dentro do frame. Cada toque redesenha a partir da cópia. Não há captura nova.
6. O checkbox só altera o rascunho.
7. `OK` grava centro, zoom e checkbox no widget, desliga `usingKeyboard`, libera a cópia e pede `loadScreen` da tela do widget, como o numpad faz com o number box. `ESC` descarta o rascunho, desliga `usingKeyboard`, libera a cópia e restaura a mesma tela.
8. O dedo que chamou `openConfig()` não aciona um botão até soltar, igual a `m_ignoreOpeningTouch` do numpad. Com a tela aberta, `updateWidgets()` e o blit ao vivo não desenham por cima.

Aceite: chamar `openConfig()` no callback de um botão cobre a tela com o último frame, parado. `+` encolhe o retângulo em volta do centro. As setas andam o retângulo sem deixá-lo sair do frame. `ESC` deixa a miniatura como estava. `OK` faz a miniatura passar a mostrar essa ROI no fps do item 4. O checkbox, depois do `OK`, muda o comportamento da tela cheia do item 3. Durante a configuração não chega frame novo ao PPA ao vivo.

---

## Ordem

1, depois 2, depois 3 e 4 em qualquer ordem entre si, depois 5, depois 6.

Teste no hardware, com o frame maior que o display: miniatura a 5 fps com dois centros diferentes; tela cheia nos dois estados do checkbox; `openConfig()` com OK e com ESC; `p4_eye` ainda responde a `setZoomLevel(1)`, `setZoomLevel(2)` e `setZoomLevel(4)`.

## Registro

| Item | Status | Nota |
| --- | --- | --- |
| 1 | feito | `CameraMipiConfig` ganhou centro e `keepZoomOnMaximized`. `setZoomLevel` aceita qualquer nível >= 1. `setCameraPreviewFps` guarda 5–24, padrão 5. |
| 2 | feito | A miniatura recorta a proporção do widget em volta do centro. Zoom 4 escreve `4x`. A escala do PPA é `n/16`; se passar do máximo do IDF, o nível usado no blit desce e há um `ESP_LOGW` por widget. |
| 3 | feito | Checkbox desligado cobre a tela com o centro e o zoom da miniatura, sem fps. Checkbox ligado encaixa a mesma ROI, com faixa preta se a proporção divergir. |
| 4 | feito | Sem tela cheia a task espera `1000/fps` ms. Na tela cheia espera 1 ms. O canto mostra o fps configurado na miniatura e o fps medido na tela cheia. |
| 5 | feito | A cópia RGB565 nasce na task da câmera, em PSRAM alinhada a 128 bytes, antes de `releaseFrame`. Quem pediu espera no máximo 500 ms. |
| 6 | feito | `CameraMipi::openConfig()` abre `CameraRoi` no modelo do numpad: frame parado, retângulo da ROI, zoom, setas, checkbox, OK e ESC. |
