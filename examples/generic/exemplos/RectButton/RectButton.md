# Exemplo com RectButton

Esse exemplo mostra como controlar uma saída digital (LED) a partir da interação com um **RectButton** exibido na tela touchscreen.  
Quando o usuário pressiona ou solta o `RectButton`, o LED conectado ao **GPIO16** é acionado, refletindo visualmente o estado do widget.

---

## Breve descrição do projeto

- O projeto utiliza a biblioteca **DisplayWX** em conjunto com o ESP32.  
- Um `RectButton` é exibido na tela para interação via toque.  
- Cada alteração no `RectButton` aciona o **callback**, que atualiza o LED físico.  
- O LED acende quando o `RectButton` está ativo e apaga quando está inativo.

---

## Ligações do hardware

O LED deve ser conectado da seguinte forma:



    ESP32 (GPIO16) ──► Resistor (220 a 330 Ω) ──► Ânodo do LED
                                       Cátodo do LED ──► GND

------------------------------------------------------------------------

## Imagem da montagem

![montagem](../montagem.png)

Ou no esp32-S3

![montagemS3](../montagemS3.png)

---

## Print da tela do projeto

![Imagem da tela](screenshot.png)

---

## Resumo

Esse exemplo mostra de forma simples como **sincronizar widgets da interface touchscreen com componentes físicos** no ESP32.  
O `RectButton` controla diretamente o LED, tornando fácil expandir a lógica para outros atuadores ou indicadores.