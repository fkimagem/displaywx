# Exemplo com Led (widget LED) — Pull-up no GPIO22

Este exemplo usa **pull-up** para o botão no **GPIO22** e espelha o estado no **LED da interface** (widget `Led`) com a biblioteca **DisplayWX**.  
Como o pino fica **alto** por padrão (pull-up), **apertar o botão** puxa para **LOW**; por isso, a lógica é **invertida** no código.

---

## Breve descrição do projeto

- O projeto inicializa display/touch e cria um widget `Led` na tela.
- Configuração do botão com **pull-up interno** no `setup()`:
  ```cpp
  pinMode(pushPin, INPUT_PULLUP);
  ```
- No `loop()`, a leitura é invertida para acender o LED quando o botão estiver pressionado (nível LOW):
  ```cpp
  led.setState(!digitalRead(pushPin)); // Pressionado (LOW) → LED ligado
  ```
- A aparência do LED é definida em `LedConfig` (raio e cor quando ligado).

---

## Ligações do hardware (botão com pull-up)

**Pull-up (interno)** — esquema simples recomendado:
```
ESP32 (GPIO22) ──[ Botão ]── GND
```
- **Solto:** leitura **HIGH** (LED **apagado**).  
- **Pressionado:** leitura **LOW** (LED **aceso** via inversão no código).

> Se preferir usar **pull-up externo** (resistor para 3V3), mantenha a mesma lógica: o botão fecha para GND → leitura LOW ao pressionar → use `!digitalRead(...)`.

---

## Imagem da montagem

![montagem](../montagem.png)

Ou no esp32-S3

![montagemS3](../montagemS3.png)

---

## Print da tela do projeto

![Imagem da tela](screenshot.png)

---

## Resumo

Com o **pull-up**, o botão fica estável em HIGH e vai a LOW quando pressionado.  
Invertendo a leitura (`!digitalRead(pushPin)`), o widget `Led` acende exatamente enquanto o botão estiver pressionado, oferecendo **feedback visual imediato** na interface.
