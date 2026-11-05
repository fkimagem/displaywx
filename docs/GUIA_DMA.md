# Guia do DMA da câmera

Este texto explica o DMA usado nos exemplos de câmera do ESP32-P4. A linguagem é simples. Cada linha de código tem um comentário.

Os exemplos ficam em `examples/generic/ESP32P4/cameras/`. Todos chamam `startPpa()`. Esse nome liga o bloco de hardware que carrega a foto da câmera até a memória da tela.

[TOC]

## Sumário

1. [O que é DMA](#1-o-que-é-dma)
2. [Por que a câmera precisa disso](#2-por-que-a-câmera-precisa-disso)
3. [O caminho da foto](#3-o-caminho-da-foto)
4. [O que o exemplo pede](#4-o-que-o-exemplo-pede)
5. [Quem repete isso sozinho](#5-quem-repete-isso-sozinho)
6. [A foto grande vira o retângulo da tela](#6-a-foto-grande-vira-o-retângulo-da-tela)
7. [A cópia que o processador faz](#7-a-cópia-que-o-processador-faz)

---

## 1. O que é DMA

DMA quer dizer acesso direto à memória.

Pense numa pilha de caixas. Sem DMA, o processador pega uma caixa, anda até o outro lado e solta. Depois volta e pega a próxima. Uma foto tem mais de dois milhões de caixas. O processador ficaria ocupado só com isso.

Com DMA, outro funcionário da placa carrega a pilha inteira. O processador só diz: a foto está aqui, a tela está ali, pode levar.

Nos exemplos, esse funcionário é o PPA do ESP32-P4. PPA quer dizer acelerador de pixels. A função `startPpa()` contrata esse funcionário. Sem ela, a foto é capturada e não chega na tela por esse caminho.

---

## 2. Por que a câmera precisa disso

Uma foto de 1920 por 1080 tem 2.073.600 pontos. Cada ponto é uma cor de 2 bytes. Uma foto ocupa cerca de 4 megabytes.

O retângulo na tela é bem menor. No exemplo da Guition de 7 polegadas ele tem 232 por 184 pontos. Alguém precisa escolher um pedaço da foto grande, diminuir e colocar nesse retângulo.

Se o processador fizesse isso ponto por ponto, a tela ficaria lenta e o resto do programa esperaria. O DMA faz o transporte. O processador só marca o começo e o fim.

---

## 3. O caminho da foto

A foto ao vivo passa por três lugares. O processador não copia os pontos no meio do caminho.

```text
câmera  ->  memória da foto  ->  memória da tela  ->  o que você vê
            (buffer)            (framebuffer)
            DMA da câmera       DMA do PPA
```

1. A câmera MIPI escreve a foto numa área de memória. Isso já é DMA. A função `captureFrame()` pega esse buffer pronto.
2. O PPA lê essa foto e escreve o pedaço certo direto na memória da tela DSI. A função que faz isso se chama `blitCameraDmaCropped()`.
3. O painel DSI lê essa mesma memória e acende os pontos. Não existe uma terceira cópia para "enviar" a imagem.

Estes exemplos usam tela DSI: Guition 7, Guition 4, Waveshare 10, Functional EV Board, ESP32-P4-EYE e os dois dashboards. Neles o destino do DMA é o framebuffer do `Arduino_DSI_Display`.

O `loop()` do sketch não desenha a câmera. Ele só espera. A tarefa da câmera, criada por `createTask()`, repete o caminho acima.

---

## 4. O que o exemplo pede

O trecho abaixo é o `setup` de `examples/generic/ESP32P4/cameras/guition_7in_yellow/guition_7in_yellow.ino`. Os comentários dizem a parte que interessa ao DMA.

```cpp
void setup() {
    Serial.begin(115200);

    // Guarda rotação e espelho. O PPA aplica isso enquanto carrega a foto.
    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);

    // Abre o sensor. 1920x1080 é o tamanho da foto na memória, não o tamanho na tela.
    if (myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, CAM_FREQUENCY_SCCB,
                              CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)) {
        // Contrata o PPA. A partir daqui a foto pode ir por DMA até a tela.
        myDisplay.getCamera().startPpa();
    } else {
        Serial.println("Camera not started");
    }

    // Sobe o painel DSI. tft->begin() cria o framebuffer, a memória que a tela lê.
    bus = new Arduino_ESP32DSIPanel(/* tempos do JD9165 */);
    tft = new Arduino_DSI_Display(1024, 600, bus, rotationScreen, true, DISP_RST,
                                  jd9165_init_operations,
                                  sizeof(jd9165_init_operations) / sizeof(lcd_init_cmd_t));
    tft->begin();

    // Entrega essa tela à biblioteca. O DMA da câmera passa a escrever nesse framebuffer.
    myDisplay.setDrawObject(tft);

    loadWidgets();                  // Define o retângulo onde a foto vai aparecer.
    myDisplay.loadScreen(screen0);  // Desenha o fundo uma vez.
    myDisplay.createTask(false, 3); // Cria a tarefa que repete a captura e o DMA.
}
```

O retângulo fica em `loadWidgets()`. A câmera continua gravando 1920x1080. O widget só diz o buraco na tela.

```cpp
void loadWidgets() {
    CameraMipiConfig configCameraMipi0 = {
        .width = 232,          // Largura do retângulo na tela, em pontos.
        .height = 184,         // Altura do retângulo na tela.
        .showZoomLevel = false,
        .showFps = false,
        .fps = 5               // A miniatura troca de foto 5 vezes por segundo.
    };

    cameraMipi.setup(configCameraMipi0);
    myDisplay.setCameraMipi(arrayCameraMipi, qtdCameraMipi);
}
```

O `loop()` fica vazio de propósito. Desenhar a câmera aqui, ponto por ponto, seria o caminho sem DMA.

```cpp
void loop() {
    delay(100); // A tarefa da câmera trabalha sozinha.
}
```

Os outros exemplos da pasta repetem esse pedido. Mudam o painel, o sensor e o tamanho do retângulo. O DMA é o mesmo: `startCamera`, `startPpa`, `setDrawObject` e `createTask`.

---

## 5. Quem repete isso sozinho

`createTask()` sobe uma tarefa chamada de câmera quando `startCamera()` deu certo. Cada volta faz quatro coisas.

```cpp
void DisplayWX::cameraTask() {
    if (!m_camera.captureFrame()) {
        return; // A câmera ainda não tem uma foto nova.
    }

    ESPVideoBufferClass *buffer = m_camera.buffer(); // Ponteiro da foto. Não é uma cópia.

    // O PPA escreve no framebuffer. O semáforo impede outro desenho ao mesmo tempo.
    if (xSemaphoreTake(m_transactionSemaphore, 0) == pdTRUE) {
        updateCameraMipi(buffer);
        xSemaphoreGive(m_transactionSemaphore);
    }

    m_camera.releaseFrame(); // Devolve o buffer para a câmera gravar a próxima foto.
}
```

`updateCameraMipi()` olha o widget e chama `blitCameraDmaCropped()`. Essa função não tem um `for` nos pontos. Ela monta um pedido e entrega ao PPA.

O pedido real, enxuto, é este:

```cpp
bool DisplayWX::blitCameraDmaCropped(...) {
    uint16_t *fb = m_cameraDsiDisplay->getFramebuffer(); // Memória que a tela já está lendo.

    ppa_srm_oper_config_t op = {};
    op.in.buffer = src;     // Foto da câmera.
    op.in.pic_w = cam_w;    // 1920, por exemplo.
    op.in.pic_h = cam_h;    // 1080, por exemplo.
    op.in.block_offset_x = crop_x; // Onde começa o pedaço usado.
    op.in.block_offset_y = crop_y;
    op.in.block_w = crop_w;
    op.in.block_h = crop_h;

    op.out.buffer = fb;     // Destino: a própria memória da tela.
    op.out.block_offset_x = out_x; // Canto do retângulo dentro dessa memória.
    op.out.block_offset_y = out_y;
    op.scale_x = scale;     // Quanto diminuir ou aumentar.
    op.scale_y = scale;
    op.rotation_angle = cameraPpaAngle(m_cameraRotation);
    op.mode = PPA_TRANS_MODE_BLOCKING; // Espera o DMA terminar.

    ppa_do_scale_rotate_mirror(ppa_srm, &op); // O hardware carrega os pontos.
}
```

`PPA_TRANS_MODE_BLOCKING` quer dizer: espera essa carga acabar antes de devolver o buffer à câmera. Assim a próxima foto não apaga a atual no meio do transporte.

Na miniatura, a tarefa espera o tempo do `fps`. Com `fps = 5`, a espera é 200 ms. Na tela inteira a espera cai para 1 ms, para a imagem acompanhar a câmera.

---

## 6. A foto grande vira o retângulo da tela

A foto e o retângulo quase nunca têm a mesma proporção. A biblioteca mede um recorte.

- O recorte fica em volta do centro da foto.
- Zoom 1 mostra o maior pedaço que cabe no retângulo.
- Zoom 2 ou 4 mostram um pedaço menor, e o PPA amplia esse pedaço até encher o retângulo.
- Rotação e espelho, gravados em `setRotationCamera()`, entram no mesmo transporte. Não existe uma segunda passada.

Dois toques no retângulo pedem tela inteira. Aí o destino do DMA deixa de ser o retângulo pequeno e passa a ser a tela toda. Outro toque volta ao retângulo. O sensor continua na mesma resolução. Quem muda é o recorte e o lugar de escrita.

---

## 7. A cópia que o processador faz

A imagem ao vivo não passa por `memcpy`. O processador só copia uma foto quando alguém abre o ajuste da câmera, com `openConfig()`.

Essa cópia congela um quadro. As setas e o zoom mexem nesse quadro parado. A câmera pode continuar capturando, mas o ajuste não usa os quadros novos. Ao confirmar ou cancelar, a tarefa volta a escrever ao vivo com o DMA.

Se a serial mostrar `PPA DMA client ready`, o funcionário foi contratado. Se mostrar `failed to start PPA DMA client` ou `Camera not started`, o caminho direto não existe e o retângulo fica sem imagem.
