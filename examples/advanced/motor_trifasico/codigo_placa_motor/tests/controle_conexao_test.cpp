#include "../controle_conexao.h"
#include <assert.h>
#include <stdio.h>

static uint32_t tempo;
static uint32_t agora() { return tempo; }

struct LinkerSimulado {
    bool wifi = false;
    bool servidor = false;
    bool registrado = false;
    bool conectaWifi = false;
    bool conectaServidor = false;
    bool confirmaRegistro = false;
    bool caiDurantePoll = false;
    uint32_t duracaoBegin = 0;
    unsigned begins = 0;
    unsigned polls = 0;
    unsigned disconnects = 0;

    bool begin() {
        ++begins;
        tempo += duracaoBegin;
        wifi = conectaWifi;
        servidor = wifi && conectaServidor;
        registrado = false;
        return servidor;
    }
    bool isWiFiUp() const { return wifi; }
    bool isServerConnected() const { return servidor; }
    bool isLinkUp() const { return wifi && servidor && registrado; }
    void poll() {
        ++polls;
        if (caiDurantePoll) servidor = false;
        if (confirmaRegistro) registrado = true;
    }
    void disconnectServer() {
        ++disconnects;
        servidor = false;
        registrado = false;
    }
};

static void conectar(ControleConexaoMotor& controle, LinkerSimulado& linker) {
    linker.conectaWifi = true;
    linker.conectaServidor = true;
    linker.confirmaRegistro = true;
    controle.atualizar(linker, agora);
    assert(controle.estado() == EstadoRedeMotor::AGUARDANDO_REGISTRO);
    controle.atualizar(linker, agora);
    assert(controle.estado() == EstadoRedeMotor::ONLINE);
}

static void bootOfflineERecuperacao() {
    tempo = 2000;
    ControleConexaoMotor controle(60000, 10000);
    LinkerSimulado linker;
    linker.duracaoBegin = 15000;
    assert(controle.deveTentarConectar(tempo));
    controle.atualizar(linker, agora);
    assert(linker.begins == 1 && linker.polls == 0);
    assert(controle.estado() == EstadoRedeMotor::OFFLINE);
    const uint32_t terminou = tempo;

    // Nenhum poll/begin durante os 60 s APOS a tentativa bloqueante terminar.
    for (unsigned i = 1; i < 60000; i += 137) {
        tempo = terminou + i;
        controle.atualizar(linker, agora);
    }
    tempo = terminou + 59999;
    controle.atualizar(linker, agora);
    assert(linker.begins == 1 && linker.polls == 0);

    tempo = terminou + 60000;
    controle.atualizar(linker, agora);
    assert(linker.begins == 2 && linker.polls == 0);
    tempo += 60000;
    conectar(controle, linker);
    assert(linker.begins == 3);
}

static void wifiPresenteServidorAusente() {
    tempo = 0;
    ControleConexaoMotor controle(60000, 10000);
    LinkerSimulado linker;
    linker.conectaWifi = true;
    controle.atualizar(linker, agora);
    assert(linker.wifi && !linker.servidor);
    tempo = 59999;
    controle.atualizar(linker, agora);
    assert(linker.begins == 1 && linker.polls == 0);
    tempo = 60000;
    conectar(controle, linker);
}

static void registroSemResposta() {
    tempo = 100;
    ControleConexaoMotor controle(60000, 10000);
    LinkerSimulado linker;
    linker.conectaWifi = linker.conectaServidor = true;
    controle.atualizar(linker, agora);
    tempo += 9999;
    controle.atualizar(linker, agora);
    assert(linker.polls == 1); // poll funciona antes de isLinkUp().
    assert(controle.estado() == EstadoRedeMotor::AGUARDANDO_REGISTRO);
    ++tempo;
    controle.atualizar(linker, agora);
    assert(controle.estado() == EstadoRedeMotor::OFFLINE);
    assert(linker.disconnects == 1);
    tempo += 59999;
    controle.atualizar(linker, agora);
    assert(linker.begins == 1 && linker.polls == 2);
    ++tempo;
    conectar(controle, linker);
}

static void perdaDeConexao(bool perdeWifi, bool durantePoll) {
    tempo = 0;
    ControleConexaoMotor controle(60000, 10000);
    LinkerSimulado linker;
    conectar(controle, linker);
    for (unsigned i = 0; i < 50; ++i) {
        tempo += 1000;
        controle.atualizar(linker, agora);
    }
    assert(linker.begins == 1); // Nao reinicia a sessao online.
    if (durantePoll) linker.caiDurantePoll = true;
    else if (perdeWifi) linker.wifi = false;
    else linker.servidor = false;
    const unsigned pollsAntes = linker.polls;
    controle.atualizar(linker, agora);
    assert(controle.estado() == EstadoRedeMotor::OFFLINE);
    assert(linker.polls == pollsAntes + (durantePoll ? 1 : 0));
    linker.caiDurantePoll = false;
    const unsigned pollsOffline = linker.polls;
    tempo += 59999;
    controle.atualizar(linker, agora);
    assert(linker.polls == pollsOffline && linker.begins == 1);
    ++tempo;
    conectar(controle, linker);
    assert(linker.begins == 2);
}

static void registroPerdidoComSocketAtivo() {
    tempo = 0;
    ControleConexaoMotor controle(60000, 10000);
    LinkerSimulado linker;
    conectar(controle, linker);
    tempo = 50000;
    linker.registrado = linker.confirmaRegistro = false;
    controle.atualizar(linker, agora);
    assert(controle.estado() == EstadoRedeMotor::AGUARDANDO_REGISTRO);
    tempo += 9999;
    controle.atualizar(linker, agora);
    assert(linker.disconnects == 0);
    ++tempo;
    controle.atualizar(linker, agora);
    assert(controle.estado() == EstadoRedeMotor::OFFLINE);
}

static void rolloverDosTemporizadores() {
    tempo = UINT32_MAX - 1000;
    ControleConexaoMotor controle(60000, 10000);
    LinkerSimulado linker;
    controle.atualizar(linker, agora);
    tempo += 59999;
    controle.atualizar(linker, agora);
    assert(linker.begins == 1);
    ++tempo;
    conectar(controle, linker);

    tempo = UINT32_MAX - 1000;
    ControleConexaoMotor registro(60000, 10000);
    linker.confirmaRegistro = false;
    registro.atualizar(linker, agora);
    tempo += 9999;
    registro.atualizar(linker, agora);
    assert(registro.estado() == EstadoRedeMotor::AGUARDANDO_REGISTRO);
    ++tempo;
    registro.atualizar(linker, agora);
    assert(registro.estado() == EstadoRedeMotor::OFFLINE);
}

int main() {
    bootOfflineERecuperacao();
    wifiPresenteServidorAusente();
    registroSemResposta();
    perdaDeConexao(true, false);
    perdaDeConexao(false, false);
    perdaDeConexao(false, true);
    registroPerdidoComSocketAtivo();
    rolloverDosTemporizadores();
    puts("OK: 8 cenarios de conexao/reconexao, incluindo rollover de millis().");
}
