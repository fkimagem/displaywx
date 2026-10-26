# Exemplo com VBar (barra vertical)

Este exemplo mostra como **exibir o nível lido de um potenciômetro** em uma **barra vertical (VBar / VerticalBar)** na tela touchscreen usando a biblioteca **DisplayWX**.  
O widget é atualizado com a leitura **ADC (0–4095)** do ESP32 no **GPIO17**.

---

## Breve descrição do projeto

- O projeto inicializa display/touch e cria um `VBar` configurado com:
  - `minValue = 0`, `maxValue = 4095`
  - `width` e `height` da barra
  - `filledColor` e `orientation = VERTICAL`
- No `loop()`, o valor do potenciômetro é lido e enviado ao widget:
  ```cpp
  vertbar.setValue(analogRead(potPin)); // 0..4095
  ```
- As seções **(ESCRITO MANUAL)** adicionam:
  - definição do pino (`int potPin = 17;`)
  - `pinMode(potPin, INPUT);`
  - atualização contínua do VBar no `loop()`.

---

## Ligações do hardware (potenciômetro)

```
3V3 ──┬─────────┐           
      │         │               (terminal central - wiper)
      │      [ Potenciômetro ] ────► ESP32 (GPIO17)  // pino analógico
      │         │
GND ──┴─────────┘
```

- O **wiper** (terminal central) vai ao **GPIO17**.  
- Extremidades do potenciômetro em **3V3** e **GND**.  
- A escala do VBar (0–4095) casa com o ADC de 12 bits do ESP32.

---

## Imagem da montagem

![montagem](../montagem.png)

Ou no esp32-S3

![montagemS3](../montagemS3.png)

---

## Print da tela do projeto

![screenshot](screenshot.png)

---

## Resumo

O `VBar` é ideal para visualizar **níveis** de forma direta.  
Com `analogRead(potPin)` alimentando o widget, a barra vertical reflete instantaneamente o valor do potenciômetro na faixa **0–4095**.
