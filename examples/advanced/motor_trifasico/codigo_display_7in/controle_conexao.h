#pragma once

#include <stdint.h>

enum class EstadoTransporte : uint8_t {
    DESATIVADA,
    OFFLINE,
    CONECTANDO,
    AGUARDANDO_REGISTRO,
    ONLINE
};

// Politica do projeto; nao modifica a reconexao interna da LinkerWXWiFi.
// O relogio e consultado novamente apos chamadas que podem bloquear.
class ControleConexaoDisplay {
public:
    /**
     * @brief Guarda os prazos da politica de reconexao do display.
     * @param intervaloMs Espera minima entre tentativas, em milissegundos.
     * @param registroTimeoutMs Prazo para receber o registro depois do TLS, em milissegundos.
     */
    ControleConexaoDisplay(uint32_t intervaloMs, uint32_t registroTimeoutMs)
        : intervaloMs_(intervaloMs), registroTimeoutMs_(registroTimeoutMs) {}

    /**
     * @brief Encerra o servidor, volta para offline e libera uma nova tentativa imediata.
     * @param cliente Cliente LinkerWXWiFi usado pela tarefa de rede.
     * @param agora Funcao que devolve o instante atual em milissegundos.
     */
    template <typename Cliente, typename Relogio>
    void desconectar(Cliente& cliente, Relogio agora) { ficarOffline(cliente, agora); primeiraTentativa_ = false; }

    /**
     * @brief Informa o estado atual do transporte.
     * @return Estado da maquina de conexao.
     */
    EstadoTransporte estado() const { return estado_; }

    /**
     * @brief Indica se a espera offline ja permite uma nova tentativa.
     * @param agora Instante atual em milissegundos.
     * @return true na primeira tentativa ou quando o intervalo offline ja passou.
     */
    bool deveTentarConectar(uint32_t agora) const {
        return estado_ == EstadoTransporte::OFFLINE
            && (primeiraTentativa_ || uint32_t(agora - esperaDesde_) >= intervaloMs_);
    }

    /**
     * @brief Avanca a conexao: tenta o enlace, aguarda o registro ou detecta queda.
     * @param cliente Cliente LinkerWXWiFi usado somente nesta tarefa.
     * @param agora Funcao que devolve o instante atual em milissegundos.
     */
    template <typename Cliente, typename Relogio>
    void atualizar(Cliente& cliente, Relogio agora) {
        if (estado_ == EstadoTransporte::OFFLINE) {
            if (!deveTentarConectar(agora())) return;

            primeiraTentativa_ = false;
            estado_ = EstadoTransporte::CONECTANDO;
            if (!cliente.begin() || !cliente.isWiFiUp() || !cliente.isServerConnected()) {
                ficarOffline(cliente, agora);
                return;
            }
            registroDesde_ = agora();
            estado_ = EstadoTransporte::AGUARDANDO_REGISTRO;
            return;
        }

        // Nao chamar poll() durante a espera offline: ele reconecta o WiFi
        // imediatamente. Uma queda dentro do proprio poll() ainda pode iniciar
        // uma tentativa interna da biblioteca, isolada na tarefa de rede.
        if (!cliente.isWiFiUp() || !cliente.isServerConnected()) {
            ficarOffline(cliente, agora);
            return;
        }

        cliente.poll(); // Tambem necessario para receber HI/AGAIN antes de ONLINE.

        if (!cliente.isWiFiUp() || !cliente.isServerConnected()) {
            ficarOffline(cliente, agora);
        } else if (cliente.isLinkUp()) {
            estado_ = EstadoTransporte::ONLINE;
        } else {
            if (estado_ == EstadoTransporte::ONLINE) {
                registroDesde_ = agora();
                estado_ = EstadoTransporte::AGUARDANDO_REGISTRO;
            }
            if (uint32_t(agora() - registroDesde_) >= registroTimeoutMs_) {
                ficarOffline(cliente, agora);
            }
        }
    }

private:
    /**
     * @brief Desconecta o servidor e marca o inicio da espera offline.
     * @param cliente Cliente cuja sessao sera encerrada.
     * @param agora Funcao que devolve o instante atual em milissegundos.
     */
    template <typename Cliente, typename Relogio>
    void ficarOffline(Cliente& cliente, Relogio agora) {
        cliente.disconnectServer();
        esperaDesde_ = agora();
        estado_ = EstadoTransporte::OFFLINE;
    }

    const uint32_t intervaloMs_;
    const uint32_t registroTimeoutMs_;
    EstadoTransporte estado_ = EstadoTransporte::OFFLINE;
    bool primeiraTentativa_ = true;
    uint32_t esperaDesde_ = 0;
    uint32_t registroDesde_ = 0;
};
