# Cartão SD na DisplayWX

O cartão é opcional. Sem `startSD()`, a tela, o toque e os widgets continuam funcionando. A biblioteca não monta o SD no construtor e não escolhe pinos da placa.

A implementação usa `SD.h`, `FS.h` e `SPI.h` do pacote Arduino-ESP32. Há uma única sessão, ligada ao objeto global `SD`. A instância `SPIClass` pertence ao sketch: `endSD()` desmonta o filesystem e não chama `SPI.end()` nem `delete`.

## Montagem

```cpp
SDConfig config;
config.cs = SD_CS;
config.sck = SD_SCK;
config.miso = SD_MISO;
config.mosi = SD_MOSI;
config.spi = &SPI;
config.frequencyHz = 4000000;
config.initializeSPI = true;
bool pronto = myDisplay.startSD(config);
```

- Com `initializeSPI == true`, o `SPIClass` ainda não pode ter sido iniciado. `SPI.begin()` em uma instância ativa ignora os pinos novos, e a DisplayWX não chama `SPI.end()` para forçar o remapeamento.
- Com `initializeSPI == false`, o sketch já iniciou o mesmo `SPIClass` usado pela tela `Arduino_HWSPI` e pelo touch. SCK, MISO e MOSI podem ficar em `-1`.
- A frequência aceita é de 100 kHz a 20 MHz. O padrão é 4 MHz. Fora da faixa, a montagem falha e nenhum outro valor é aplicado no lugar.
- Repetir `startSD()` com a mesma configuração válida retorna `true` sem remontar. Uma configuração diferente retorna `false` e mantém a sessão. A troca é `endSD()` seguido de `startSD()`.
- Outra instância de `DisplayWX` não assume nem desmonta a sessão da dona.
- Uma montagem feita diretamente por `SD.begin()` fora da DisplayWX é recusada quando o core consegue indicá-la.

`isSDReady()` informa o estado conhecido da montagem. Não é sensor de cartão inserido. `getSD()` devolve o `fs::FS*` montado, ou `nullptr`. Esse ponteiro não segura a sessão: o chamador fecha seus arquivos e não concorre com `endSD()`.

`sdcardOK` permanece por compatibilidade. Código novo consulta `isSDReady()`.

As chamadas `startSD(pin, spi)` e `startSD(pin, spi, hz)` continuam válidas para um SPI já iniciado. `hz == 0` significa 4 MHz. Frequência negativa falha.

## Arquivos

Os caminhos são absolutos na raiz do cartão: `/config.txt`, `/imagens/logo.bin`. Não inclua o prefixo VFS `/sd`.

| Função | Resultado |
| --- | --- |
| `readSDFile(path, String&)` | Devolve o texto, inclusive quebras de linha. Arquivo vazio é sucesso. Acima de `maxBytes` falha sem truncar. |
| `readSDFile(path, buffer, capacity, bytesRead, offset)` | Lê um bloco. Capacidade menor que o arquivo é válida. Offset exatamente no fim devolve zero bytes. Offset além do fim falha. |
| `writeSDFile` | Cria ou substitui o conteúdo, sem acrescentar `\n`. |
| `appendSDFile` | Acrescenta os bytes informados, ou cria o arquivo. Não aplica `LOG_MAX_SIZE`. |
| `existsSDFile` | Verdadeiro só para arquivo regular. |
| `createSDDir` | Cria um nível. Diretório existente é sucesso. Arquivo no mesmo caminho falha. Os pais precisam ser criados antes. |

Texto `nullptr` é erro; string vazia é válida. Ponteiro de dados nulo só é aceito quando o tamanho é zero. Bytes zero no meio do binário são preservados. Uma escrita que retorna `false` pode ter deixado conteúdo parcial.

`appendFile(fs, path, message)` conserva o comportamento antigo de log: adiciona quebra de linha, recusa texto de até um caractere e limita o arquivo a `LOG_MAX_SIZE`. Para conteúdo geral, use `appendSDFile()`. `readFile(fs, path)` continua imprimindo na Serial.

Operações em SPIFFS ou FFat não dependem do cartão. Ausência de arquivo, diretório inválido ou falta de espaço retornam falha e não marcam o cartão como removido.

## Concorrência

Cada operação segura o mutex do cartão até fechar o arquivo. A espera pública é de 1000 ms. Se o mutex não for obtido, a chamada falha e a sessão permanece. Não há transação SPI extra em volta de `SD.begin()` nem das leituras: o driver do core controla o barramento.

Funções que seguram esse mutex não desenham e não chamam callback. Carregue imagens no `setup()`, antes da task da interface. A ordem segura, quando a UI já desenha, é desenho e depois SD.

No SPI compartilhado suportado, tela `Arduino_HWSPI`, touch XPT2046 e SD usam a mesma instância, com CS independentes e inativos antes da montagem. `Arduino_ESP32SPI` e outros backends que operam o host por conta própria pedem um SPI separado para o SD. Display RGB, DSI ou I2C não significa que um host SPI esteja livre.

Cartão removido depois da montagem produz erro na operação seguinte. A reinserção é explícita: `endSD()` e, se retornar `true`, `startSD()` outra vez.
