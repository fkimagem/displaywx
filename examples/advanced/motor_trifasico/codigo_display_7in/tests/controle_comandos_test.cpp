#include "../controle_comandos.h"
#include <assert.h>
#include <stdio.h>

using namespace Fluxo;

static void rejeitarSemConexao() {
    FilaComandosGrupo fila;
    CmdQueueItem item;
    const char* lote[] = {"50", "START"};
    const EstadoConexao estados[] = {EstadoConexao::OFFLINE, EstadoConexao::CONECTANDO};
    for (EstadoConexao estado : estados) {
        fila.mudarEstado(estado);
        assert(!fila.enfileirar("START"));
        assert(!fila.enfileirarLote(lote, 2));
        assert(!fila.retirar(item));
        assert(fila.tamanho() == 0);
    }
    fila.mudarEstado(EstadoConexao::CONECTADO);
    assert(!fila.retirar(item)); // Nenhuma interacao offline foi guardada.
    assert(fila.enfileirar("START"));
    assert(fila.retirar(item));
    assert(strcmp(item.text, "START") == 0);
}

static void copiarTextoEFifoCircular() {
    FilaComandosGrupo fila;
    fila.mudarEstado(EstadoConexao::CONECTADO);
    char texto[32];
    for (unsigned n = 0; n < 15; ++n) {
        snprintf(texto, sizeof(texto), "%u", n);
        assert(fila.enfileirar(texto));
    }
    assert(!fila.enfileirar("EXTRA"));
    CmdQueueItem item;
    for (unsigned n = 0; n < 8; ++n) {
        assert(fila.retirar(item));
        snprintf(texto, sizeof(texto), "%u", n);
        assert(strcmp(item.text, texto) == 0);
    }
    for (unsigned n = 15; n < 23; ++n) {
        snprintf(texto, sizeof(texto), "%u", n);
        assert(fila.enfileirar(texto));
    }
    for (unsigned n = 8; n < 23; ++n) {
        assert(fila.retirar(item));
        snprintf(texto, sizeof(texto), "%u", n);
        assert(strcmp(item.text, texto) == 0);
        assert(fila.pertenceASessao(item.geracao));
    }
    assert(!fila.retirar(item));
}

static void loteIndivisivel() {
    FilaComandosGrupo fila;
    fila.mudarEstado(EstadoConexao::CONECTADO);
    for (unsigned n = 0; n < 14; ++n) assert(fila.enfileirar("STOP"));
    const char* lote[] = {"50", "START"};
    assert(!fila.enfileirarLote(lote, 2));
    assert(fila.tamanho() == 14); // Nem o setpoint entrou sozinho.
    CmdQueueItem item;
    assert(fila.retirar(item));
    assert(fila.enfileirarLote(lote, 2));
    for (unsigned n = 0; n < 13; ++n) {
        assert(fila.retirar(item));
        assert(strcmp(item.text, "STOP") == 0);
    }
    assert(fila.retirar(item) && strcmp(item.text, "50") == 0);
    assert(fila.retirar(item) && strcmp(item.text, "START") == 0);
}

static void validarTamanhoSemInsercaoParcial() {
    FilaComandosGrupo fila;
    fila.mudarEstado(EstadoConexao::CONECTADO);
    char maximo[32];
    memset(maximo, 'A', 31);
    maximo[31] = 0;
    char longo[33];
    memset(longo, 'A', 32);
    longo[32] = 0;
    assert(!fila.enfileirar(nullptr));
    assert(!fila.enfileirar(""));
    assert(!fila.enfileirar(longo));
    const char* lote[] = {"START", longo};
    assert(!fila.enfileirarLote(lote, 2));
    assert(fila.tamanho() == 0);
    assert(fila.enfileirar(maximo));
    CmdQueueItem item;
    assert(fila.retirar(item) && strcmp(item.text, maximo) == 0);
}

static void descartarSessaoAntiga(EstadoConexao perda) {
    FilaComandosGrupo fila;
    fila.mudarEstado(EstadoConexao::CONECTADO);
    const uint32_t geracao = fila.sessao().geracao;
    assert(fila.enfileirar("START"));
    assert(fila.enfileirar("STOP"));
    CmdQueueItem retirado;
    assert(fila.retirar(retirado));
    fila.mudarEstado(perda);
    assert(fila.tamanho() == 0);
    assert(!fila.pertenceASessao(retirado.geracao));
    assert(!fila.enfileirar("REVERSE"));
    fila.mudarEstado(EstadoConexao::CONECTADO);
    assert(!fila.pertenceASessao(geracao));
    CmdQueueItem item;
    assert(!fila.retirar(item));
    assert(fila.enfileirar("EMERGENCY"));
    fila.mudarEstado(EstadoConexao::CONECTADO); // Atualizacao igual preserva fila.
    assert(fila.retirar(item) && strcmp(item.text, "EMERGENCY") == 0);
    assert(fila.pertenceASessao(item.geracao));
}

static void statusVisivel() {
    assert(strcmp(textoConexao(EstadoConexao::CONECTANDO), "Conectando") == 0);
    assert(strcmp(textoConexao(EstadoConexao::CONECTADO), "Conectado") == 0);
    assert(strcmp(textoConexao(EstadoConexao::OFFLINE), "Offline") == 0);
    assert(corConexao(EstadoConexao::CONECTANDO) == 0xFFE0);
    assert(corConexao(EstadoConexao::CONECTADO) == 0x07E0);
    assert(corConexao(EstadoConexao::OFFLINE) == 0xF800);
}

int main() {
    rejeitarSemConexao();
    copiarTextoEFifoCircular();
    loteIndivisivel();
    validarTamanhoSemInsercaoParcial();
    descartarSessaoAntiga(EstadoConexao::OFFLINE);
    descartarSessaoAntiga(EstadoConexao::CONECTANDO);
    statusVisivel();
    puts("OK: 7 cenarios de fila, sessoes e indicacao visual.");
}
