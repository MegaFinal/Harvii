#pragma once

#include <Arduino.h>

/// Registra as rotas, o header de autenticação e sobe o servidor na porta 80.
void servidor_iniciar();

/// Processa requisições pendentes. Chamar a cada volta do loop().
void servidor_atender();