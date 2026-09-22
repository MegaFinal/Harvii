#pragma once

#include <Arduino.h>

/// Configura o pino do botão. Chamar no setup().
void reset_iniciar();

/// Pede um reset para ser feito no próximo loop() (usado pela rota /reset).
void reset_agendar();

/// Chamar no loop(): confere o botão e executa um reset pendente.
void reset_loop();