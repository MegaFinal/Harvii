#pragma once

#include <Arduino.h>

/// Decide o modo de operação no boot: conecta se já houver credenciais,
/// senão levanta o access point de configuração.
void wifi_iniciar();

/// Guarda as credenciais recebidas e tenta conectar na rede.
/// Se falhar, volta sozinho para o access point.
void wifi_conectar(const String& ssid, const String& senha);

/// Levanta o access point de configuração e mostra os dados no display.
void wifi_iniciar_access_point();

/// Guarda as credenciais para conectar depois, fora do handler web.
void wifi_agendar_conexao(const String& ssid, const String& senha);

/// Chamar no loop(): executa a conexão pendente, se houver.
void wifi_loop();