#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace Fluxo {

enum class EstadoConexao : uint8_t { OFFLINE, CONECTANDO, CONECTADO };

struct EstadoSessao {
    EstadoConexao estado;
    uint32_t geracao;
};

struct CmdQueueItem {
    char text[32];
    uint32_t geracao;
};

// Fila fixa, sem alocacao. O projeto protege TODAS as chamadas com o mesmo
// portMUX: verificar o estado e enfileirar e uma unica operacao atomica.
// Nenhuma operacao de rede, desenho ou log ocorre com esse lock adquirido.
class FilaComandosGrupo {
public:
    static constexpr size_t CAPACIDADE = 15;

    /**
     * @brief Copia o estado e a geracao atuais da sessao.
     * @return Sessao observada no momento da chamada.
     */
    EstadoSessao sessao() const { return {estado_, geracao_}; }
    /**
     * @brief Informa quantos comandos ainda estao na fila.
     * @return Quantidade de itens pendentes.
     */
    size_t tamanho() const { return quantidade_; }

    /**
     * @brief Troca o estado da conexao e descarta a fila quando o estado muda.
     * @param novo Novo estado da sessao.
     */
    void mudarEstado(EstadoConexao novo) {
        if (novo == estado_) return;
        estado_ = novo;
        ++geracao_;
        inicio_ = quantidade_ = 0; // Nunca transportar comandos entre sessoes.
    }

    /**
     * @brief Enfileira um comando da sessao conectada.
     * @param texto Comando terminado em zero e menor que o campo da fila.
     * @return true quando o comando foi copiado para a fila.
     */
    bool enfileirar(const char* texto) {
        const char* textos[] = {texto};
        return enfileirarLote(textos, 1);
    }

    /**
     * @brief Enfileira varios comandos como um unico lote da sessao atual.
     * @param textos Ponteiros para os textos do lote.
     * @param total Quantidade de textos em textos.
     * @return true quando todos os itens couberam e foram copiados.
     */
    bool enfileirarLote(const char* const* textos, size_t total) {
        if (estado_ != EstadoConexao::CONECTADO || !textos || total == 0
            || total > CAPACIDADE - quantidade_) return false;
        for (size_t i = 0; i < total; ++i) {
            if (!textos[i] || !textos[i][0] || strlen(textos[i]) >= sizeof(CmdQueueItem::text)) return false;
        }
        for (size_t i = 0; i < total; ++i) {
            CmdQueueItem& item = itens_[(inicio_ + quantidade_) % CAPACIDADE];
            memcpy(item.text, textos[i], strlen(textos[i]) + 1);
            item.geracao = geracao_;
            ++quantidade_;
        }
        return true;
    }

    /**
     * @brief Remove o comando mais antigo da fila.
     * @param item Recebe o item retirado.
     * @return true quando havia um item na sessao conectada.
     */
    bool retirar(CmdQueueItem& item) {
        if (estado_ != EstadoConexao::CONECTADO || quantidade_ == 0) return false;
        item = itens_[inicio_];
        inicio_ = (inicio_ + 1) % CAPACIDADE;
        --quantidade_;
        return true;
    }

    /**
     * @brief Confere se a geracao ainda pertence a sessao conectada.
     * @param geracao Geracao capturada quando o comando foi criado.
     * @return true quando a sessao continua conectada com essa geracao.
     */
    bool pertenceASessao(uint32_t geracao) const {
        return estado_ == EstadoConexao::CONECTADO && geracao == geracao_;
    }

private:
    EstadoConexao estado_ = EstadoConexao::OFFLINE;
    uint32_t geracao_ = 0;
    CmdQueueItem itens_[CAPACIDADE] = {};
    size_t inicio_ = 0;
    size_t quantidade_ = 0;
};

/**
 * @brief Traduz o estado da conexao para o texto mostrado no display.
 * @param estado Estado atual da sessao LinkerWX.
 * @return Conectando, Conectado ou Offline.
 */
inline const char* textoConexao(EstadoConexao estado) {
    switch (estado) {
        case EstadoConexao::CONECTANDO: return "Conectando";
        case EstadoConexao::CONECTADO: return "Conectado";
        default: return "Offline";
    }
}

/**
 * @brief Escolhe a cor RGB565 do LED conforme o estado da conexao.
 * @param estado Estado atual da sessao LinkerWX.
 * @return Amarelo enquanto conecta, verde quando conectado e vermelho quando offline.
 */
inline uint16_t corConexao(EstadoConexao estado) {
    switch (estado) {
        case EstadoConexao::CONECTANDO: return 0xFFE0; // Amarelo RGB565.
        case EstadoConexao::CONECTADO: return 0x07E0; // Verde RGB565.
        default: return 0xF800; // Vermelho RGB565.
    }
}

} // namespace Fluxo
