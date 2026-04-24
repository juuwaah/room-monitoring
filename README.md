# Room Monitoring System

Sistema para mostrar el estado de presencia de el/la encargad@ en una sala usando ESP32 + teclado + LCD, visible en la web, con el fin de mejorar la accebilidad a la sala. 


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
| VO      | GND             |
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

ajustar contraste con potenciometro de 10kΩ si es necesario o insertar resistor de 10k - 20Ω entre
```
[VO - GND]
```

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

## Configuracion del ESP32

### Bibliotecas necesarias (Arduino IDE Library Manager)

- `Keypad` by Mark Stanley
- `LiquidCrystal` (incluida con ESP32, no requiere instalacion)
- `ArduinoJson` by Benoit Blanchon

### Uso del teclado

| Tecla | Accion |
|-------|--------|
| A     | Cambiar estado + modo de entrada de minutos |
| 0-9   | Ingresar minutos |
| *     | Confirmar y enviar |
| #     | Borrar ultimo digito |
| B     | Cancelar |

