# Revisão técnica da DisplayWX — Arduino ESP32 Core 3.3.12

Data: 25/11/2026. Biblioteca: versão declarada `1.0.35`, considerando o conteúdo atual do workspace, inclusive alterações locais preexistentes. Nenhum código foi alterado por esta revisão.

**Escopo:** implementação central, widgets, armazenamento, drivers de toque, câmera/PPA, configuração, fontes, exemplos e testes existentes. Fontes geradas e tabelas de sensores foram examinadas estruturalmente; não houve validação visual dos glifos nem teste elétrico dos registradores.

**Natureza da análise:** inspeção estática e verificação de contratos existentes. Os defeitos abaixo indicam a condição que os ativa. Ganhos de velocidade e redução de memória são propostas a medir, sem percentuais estimados. Os caminhos e linhas referem-se à raiz `C:/arduino_edgewx/portable/sketchbook/libraries/displaywx` nesta revisão.

**Ambiente:** instalação local do Core `3.3.12`, baseada em ESP-IDF `5.5.5`, conforme também consta na [release oficial](https://github.com/espressif/arduino-esp32/releases/tag/3.3.12). O pacote Xtensa instalado identifica GCC `14.2.0`. Os arquivos locais `cpp_flags` examinados contêm `-std=gnu++2b` seguido de `-std=gnu++2a`: prevalece a última opção, correspondente a GNU C++20, salvo substituição posterior no comando de compilação. Também contêm `-fexceptions` e `-fno-rtti`; C usa GNU17. O modo release usa `-Os`. Registrar o comando completo ao validar uma correção; não pressupor C++11 ou exceções desativadas.

**Diretriz C/C++:** preferir funções C17, estruturas simples, capacidade explícita e buffers fornecidos pelo chamador nos algoritmos independentes de Arduino. Manter adaptadores C++ para `Print`, `FS`, displays e drivers, além de RAII e atômicos onde asseguram liberação e sincronização. C não é automaticamente mais rápido: não trocar classes por `malloc` nem substituir `std::atomic` por `volatile`. Para módulos `.c` chamados por C++, usar cabeçalhos compatíveis com ambas as linguagens e proteção `extern "C"`.

**Prioridades:** P0 = corrigir primeiro, por corrupção de memória, acesso inválido ou travamento; P1 = corrigir antes de uso contínuo, concorrente ou reconfigurável; P2 = otimização ou manutenção após estabilização. O impacto descreve a consequência no recurso afetado, não significa que todas as placas executam aquele caminho.

| ID | Melhoria | Prioridade | Nível de impacto | Problema que resolve |
|---|---|---|---|---|
| DWX-01 | Propriedade dos botões do RadioGroup | P0 | Crítico — heap | Ponteiro não inicializado e liberação duplicada |
| DWX-02 | Propriedade do SPI no XPT2046 | P0 | Crítico — heap/SPI | Destruição de barramento emprestado |
| DWX-03 | Calibração e reinicialização do toque | P0 | Crítico — heap | Liberação duplicada, vazamento e cópia com tamanho incorreto |
| DWX-04 | Limites das faixas do GaugeSuper | P0 | Crítico — memória | Acesso fora do vetor ou por ponteiro nulo |
| DWX-05 | Formatação numérica limitada | P0 | Crítico — pilha | Escrita além de buffers e overflow aritmético |
| DWX-06 | Quantidade de pontos do GSL3680 | P0 | Crítico — memória | Escrita fora da capacidade fornecida pelo chamador |
| DWX-07 | Validação de dados do GT911 | P0 | Crítico — memória | Contagem maior que `points[5]` e checksum indefinido |
| DWX-08 | Dimensões e contadores do LineChart | P0 | Crítico — execução | Laços infinitos, underflow e divisão degenerada |
| DWX-09 | Encerramento coordenado da DisplayWX | P0 | Crítico — ciclo de vida | Recursos destruídos enquanto tarefas ainda os utilizam |
| DWX-10 | Contrato de formato dos frames | P0 | Crítico — câmera | RAW tratado como RGB565, inclusive tamanho de leitura |
| DWX-11 | Encerramento seguro do timer | P1 | Alto — ciclo de vida | Callback acessando objeto já encerrado |
| DWX-12 | Exclusão mútua de desenho | P1 | Alto — concorrência | Desenho sem aquisição do mutex e liberação por outro dono |
| DWX-13 | Publicação de estado para a UI | P1 | Alto — concorrência | Texto inconsistente e solicitações de tela perdidas |
| DWX-14 | Transações do I2C compartilhado | P1 | Alto — barramento | Buffers e estado compartilhados fora da proteção |
| DWX-15 | Escopo dos guards do SD legado | P1 | Alto — armazenamento | Operações executadas após soltar a proteção do SD |
| DWX-16 | Inicialização e atualização do LineChart | P1 | Alto — memória/UI | Objeto marcado pronto após falha e atualização perdida |
| DWX-17 | Cancelamento da cópia de câmera | P1 | Alto — concorrência | Resultado tardio interferindo em nova solicitação |
| DWX-18 | Capacidade alinhada dos buffers PPA | P1 | Alto — DMA/cache | Tamanho incompatível e sincronização além da alocação |
| DWX-19 | Inicialização idempotente e falhas de alocação | P1 | Alto — heap | Fila sobrescrita e falhas de `new` sem tratamento coerente |
| DWX-20 | Logs com recuperação e flush limitado | P1 | Alto — dados/latência | Acúmulo permanente, perda de logs e bloqueio da UI |
| DWX-21 | Encerramento dos recursos de câmera | P1 | Alto — recursos | PPA/XCLK sem liberação completa e rollback incompleto |
| DWX-22 | Rollback e propriedade no GSL3680 | P1 | Alto — recursos | Vazamentos em falhas e barramento sem dono definido |
| DWX-23 | Impedir cópia rasa de objetos donos | P1 | Alto — heap | Dois objetos liberando o mesmo recurso |
| DWX-24 | Validar geometria e números especiais | P1 | Alto — execução | Laços sem término e escalas inválidas |
| DWX-25 | Validar e indexar métricas de texto | P1 | Alto — memória/CPU | Índice de substituição inválido e buscas repetidas |
| DWX-26 | Definir política de instância única | P1 | Alto — tarefas | Instâncias compartilhando contexto, pilha e TCB |
| DWX-27 | Retirar diagnósticos periódicos da UI | P1 | Alto — latência | Pausas por estatísticas de tarefas, heap e serial |
| DWX-28 | Preservar a política global de watchdog | P1 | Alto — integração | Biblioteca reconfigurando watchdog da aplicação |
| DWX-29 | Reduzir custo do snapshot do gráfico | P2 | Médio — RAM/CPU | Módulo por ponto, mutex prolongado e pequenas alocações |
| DWX-30 | Sincronizar cache por propriedade do frame | P2 | Alto — câmera | Varreduras redundantes e invalidação excessiva |
| DWX-31 | Orçamento e reutilização de buffers | P2 | Alto — RAM | Pressão sobre RAM interna e alocações grandes recorrentes |
| DWX-32 | Desenhar imagens rotacionadas por blocos | P2 | Alto — desenho | Transformação e chamadas ao display por pixel |
| DWX-33 | Formatação simples com buffers C | P2 | Médio — CPU/heap | Construção repetida de strings temporárias |
| DWX-34 | Selecionar widgets e fontes na compilação | P2 | Alto — flash/build | Catálogos amplos e recursos habilitados sem necessidade |
| DWX-35 | Limitar o trabalho executado na tarefa de UI | P2 | Alto — resposta | Callbacks lentos e operações bloqueantes no ciclo gráfico |
| DWX-36 | Fixar dependências e matriz de validação | P2 | Alto — manutenção | Builds não reproduzíveis e regressões entre backends |

## DWX-01 — Propriedade dos botões do RadioGroup

**Onde:** `src/widgets/radiobutton/wradiogroup.cpp:12`, `:38`, `:185`; `wradiogroup.h:61`.

**Problema:** `m_buttons` não recebe inicialização no construtor. Na primeira chamada de `setup()`, `cleanupMemory()` pode tentar liberar um valor indeterminado em objetos locais ou alocados dinamicamente. Depois de um setup válido, `m_config.buttons` e `m_buttons` apontam para a mesma alocação, mas o cleanup executa `delete[]` nos dois. Objetos globais podem esconder o primeiro problema pela inicialização estática; o segundo permanece.

**Correção:** inicializar o ponteiro com `nullptr`; manter um único dono da alocação; tratar `m_config.buttons` apenas como referência, sem liberá-lo separadamente. Cleanup deve ser idempotente. Rejeitar quantidade positiva com origem nula antes de copiar e marcar `m_loaded`. Alternativa C: armazenamento fixo ou buffer do chamador com capacidade explícita.

**Aceite:** criar objeto local e dinâmico, configurar duas vezes e destruir; nenhum acesso inválido, double free ou redução persistente do heap.

## DWX-02 — Propriedade do SPI no XPT2046

**Onde:** `src/touch/xpt2046/XPT2046.cpp:103`, `:177`.

**Problema:** `begin()` aceita um `SPIClass*` externo ou cria um próprio. O destrutor sempre executa `delete touchspi`. Com SPI global, local ou compartilhado com display/SD, isso libera memória que o driver não possui e pode invalidar os demais usuários.

**Correção:** registrar `ownsSpi`; encerrar/liberar somente o SPI criado internamente. O ponteiro externo deve continuar válido após o fim do toque. Definir o comportamento de `begin()` repetido, liberando apenas o recurso próprio anterior e preservando as transações dos demais dispositivos.

**Aceite:** testar SPI próprio e emprestado; após destruir/reinicializar o toque, o barramento emprestado continua funcionando e não há crescimento do heap usado.

## DWX-03 — Calibração e reinicialização do toque

**Onde:** `src/touch/touch.cpp:187`, `:1199`; métodos `startAs...` do mesmo arquivo.

**Problema:** no ramo de quatro pontos, `setCalibration()` aplica `delete[]` e depois `delete` ao mesmo ponteiro. Usa também `sizeof(array)` como quantidade, embora `array` seja ponteiro; coincidir com quatro bytes no ESP32 não torna o cálculo correto. O destrutor não libera a matriz dinâmica de calibração. Repetir inicializações de drivers também pode substituir `m_ts` sem encerrar o anterior.

**Correção:** preferir `CalibrationPoint_t matrix[CALIBRATE_POINTS]` e um indicador de validade; copiar a quantidade explícita após validar a origem. Caso o heap seja mantido, usar uma única forma de liberação, rollback e tratamento de falha coerente com `new`. Tornar a troca de driver idempotente e desarmar callbacks/interrupções antes de liberar o anterior.

**Aceite:** compilar os ramos de quatro e nove pontos; repetir calibração, limpar com entrada nula conforme contrato e reinicializar o driver sem corrupção ou vazamento.

## DWX-04 — Limites das faixas do GaugeSuper

**Onde:** `src/widgets/gauge/wgaugesuper.cpp:192`, `:257`, `:318`.

**Problema:** a condição lê `m_intervals[m_indexCurrentStrip]` antes de verificar se o índice é menor que `amountIntervals`. Depois da última faixa, pode ler além do vetor. Com zero intervalos, a checagem inicial permite prosseguir com vetores nulos, mas o desenho ainda os consulta.

**Correção:** verificar limite e ponteiro antes de indexar. Definir zero intervalos como desenho com cor neutra ou configuração inválida. Validar ordenação, quantidade e correspondência entre limites e cores. Não corrigir apenas uma das duas ocorrências.

**Aceite:** desenhar com zero, uma e várias faixas, incluindo valor máximo e último limite antes do fim da escala; nenhum acesso além dos vetores.

## DWX-05 — Formatação numérica limitada

**Onde:** `src/widgets/label/wlabel.cpp:70`; `src/widgets/gauge/wgaugesuper.cpp:113`, `:463`.

**Problema:** o Label usa `dtostrf()` em `char[20]`; limitar casas decimais não limita os dígitos inteiros de um float grande. O Gauge formata um `int` em `char[8]`, insuficiente para todo o intervalo de 32 bits. Multiplicar os limites do Gauge por dez apenas para medir texto também pode causar overflow com sinal.

**Correção:** usar `snprintf(buffer, sizeof buffer, ...)`, verificar retorno e definir política para truncamento, `NaN`, infinito e valores muito grandes: notação científica ou erro explícito. Remover a formatação do buffer do Gauge se ele continuar sem uso. Medir o limite original e acrescentar a largura de um dígito, sem multiplicar o valor.

**Aceite:** testar `INT_MIN`, `INT_MAX`, floats grandes, negativos e valores não finitos; nenhuma escrita fora do buffer e representação previsível.

## DWX-06 — Quantidade de pontos do GSL3680

**Onde:** `src/touch/gsl3680/src/esp_lcd_gsl3680.c:309`, `:436`; `gsl3680_touch.cpp:249`; implementação genérica `esp_lcd_touch.c`, função `esp_lcd_touch_get_coordinates()`.

**Problema:** `get_xy()` ignora `max_point_num`, publica `Finger_num` integral e escreve coordenadas com índice `i - 1`. O wrapper fornece apenas variáveis escalares e capacidade um. A contagem publicada faz também o tratamento genérico percorrer mais elementos que a capacidade real. A leitura I2C não verifica o erro antes de consumir `touch_data`.

**Correção:** em falha de leitura, limpar o estado e retornar erro. Limitar a contagem à capacidade do destino, ao vetor interno e ao número de pontos efetivamente decodificados. Copiar `x[i]`/`y[i]` com índices correspondentes, preencher força quando solicitada e publicar somente a contagem copiada. Manter essa implementação em C.

**Aceite:** simular zero, um, dois e contagem inválida de toques; usar sentinelas depois de buffers de capacidade um e dois; nenhuma sentinela pode mudar.

## DWX-07 — Validação de dados do GT911

**Onde:** `src/touch/gt911/TAMC_GT911.cpp:60`, `:110`, `:120`, `:154`; `TAMC_GT911.h:165`.

**Problema:** o nibble da quantidade admite até 15, mas `points` possui cinco posições. Dados inválidos do controlador podem escrever além do array. `checksum` começa sem inicialização. `isTouched` pode ficar verdadeiro sem um pacote novo válido; dimensões salvas inválidas comprometem o mapeamento.

**Correção:** inicializar o checksum com zero; rejeitar contagem maior que a capacidade antes do laço; publicar toque somente após leitura completa e status válido. Validar resolução antes de mapear. Propagar falhas de I2C sem reutilizar coordenadas como se fossem uma nova amostra.

**Aceite:** testar contagens de zero a 15, status sem dado novo, falha intermediária e resolução zero; nenhuma escrita fora do array e checksum determinístico.

## DWX-08 — Dimensões e contadores do LineChart

**Onde:** `src/widgets/linechart/wlinechart.cpp:67`, `:178`, `:181`, `:345`, `:356`.

**Problema:** o preenchimento usa índice `uint16_t` para `3 * séries * pontos`; acima de 65.535 elementos ele retorna a zero e não termina. A largura útil pode ficar negativa antes da conversão para unsigned. A grade pode calcular espaçamento zero quando há mais divisões que pixels; o laço `x += sizeVSpace` então não progride. Um contador unsigned estreito também pode voltar a zero no fim da coordenada.

**Correção:** calcular geometria em tipo assinado suficientemente largo e validar antes de converter. Exigir área útil positiva, pelo menos dois pontos e passos maiores que zero. Usar `size_t` para tamanhos/índices de buffers, multiplicação verificada e limites explícitos de memória. Desenhar a grade por quantidade limitada de divisões, evitando laço dependente de soma que pode transbordar.

**Aceite:** rejeitar gráficos menores que os rótulos, divisões excessivas e capacidades inválidas; testar o limite de 65.535 elementos sem travar.

## DWX-09 — Encerramento coordenado da DisplayWX

**Onde:** `src/displaywx.cpp:1560` em diante, funções de parada/cleanup; `:1819`, `:1903`.

**Problema:** `cleanup()` público libera recursos sem primeiro garantir a parada das tarefas. No destrutor, um timeout causa `return`, mas isso não cancela a destruição C++: os membros ainda são destruídos e a memória do objeto pode ser liberada. Tarefas que acessam `this`, filas, teclado ou câmera ficam com referências inválidas.

**Correção:** criar uma sequência única de encerramento: impedir novos pedidos; parar produtores, timer e callbacks; confirmar saída das tarefas; só então liberar recursos. Um `end()` pode retornar falha mantendo o objeto vivo. O destrutor não pode considerar um timeout como liberação segura. Evitar destruição pela própria tarefa/callback e não reutilizar pilha/TCB enquanto a tarefa anterior ainda encerra. Integrar DWX-11, DWX-21 e DWX-26.

**Aceite:** chamar cleanup/end com UI e câmera ativas, com callback lento e mais de uma vez; nenhum acesso após liberação ou tarefa remanescente usando o objeto.

## DWX-10 — Contrato de formato dos frames

**Onde:** `src/camera/cameracontroller/cameracontroller.cpp:242`; `src/displaywx.cpp:4055`, `:5386`, `:5444`, `:5662`.

**Problema:** após falhar RGB565, a câmera pode iniciar em RAW8/RAW10 e ser marcada pronta. O consumidor trata o frame como `uint16_t*`, configura PPA como RGB565 e calcula cópia com dois bytes por pixel. RAW não tem esse contrato; além das cores erradas, a cópia pode ler além do frame disponível.

**Correção:** separar captura pronta de formato renderizável. Entregar à UI apenas RGB565 validado ou executar conversão/ISP explícito antes. Transportar formato, stride e tamanho junto do frame. Verificar tamanho e overflow antes de copiar ou submeter ao PPA. Se não houver conversão, retornar erro de formato ao invés de sucesso.

**Aceite:** forçar falha de RGB565 e sucesso do fallback; a UI deve recusar ou converter corretamente, sem acesso além de `buffer.size`.

## DWX-11 — Encerramento seguro do timer

**Onde:** `src/displaywx.cpp:1624`, `:6186`, `:6217`.

**Problema:** chamadas `xTimerStop()`/`xTimerDelete()` com espera zero têm retorno ignorado. São comandos enviados à tarefa de timers; zerar o handle não prova execução nem ausência de callback em andamento. O callback recupera o objeto por timer ID e escreve no seu estado.

**Correção:** tratar falha no envio dos comandos e manter o objeto vivo até uma confirmação ordenada pela tarefa de timers, posterior aos callbacks pendentes. Implementar protocolo de parada sem bloquear a própria tarefa de timers. Publicar o evento para a UI por notificação/fila ou atômico, em vez de escrever uma flag comum concorrente.

**Aceite:** parar/reconfigurar timer sob carga e com fila de comandos ocupada; após a confirmação de encerramento, nenhum callback acessa o objeto.

## DWX-12 — Exclusão mútua de desenho

**Onde:** `src/displaywx.cpp:1978`, `:2003`, `:4074`, `:4166`, `:4187`.

**Problema:** `m_runningTransaction` é uma flag global do objeto, não prova que a tarefa atual possui o mutex. `startCustomDraw()` retorna `void` inclusive em timeout, permitindo ao chamador continuar desenhando sem proteção. `finishCustomDraw()` não valida o dono. A proteção do ciclo de UI cobre `updateWidgets()`, enquanto abertura/serviço de teclado e numpad possuem outros caminhos que desenham.

**Correção:** aquisição deve retornar sucesso e o chamador só desenhar após adquiri-la. Definir proprietário e política de reentrada; usar mutex recursivo apenas quando a mesma tarefa precisar aninhar operações. Cobrir todos os caminhos que usam o backend gráfico. Uma guarda RAII C++ é adequada aqui. Considerar que funções de troca de tela podem chamar desenho customizado enquanto a UI já segura o mutex.

**Aceite:** duas tarefas desenhando, abertura de teclado e callback que troca tela; exclusão efetiva, sem timeout seguido de desenho desprotegido e sem deadlock.

## DWX-13 — Publicação de estado para a UI

**Onde:** `src/widgets/label/wlabel.cpp`, setters e `redraw()`; `src/widgets/widgetbase.h`, estados estáticos; `src/displaywx.cpp:3964`, `:4192`.

**Problema:** mutex do display protege o periférico, mas não necessariamente dados dos widgets. Um setter em `loop()`/outra tarefa pode escrever texto enquanto a UI o lê. `WidgetBase::loadScreen` é lido, executado e depois zerado; um novo pedido entre essas etapas pode desaparecer. `volatile` não resolve exclusão nem consistência entre núcleos.

**Correção:** adotar a UI como dona do estado e receber comandos em fila limitada com payload copiado, ou proteger atualização e snapshot dos dados. Para um único pedido pendente, usar troca atômica e documentar se pedidos intermediários podem ser substituídos. O texto deve ser copiado dentro do protocolo, não apenas transferido por ponteiro temporário. Nunca manter o lock de dados durante I/O gráfico.

**Aceite:** atualizar texto e trocar telas intensamente por outra tarefa; nenhum texto parcialmente atualizado e nenhuma perda que contrarie a política documentada.

## DWX-14 — Transações do I2C compartilhado

**Onde:** `src/touch/share_i2c.cpp:22`, `:288`, `:377`; `src/camera/cameracontroller/cameracontroller.cpp`, `SharedBusLock`.

**Problema:** `beginTransmission()` altera campos compartilhados antes de obter o lock; `holdingBus` não distingue corretamente o dono. `requestFrom()` libera a proteção antes de `read()` consumir o buffer compartilhado. O reconhecimento de aquisição repetida pelo mesmo dono não mantém contagem equivalente na liberação.

**Correção:** proteger uma transação inteira, com propriedade e política de repetição explícitas. Preferir funções C como leitura de registrador para buffer do chamador, sem RX/TX global consumido em etapas desprotegidas. Se a interface estilo Wire for preservada, definir a duração do lock até o consumo ou usar buffers por contexto. Falha ao adquirir não pode alterar o estado. Não manter seção crítica de interrupções durante I2C bloqueante.

**Aceite:** câmera e toque usando o mesmo barramento, com repetição de leitura e falhas; dados não se misturam e nenhum usuário libera o lock do outro.

## DWX-15 — Escopo dos guards do SD legado

**Onde:** `src/displaywx.cpp:1048` a `:1123`, wrappers `listFiles`, `createDir`, `readFile`, `appendFile`, `writeFile` e `appendLog`.

**Problema:** o `SdGuard` local é criado dentro do `if`, mas a operação de arquivo ocorre depois do fechamento desse bloco. A proteção já terminou quando o arquivo é acessado. Outra tarefa pode desmontar ou reconfigurar o SD durante o I/O. Retornar um ponteiro de `getSD()` também não prolonga o guard.

**Correção:** executar todo o I/O e fechamento dos arquivos dentro da duração do guard, como nas APIs novas. Unificar wrappers sobre a implementação protegida, evitando mutex não recursivo aninhado. Documentar ponteiros retornados como emprestados; oferecer operação/guard explícito para quem precisa manter uma sessão.

**Aceite:** executar cada wrapper em paralelo com `endSD()`/nova montagem; desmontagem deve aguardar a operação e erros devem chegar ao chamador.

## DWX-16 — Inicialização e atualização do LineChart

**Onde:** `src/widgets/linechart/wlinechart.cpp:9`, `:35`, `:211`, `:465`.

**Problema:** `allocBuffers()` pode falhar, mas `setup()` marca `m_loaded = true` independentemente do resultado. O desenho então pode usar arrays nulos. Falha na criação do mutex também não impede inicialização. No fim do redraw, a versão é lida com lock, mas `m_shouldRedraw` é atualizada depois de soltá-lo: um `push()` intermediário pode ter seu pedido apagado.

**Correção:** retornar status de criação do mutex, alocação e `start()`; publicar estado pronto somente após sucesso integral e fazer rollback seguro. Comparar versão e atualizar a flag na mesma proteção usada pelo produtor, ou usar protocolo atômico equivalente. Todas as leituras da flag devem seguir o protocolo. Manter o desenho fora do lock de dados.

**Aceite:** injetar falha em cada alocação e em criação do mutex; o widget permanece inativo sem acesso nulo. Inserir dado no final do redraw deve provocar a próxima atualização.

## DWX-17 — Cancelamento da cópia de câmera

**Onde:** `src/displaywx.cpp:5590`, `:5601`, `:5645`.

**Problema:** após 500 ms e espera adicional, a solicitação pode zerar a fase mesmo quando o produtor ainda está copiando na fase 2. O produtor pode publicar posteriormente em `m_frameCopy` e notificar um solicitante antigo. Outra solicitação pode reutilizar o protocolo antes dessa publicação; a flag atômica isolada não protege todos os ponteiros e seus tempos de vida.

**Correção:** identificar cada solicitação por geração e separar cancelamento solicitado de produtor encerrado. O produtor mantém a propriedade do buffer até a entrega confirmada ou descarte. Não reiniciar uma solicitação enquanto a anterior escreve no estado compartilhado. Proteger o handle do solicitante e preferir notificação dedicada. Tornar a espera da UI assíncrona quando possível.

**Aceite:** atrasar deliberadamente a cópia além do timeout e solicitar outra; nenhum resultado antigo é aceito, nenhum buffer é perdido/liberado em uso e nenhuma tarefa encerrada é notificada.

## DWX-18 — Capacidade alinhada dos buffers PPA

**Onde:** `src/displaywx.cpp:5220`, `:5372`, `:5388`, `:5407`.

**Problema:** alinhar o endereço em 128 bytes não alinha automaticamente `w * h * 2`. O preview guarda e passa ao PPA o tamanho bruto. Dimensões pares não bastam: 66 × 66 RGB565 ocupa 8.712 bytes, não múltiplo de 128. A sincronização posterior arredonda a região para além do tamanho solicitado à alocação. O PPA exige alinhamento também da capacidade de saída, conforme a [documentação do IDF 5.5.5](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32p4/api-reference/peripherals/ppa.html).

**Correção:** calcular bytes lógicos com overflow verificado; obter o alinhamento requerido pelo alvo/cache; arredondar a capacidade antes de alocar. Manter `logicalBytes` e `capacityBytes` separados e fornecer a capacidade correta ao PPA. Toda região sincronizada deve caber na alocação realmente reservada. Verificar também buffers fornecidos pelo backend DSI e retornos das operações.

**Aceite:** testar dimensões como 66 × 66 e capacidades reutilizadas; nenhuma rejeição por alinhamento ou operação de cache além do buffer reservado.

## DWX-19 — Inicialização idempotente e falhas de alocação

**Onde:** `src/displaywx.cpp:1949`, `:2661`, `:6310`; alocações em `src/touch/touch.cpp` e nos widgets RadioGroup/GaugeSuper.

**Problema:** `setup()` atribui uma nova fila a `WidgetBase::xFilaLog` sem verificar se já existe uma. Uma segunda chamada perde o handle anterior; uma falha de alocação também pode substituir um handle válido por nulo. `createTask()` chama esse setup, tornando importante a política de repetição.

Há também `new` comum seguido de teste de ponteiro nulo, inclusive sob comentário que promete nothrow. Esse teste não trata `std::bad_alloc` do `new` lançador. A política de falta de memória fica diferente daquela usada em outros caminhos com `std::nothrow`.

**Correção:** criar recursos apenas quando ausentes, verificar retornos e publicar handles somente após sucesso. Em falha parcial, liberar apenas o adquirido naquela tentativa. Encerrar consumidores antes de substituir/remover filas. Onde a API promete retornar falha, usar `new (std::nothrow)` com teste e garantir que construtores não escondam outras alocações sem tratamento; alternativamente, tratar exceções explicitamente. Manter `unique_ptr` para propriedade, pois ele não define sozinho a política de falha da alocação.

**Aceite:** repetir setup/createTask e injetar falha em filas, objetos e arrays; retornar erro controlado, sem perder handles, encerrar a aplicação por exceção não tratada ou reduzir o heap a cada tentativa.

## DWX-20 — Logs com recuperação e flush limitado

**Onde:** `src/displaywx.cpp:2813`, `:2845`; `processLogQueue()` e `processLogBuffer()`.

**Problema:** a UI consome uma mensagem por passagem e grava lotes no SD de forma síncrona. Sem cartão, o buffer permanece ocupado; ao atingir a capacidade, o caminho de overflow retorna antes de tentar o flush, podendo impedir a recuperação quando o cartão volta. Poucas mensagens não atingem o lote e podem ficar indefinidamente pendentes. A gravação sem status verificável impede distinguir sucesso de perda.

**Correção:** usar fila/ring limitado, contador de descartes e política explícita quando cheio. Retentar flush mesmo com buffer cheio, por quantidade ou tempo máximo; manter dados até sucesso confirmado, conforme a política de retenção. Mover I/O para tarefa de baixa prioridade, usando o guard SD, ou oferecer serviço explícito fora da UI. Definir comportamento no encerramento e em cartão ausente, sem espera infinita.

**Aceite:** gerar logs sem SD até lotar, inserir cartão e verificar recuperação; gerar menos de um lote e aguardar prazo de flush; simular erro de escrita e medir latência da UI.

## DWX-21 — Encerramento dos recursos de câmera

**Onde:** `src/camera/cameracontroller/cameracontroller.h`; `cameracontroller.cpp:294`, `:332`.

**Problema:** o controlador registra cliente PPA e aloca XCLK sem uma rotina completa de encerramento desses handles. Se iniciar XCLK falhar depois da alocação, o handle permanece; a próxima chamada retorna sucesso apenas porque ele existe. Os membros de vídeo/captura já possuem mecanismos próprios de liberação; isso não libera automaticamente os handles adicionais do controlador.

**Correção:** implementar `end()` idempotente e destrutor coerente, chamados somente após parar tarefas usuárias. Liberar frame/captura/vídeo na ordem exigida pelas APIs, remover cliente PPA e parar/liberar XCLK. Em falha de start, desfazer os recursos adquiridos naquela etapa. Separar handle alocado de recurso efetivamente iniciado. Barramento emprestado não deve ser destruído.

**Aceite:** repetir begin/end e falhas em cada etapa; número de clientes/handles e heap retornam ao patamar inicial, e falha de XCLK não vira sucesso na tentativa seguinte.

## DWX-22 — Rollback e propriedade no GSL3680

**Onde:** `src/touch/gsl3680/src/gsl3680_touch.cpp:139`, `:192`; `gsl3680_touch.h`; `esp_lcd_gsl3680.c:168`, `:222`, `:271`; `src/touch/share_i2c.cpp:155`.

**Problema:** a inicialização cria barramento, IO e handle de toque sem encerramento simétrico no wrapper. Há saídas antecipadas depois de alocar o controlador; o caminho de erro pode liberar a estrutura e ainda atribuir seu endereço a `out_touch`. Falhas ao registrar pinos deixam barramento recém-criado sem rollback. Há retornos de inicialização ignorados e uso de abort em falhas recuperáveis.

**Correção:** começar saídas com `NULL`; verificar cada etapa; concentrar falhas em cleanup que libere somente o que foi adquirido. Publicar handles apenas no sucesso. Registrar se o barramento é próprio ou reutilizado. Implementar encerramento na ordem toque → IO → barramento próprio, após parar usuários e interrupções. Propagar erro ao chamador em vez de abortar para falhas esperadas de periférico ausente.

**Aceite:** falhar cada etapa de criação, repetir begin/end e compartilhar o I2C; não retornar ponteiro liberado, vazar handle nem destruir barramento de outro dispositivo.

## DWX-23 — Impedir cópia rasa de objetos donos

**Onde:** cabeçalhos de `RadioGroup`, `GaugeSuper`, `Image`, `LineChart` e `TouchScreen`.

**Problema:** classes que possuem ponteiros alocados, mutexes ou drivers não declaram claramente a proibição de cópia. A cópia padrão replica endereços; dois objetos podem liberar o mesmo recurso, compartilhar um mutex inesperadamente ou manter referências internas ao objeto original.

**Correção:** declarar construtor de cópia e atribuição como `= delete` nas classes donas. Proibir também movimento, salvo implementação explícita com transferência de propriedade e sem tarefas/callbacks ativos. Manter estruturas de configuração copiáveis quando apenas descrevem valores ou referências emprestadas com duração documentada. Essa proteção de C++ é útil e não precisa ser substituída por C.

**Aceite:** tentativas de cópia das classes donas falham na compilação; configurar widgets a partir de structs continua funcionando. Não exigir alteração equivalente em classes que já são não copiáveis por seus membros.

## DWX-24 — Validar geometria e números especiais

**Onde:** `src/widgets/image/wimage.cpp:816`; `src/widgets/hslider/whslider.cpp`, `updateDimensions()`; `src/widgets/verticalanalog/wverticalanalog.cpp`, cálculo de divisões; escalas de VBar e Thermometer.

**Problema:** normalizar ângulo somando/subtraindo 360 em `while` não termina para infinito e pode não progredir para float de magnitude muito grande. `NaN` passa pelas comparações. No HSlider, largura menor ou igual a dois raios produz curso inválido; raio pequeno prejudica a geometria interna. Divisões zero no indicador analógico e escalas com mínimo igual ao máximo geram cálculos degenerados.

**Correção:** rejeitar valores não finitos e usar `fmodf()` uma vez para normalizar o ângulo. Validar invariantes geométricas antes de subtrair/converter: área interna positiva, curso positivo, raio compatível, quantidade de divisões maior que zero e `min < max` para escalas. Calcular intermediários em tipo assinado largo e retornar configuração inválida sem marcar o widget pronto.

**Aceite:** testar ângulos infinitos/NaN/enormes, dimensões mínimas, raio excessivo, divisões zero e extremos iguais; retorno limitado no tempo e ausência de geometria que transborda.

## DWX-25 — Validar e indexar métricas de texto

**Onde:** `src/widgets/label/labeltextmetrics.h:24`, `:50`, `:58`, `:64`.

**Problema:** o fallback assume que o token de substituição está no índice `'?' - ' '`, sem conferir `tokenCount` nem a presença do array. Uma tabela curta ou personalizada pode gerar leitura fora dos limites. A medição procura todos os tokens, calcula seus comprimentos e percorre pares de kerning repetidamente a cada trecho do texto.

**Correção:** validar a tabela ao registrá-la e fornecer índice de substituição explícito, válido ou ausente. Tratar tabela vazia sem indexação, validar textos dos tokens e pares. Pré-calcular comprimentos e índice por primeiro byte/codepoint; ordenar/indexar pares para reduzir buscas, preservando a escolha do token mais longo. Limitar comprimento/escala e verificar overflow dos acumuladores. O algoritmo pode virar módulo C com structs e tabelas constantes.

**Aceite:** tabelas vazias/curtas, caractere ausente, UTF-8 e tokens multicaractere não causam acesso inválido. Comparar dimensões com a implementação atual em entradas válidas e medir tempo em textos longos.

## DWX-26 — Definir política de instância única

**Onde:** `src/displaywx.cpp:32`, `:41`, construtor em `:1320`; `TaskEventoTouch()` em `:4227` e `TaskCamera()` em `:4303`.

**Problema:** contexto global `instance`, pilhas e TCBs estáticos são compartilhados. Construir outro DisplayWX substitui a instância usada pelas tarefas; as funções de entrada usam esse global em vez do contexto recebido. Iniciar duas instâncias pode reutilizar armazenamento de tarefa ainda ativo. O ponteiro global também precisa perder validade no encerramento.

**Correção:** escolher e documentar um contrato. Para singleton, impedir uma segunda instância ativa e limpar o registro somente após encerrar a proprietária. Para múltiplas instâncias, passar `this` em `pvParameters` e mover todos os recursos e estados necessários para cada instância, inclusive os estáticos de WidgetBase. Não corrigir apenas o ponteiro ignorando pilhas/TCBs compartilhados.

**Aceite:** segunda instância é recusada de forma previsível ou funciona com recursos independentes; destruir a primeira e criar outra não reutiliza tarefa ainda ativa.

## DWX-27 — Retirar diagnósticos periódicos da UI

**Onde:** `src/displaywx.cpp:4260`, chamadas a `logMemoriaSistema()` e `printRunningTasks()`; Core local `cores/esp32/freertos_stats.h`.

**Problema:** a cada cinco segundos a tarefa gráfica imprime estatísticas de heap/tarefas, mesmo sem um modo de diagnóstico explícito. A função do Core informa que bloqueia temporariamente interrupções e escalonador; sua implementação também aloca memória auxiliar. Serial e consultas de heap acrescentam variação de latência. Não se trata de vazamento confirmado dessa função.

**Correção:** condicionar as chamadas a uma opção de diagnóstico desativada por padrão. Coletar somente métricas necessárias sob solicitação ou em tarefa de baixa prioridade, com frequência controlada. Evitar estatística global de tarefas no ciclo de desenho. A documentação do [FreeRTOS no IDF](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32/api-reference/system/freertos_idf.html) deve orientar as restrições dessas consultas.

**Aceite:** build normal não chama nem imprime essas estatísticas periodicamente; comparar maior duração e percentis do ciclo gráfico com diagnóstico desligado/ligado.

## DWX-28 — Preservar a política global de watchdog

**Onde:** `src/displaywx.cpp:108`, `:2752`, função `changeWTD()`.

**Problema:** criar a tarefa pode reconfigurar o task watchdog global, alterando máscara de tarefas idle e política de panic. Isso muda a supervisão de componentes externos à biblioteca, inclusive quando o parâmetro representa desabilitar o uso local.

**Correção:** deixar a configuração global sob responsabilidade da aplicação. Se oferecida, a reconfiguração deve ser uma operação separada, explicitamente solicitada. A biblioteca pode registrar/alimentar sua própria tarefa quando habilitado, respeitando um watchdog já inicializado e tratando erros. Não aumentar timeout como correção de laços travados ou I/O longo.

**Aceite:** iniciar/parar DisplayWX não altera a política global configurada pelo sketch; funcionamento correto com watchdog já ativo, não inicializado e sem registro da tarefa da UI.

## DWX-29 — Reduzir custo do snapshot do gráfico

**Onde:** `src/widgets/linechart/wlinechart.cpp:35`, `:261`, `:461`.

**Problema:** o snapshot percorre cada ponto do ring com operação de módulo dentro do mutex. Existem três alocações pequenas para arrays de ponteiros além do pool. O pool mantém ring, snapshot e último desenho: com `int` de quatro bytes, ocupa `12 * séries * pontos` bytes, ou 120.000 bytes para dez séries de mil pontos. Isso é consumo previsto, não vazamento.

**Correção:** copiar cada série circular em até dois `memcpy`, separando o trecho até o fim e o trecho desde o início. Usar arrays fixos de ponteiros limitados por `MAX_SERIES` ou calcular offsets. Avaliar guardar apenas coordenadas do último desenho em tipo menor, caso sua faixa seja garantida; preservar precisão dos dados originais. Manter o snapshot coerente e o I/O fora do mutex.

**Aceite:** equivalência para ring vazio, parcial, cheio e após wrap; medir tempo de lock e RAM por capacidade. Implementação de ring/snapshot pode ser C puro, com capacidade e strides explícitos.

## DWX-30 — Sincronizar cache por propriedade do frame

**Onde:** `src/displaywx.cpp:5133`, `:5375`, `:5411`, `:5417`, `:5674`.

**Problema:** há sincronização do frame de origem por desenho de widget e invalidação ampla do framebuffer de destino. No preview SPI aparecem M2C e C2M consecutivos. Parte disso pode ser redundante; invalidar uma região que contém alterações recentes da CPU também exige cuidado. O efeito exato depende dos contratos dos drivers CSI, PPA e display e precisa ser validado no P4.

**Correção:** documentar quem escreve e quem lê cada buffer. CPU → DMA pode exigir C2M; DMA → CPU pode exigir M2C. Sincronizar a origem uma vez por frame quando o contrato permitir, e evitar invalidar áreas com desenho da CPU ainda não publicado. Usar regiões alinhadas pertencentes ao buffer ou staging separado para preservar pixels vizinhos. Seguir as regras de [coerência de cache do IDF](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32p4/api-reference/system/mm_sync.html); não remover chamadas apenas porque parecem duplicadas.

**Aceite:** verificar múltiplas câmeras/widgets, overlay e atualizações simultâneas; imagem sem artefatos e menor tempo de sincronização medido, com DWX-18 já resolvido.

## DWX-31 — Orçamento e reutilização de buffers

**Onde:** `src/widgets/image/wimage.cpp:169`, `:221`; `src/displaywx.cpp:5220`, `:5590`, `:5662`.

**Problema:** imagens são expandidas integralmente em RAM, preview tenta primeiro RAM interna com capacidade DMA, e a cópia de frame é alocada/liberada por solicitação. Isso disputa memória contígua com tarefas e periféricos. Em RGB565, 320 × 240 já exige 153.600 bytes de pixels, além de máscara e estruturas. O carregador ainda zera buffers que serão preenchidos integralmente antes da publicação.

**Correção:** definir orçamento por recurso, validar dimensões antes da alocação e escolher memória segundo consumidor e alvo. Preferir PSRAM para assets grandes quando o backend permitir; usar staging interno pequeno quando necessário. Reutilizar capacidade de preview/cópia, com limite e liberação explícita no encerramento. Remover o memset somente após provar preenchimento de todos os bytes relevantes, inclusive padding da máscara. Preservar a carga transacional e verificações de formato já presentes.

**Aceite:** ciclos de carga/troca/ROI com e sem PSRAM, incluindo falha por falta de memória. Medir heap livre, mínimo e maior bloco por capacidade; heap total sozinho não detecta fragmentação. Respeitar as capacidades descritas no [alocador do IDF](https://docs.espressif.com/projects/esp-idf/en/v5.5.5/esp32/api-reference/system/mem_alloc.html), sem supor suporte DMA idêntico em ESP32, S3 e P4.

## DWX-32 — Desenhar imagens rotacionadas por blocos

**Onde:** `src/widgets/image/wimage.cpp:720`, especialmente o laço que chama `drawPixel()`.

**Problema:** a rotação calcula coordenadas e valida limites por pixel, chamando o backend individualmente. Em display SPI, o custo de chamadas/transações pode superar o custo matemático. A disponibilidade de framebuffer ou DMA varia entre backends.

**Correção:** calcular seno/cosseno e limites uma vez; recortar a região visível antes do laço. Usar transformação incremental por linha e buffers de linha/spans para transferências agrupadas, preservando transparência e semântica da máscara. Criar caminhos simples para 0/90/180/270 graus. No P4, considerar PPA apenas para formatos e transformações suportados. Manter um fallback por pixel quando necessário.

**Aceite:** comparar imagens de referência para ângulos, recorte e transparência; medir pixels por segundo e quantidade de chamadas ao backend. O núcleo matemático e o rasterizador de linha são bons candidatos a C.

## DWX-33 — Formatação simples com buffers C

**Onde:** `src/widgets/spinbox/wspinbox.cpp:153`, `:273`; `src/widgets/gauge/wgaugesuper.cpp:345`; `src/widgets/verticalanalog/wverticalanalog.cpp`, formatação numérica de desenho.

**Problema:** expressões `String(valor)` são construídas repetidamente para imprimir, contar caracteres ou escolher fontes. Elas repetem conversão e trabalho de objetos temporários. Não é correto afirmar que toda String curta aloca heap: a implementação do Core pode otimizar esses casos.

**Correção:** formatar uma vez com `snprintf` em buffer local suficiente e reutilizar conteúdo/comprimento naquele desenho. Para inteiro de 32 bits com sinal, reservar ao menos 12 bytes contando o terminador. Usar buffer do chamador em funções reutilizáveis e impedir que ponteiros locais escapem. Manter overloads públicos C++ por compatibilidade, delegando ao núcleo C quando útil. Não introduzir buffer estático compartilhado entre tarefas.

**Aceite:** texto e alinhamento permanecem iguais nos extremos; medir tempo e alocações reais antes/depois. Aplicar após DWX-05 para não trocar conveniência por overflow.

## DWX-34 — Selecionar widgets e fontes na compilação

**Onde:** `src/widgets/widgetsetup.h:74`, `:322`; `src/widgets/widgetbase.h:34`; `src/widgets/widgetbase.cpp:412`; `src/fonts/`.

**Problema:** a configuração gráfica habilita um conjunto amplo de widgets e famílias de fontes. Cabeçalhos distribuem grandes definições; a seleção automática RobotoBold referencia muitos tamanhos, mantendo esses candidatos alcançáveis no binário quando usada. A inclusão de um header, isoladamente, não prova que todos os bytes chegarão ao firmware: confirmar pelo mapa de link.

**Correção:** oferecer seleção explícita de recursos e catálogo de fontes por projeto. Centralizar definições grandes em uma unidade de compilação com declarações externas quando apropriado. Permitir uma lista menor de fontes para seleção automática. Aplicar a configuração tanto aos headers quanto aos `.cpp`; definir macro apenas no `.ino` não configura automaticamente as unidades da biblioteca. Como há testes com `defined()`, definir uma macro como zero não equivale a desabilitá-la.

**Aceite:** compilar configuração mínima e completa, comparar `.map`, flash, RAM e tempo de build; verificar que a fonte escolhida e widgets habilitados continuam corretos. Não mover tabelas constantes da flash para RAM para reduzir includes.

## DWX-35 — Limitar o trabalho executado na tarefa de UI

**Onde:** `src/displaywx.cpp`, despacho de callbacks em `displaywxTask()`, serviços de entrada e cópia de câmera; exemplos com callbacks de usuário.

**Problema:** limitar a quantidade de callbacks por ciclo não limita o tempo de um callback. Um callback com delay, rede, SD ou cálculo longo bloqueia toque e desenho, aumenta backlog e interfere no encerramento. A espera síncrona por frame também pode consumir centenas de milissegundos da UI.

**Correção:** documentar callbacks como curtos e sem I/O bloqueante; encaminhar trabalho demorado a outra tarefa por comando com dados copiados. Medir duração e limitar o despacho por orçamento de tempo entre callbacks. Converter espera de câmera em estado pendente com conclusão posterior. Não interromper arbitrariamente callbacks em execução, nem aumentar prioridade da UI para mascarar bloqueio.

**Aceite:** exemplos de operação demorada permanecem responsivos; registrar latência toque → desenho e maior duração do ciclo sob carga, com critérios definidos pela aplicação.

## DWX-36 — Fixar dependências e matriz de validação

**Onde:** `library.properties`; `README.md`; `src/extras/check_version.h`; `tests/display_backend_contract_test.py`; `tests/sd_contract_test.cpp`; exemplos por placa/backend.

**Problema:** metadados ainda possuem descrição provisória e não expressam claramente dependências. O README cita Arduino_GFX 1.5.9, enquanto a instalação examinada usa 1.6.5. Mensagens de versão precisam refletir o contrato real; uma checagem de macros não demonstra que todas as combinações compilaram. O teste existente de backend verifica contratos estáticos, não tipos C++, link ou execução.

**Correção:** documentar versões realmente validadas e dependências obrigatórias/opcionais, usando o formato de [bibliotecas Arduino](https://docs.arduino.cc/arduino-cli/library-specification/). Fixar Core 3.3.12 e versões das bibliotecas na validação; registrar FQBN, opções de PSRAM/flash, flags e saída de tamanho. Atualizar mensagens e exemplos segundo a mesma configuração. Não declarar dependência opcional como obrigatória sem decidir o custo para usuários de outros backends.

**Aceite:** instalação limpa consegue compilar exemplos mínimos nas combinações oficialmente suportadas. Automatizar os contratos estáticos existentes e builds representativos; executar os testes de memória/concorrência dos itens P0/P1 antes de comparar performance.

## Validação executada e limites

- Foi executado o teste existente `python -B tests/display_backend_contract_test.py`: aprovado para 50 configurações, verificando 9.844 chamadas ativas aos backends quanto a nomes e quantidade de argumentos. Isso não equivale a compilar C++.
- Não houve alteração de código, criação de testes, upload de firmware nem teste em placa. Os executáveis locais de cross-compilação não iniciaram devido a erro de DLL/relocação do Windows; a compatibilidade de compilação completa permanece pendente. Essa falha do ambiente não foi atribuída à biblioteca.
- A presença de alocação não foi classificada automaticamente como vazamento. Separaram-se liberações ausentes/duplicadas, pressão de memória, fragmentação potencial e custo de execução.
- Há práticas úteis a preservar: validação de layout binário das imagens, carga com publicação após sucesso, APIs novas do SD com guard, pool contíguo e snapshot no LineChart, além de atômicos já usados em parte da UI. As correções devem completar esses contratos.

## Sequência de implementação e medição

1. Resolver DWX-01 a DWX-10 e os contratos de parada, propriedade e sincronização que eles exigem. Criar testes de regressão pequenos para os casos de erro indicados, sem depender de tela para testar funções puras.
2. Resolver os demais P1; injetar falhas em alocações/I2C/SD e atrasos na câmera. Nos módulos portáveis C/C++, usar sanitizadores no host quando disponíveis; isso complementa, mas não substitui, a placa.
3. Compilar ao menos ESP32 e ESP32-S3 com display SPI, um backend monocromático suportado e ESP32-P4 com DSI/câmera/PPA. Cobrir configurações sem toque, sem SD, sem PSRAM e as variantes de toque afetadas. Validar somente combinações realmente suportadas pelo hardware.
4. Medir uma linha de base antes dos P2: flash/RAM do link, heap livre/mínimo/maior bloco, maior uso de pilha, duração mediana/p95/máxima do ciclo gráfico, latência de toque e FPS com resolução e barramento fixos. Repetir a mesma carga após cada otimização.
5. Fazer teste prolongado de troca de telas, carga de imagens, calibração, logs, câmera e begin/end. Queda persistente após retornar ao mesmo estado indica investigação de vazamento; queda do maior bloco com total semelhante sugere fragmentação. Não atribuir ganho ao uso de C sem medir o caminho alterado.

**Orientação para execução por outro agente:** tratar cada ID como uma tarefa com escopo e aceite próprios; resolver dependências entre ciclo de vida, locks e buffers antes de otimizações. Preservar as alterações preexistentes do workspace. Registrar versões e resultados; não marcar como resolvido apenas porque a compilação passou.
