# Tras compilar un entorno wokwi-*, copia el firmware a .pio/build/wokwi
# para que el simulador (wokwi.toml) arranque siempre el último robot compilado.
import os
import shutil

Import("env")


def copiar(source, target, env):
    origen = env.subst("$BUILD_DIR")
    destino = os.path.join(env.subst("$PROJECT_BUILD_DIR"), "wokwi")
    os.makedirs(destino, exist_ok=True)
    for archivo in ("firmware.bin", "firmware.elf"):
        shutil.copy(os.path.join(origen, archivo), destino)
    print("Simulador: cargado " + env.subst("$PIOENV"))


env.AddPostAction("$BUILD_DIR/firmware.bin", copiar)
