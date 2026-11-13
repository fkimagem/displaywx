#pragma once

#include <stdint.h>

enum class EstadoRedeMotor : uint8_t {
    DESATIVADA,
    OFFLINE,
    CONECTANDO,
    AGUARDANDO_REGISTRO,
    ONLINE
};

// Politica do projeto; nao modifica a reconexao interna da LinkerWXWiFi.
// O relogio e consultado novamente apos chamadas que podem bloquear.
class ControleConexaoMotor {
public:
    /**
     * @brief Guarda os prazos da politica de reconexao da placa do motor.
     * @param intervaloMs Espera minima entre tentativas, em milissegundos.
     * @param registroTimeoutMs Prazo para receber o registro depois do TLS, em milissegundos.
     */
    ControleConexaoMotor(uint32_t intervaloMs, uint32_t registroTimeoutMs)
        : intervaloMs_(intervaloMs), registroTimeoutMs_(registroTimeoutMs) {}

    /**
     * @brief Informa o estado atual da rede.
     * @return Estado da maquina de conexao.
     */
    EstadoRedeMotor estado() const { return estado_; }

    /**
     * @brief Indica se a espera offline ja permite uma nova tentativa.
     * @param agora Instante atual em milissegundos.
     * @return true na primeira tentativa ou quando o intervalo offline ja passou.
     */
    bool deveTentarConectar(uint32_t agora) const {
        return estado_ == EstadoRedeMotor::OFFLINE
            && (primeiraTentativa_ || uint32_t(agora - esperaDesde_) >= intervaloMs_);
    }

    /**
     * @brief Avanca a conexao: tenta o enlace, aguarda o registro ou detecta queda.
     * @param cliente Cliente LinkerWXWiFi usado somente nesta tarefa.
     * @param agora Funcao que devolve o instante atual em milissegundos.
     */
    template <typename Cliente, typename Relogio>
    void atualizar(Cliente& cliente, Relogio agora) {
        if (estado_ == EstadoRedeMotor::OFFLINE) {
            if (!deveTentarConectar(agora())) return;

            primeiraTentativa_ = false;
            estado_ = EstadoRedeMotor::CONECTANDO;
            if (!cliente.begin() || !cliente.isWiFiUp() || !cliente.isServerConnected()) {
                ficarOffline(cliente, agora);
                return;
            }
            registroDesde_ = agora();
            estado_ = EstadoRedeMotor::AGUARDANDO_REGISTRO;
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
            estado_ = EstadoRedeMotor::ONLINE;
        } else {
            if (estado_ == EstadoRedeMotor::ONLINE) {
                registroDesde_ = agora();
                estado_ = EstadoRedeMotor::AGUARDANDO_REGISTRO;
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
        estado_ = EstadoRedeMotor::OFFLINE;
    }

    const uint32_t intervaloMs_;
    const uint32_t registroTimeoutMs_;
    EstadoRedeMotor estado_ = EstadoRedeMotor::OFFLINE;
    bool primeiraTentativa_ = true;
    uint32_t esperaDesde_ = 0;
    uint32_t registroDesde_ = 0;
};
