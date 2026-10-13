// Lo que detectan los sensores
#pragma once

enum Evento {
  NINGUNO, TOQUE, DOBLE_TOQUE, TRIPLE_TOQUE, MUCHOS_TOQUES, CARICIA,
  GOLPE_SUAVE, GOLPE_FUERTE, SACUDIDA, LEVANTADO
};

inline const char *nombreEvento(Evento e) {
  static const char *NOMBRES[] = {
    "-", "toque", "doble toque", "triple toque", "5 toques", "caricia",
    "golpe suave", "golpe fuerte", "sacudida", "levantado"
  };
  return NOMBRES[e];
}
