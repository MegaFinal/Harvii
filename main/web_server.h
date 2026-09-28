#pragma once

#include <Arduino.h>

/// Registra as rotas, o header de autenticação e sobe o servidor na porta 80.
void servidor_iniciar();

/// Processa requisições pendentes. Chamar a cada volta do loop().
void servidor_atender();

/// Marca que a tela perdeu o gráfico (boot ou reconexão). O app consulta em /status.
void servidor_pedir_atualizacao();