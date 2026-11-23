#pragma once

#include <Arduino.h>

/// Gera um identificador único de pareamento (UUID versão 4, RFC 4122).
String gerarUUIDv4();

/// Devolve o hash salvo na flash. Se não existir, gera um novo e salva.
String auth_obter_hash();