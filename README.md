# Room Monitoring System

Sistema para mostrar el estado de presencia en una sala usando ESP32 + teclado + LCD, visible en la web.

## Arquitectura

```
[ESP32 + Keypad + LCD] --(WiFi/HTTP)--> [Flask en Railway] <--(Navegador)--> Visitantes
```

## Cableado

### LCD1602 (modo paralelo 4-bit)

| Pin LCD | Conexion        |
|---------|-----------------|
| VSS     | GND             |
| VDD     | 5V (VIN)        |
| VO      | GND (ajustar contraste con potenciometro de 10kΩ si es necesario) |
| RS      | GPIO19          |
| RW      | GND             |
| E       | GPIO23          |
| D0-D3   | (sin conexion)  |
| D4      | GPIO18          |
| D5      | GPIO17          |
| D6      | GPIO16          |
| D7      | GPIO15          |
| A       | 5V (VIN) - luz de fondo |
| K       | GND - luz de fondo      |

### Teclado de membrana 4x4

| Pin del teclado | ESP32 GPIO | Funcion |
|----------------|------------|---------|
| 1              | GPIO13     | Fila 1  |
| 2              | GPIO12     | Fila 2  |
| 3              | GPIO14     | Fila 3  |
| 4              | GPIO27     | Fila 4  |
| 5              | GPIO26     | Col 1   |
| 6              | GPIO25     | Col 2   |
| 7              | GPIO33     | Col 3   |
| 8              | GPIO32     | Col 4   |

Los pines del teclado van de izquierda (Pin 1) a derecha (Pin 8) visto desde el frente.

## Configuracion del ESP32

### Bibliotecas necesarias (Arduino IDE Library Manager)

- `Keypad` by Mark Stanley
- `LiquidCrystal` (incluida con ESP32, no requiere instalacion)
- `ArduinoJson` by Benoit Blanchon

### Pasos

1. Agregar la placa ESP32 en Arduino IDE (buscar `esp32` en el Board Manager)
2. Instalar las bibliotecas mencionadas
3. Abrir `esp32/room_monitor/room_monitor.ino`
4. Cambiar el SSID y contrasena del WiFi
5. Cambiar `SERVER_URL` a la URL de Railway
6. Seleccionar `ESP32 Dev Module` como placa y subir el codigo

### Uso del teclado

| Tecla | Accion |
|-------|--------|
| A     | Cambiar estado + modo de entrada de minutos |
| 0-9   | Ingresar minutos |
| *     | Confirmar y enviar |
| #     | Borrar ultimo digito |
| B     | Cancelar |

## Despliegue Web (Railway)

### Prueba local

```bash
cd web
pip install -r requirements.txt
python app.py
```

Prueba con curl:

```bash
# Enviar estado
curl -X POST http://localhost:5000/api/status \
  -H "Content-Type: application/json" \
  -d '{"status": "in_room", "minutes": 30}'

# Consultar estado
curl http://localhost:5000/api/status
```

Abrir http://localhost:5000 en el navegador para verificar.

### Despliegue en Railway

1. Iniciar sesion en [Railway](https://railway.app)
2. `New Project` → `Deploy from GitHub repo` o usar el CLI
3. Configurar Root Directory como `web`
4. Se construye y despliega automaticamente
5. Copiar la URL generada y configurarla en `SERVER_URL` del ESP32
