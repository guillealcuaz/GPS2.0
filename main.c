
/* ###################################################################
**     Filename    : main.c
**     Project     : PARSEADOR
**     Processor   : MK64FN1M0VLL12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2024-01-12, 11:12, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "Pins1.h"
#include "ASerialLdd1.h"
#include "AS2.h"
#include "ASerialLdd2.h"
/* Including shared modules, which are used for whole project */
#include "AS1.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PDD_Includes.h"
#include "Init_Config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#define LONG_MAX_CADENA 10000
#define SIZE 64
#define LONG_MAX_FECHA 64


typedef struct {
    char latitudStr[SIZE];
    char longitudStr[SIZE];
    double latitud;
    double longitud;
    char tiempo[20];
    char direccionLatitud;
    char direccionLongitud;
    char estado;
    double velocidad;
    double direccion;
    double altitud;
    char fecha[LONG_MAX_FECHA];
} GPSData;


/* User includes (#include below this line is not maintained by Processor Expert) */
double convertirADecimal(const char* gradosMinutosStr, char direccion) {
    char gradosStr[4] = { 0 };
    char minutosStr[10] = { 0 };

    // primeros dos dígitos son los grados para la latitud y los primeros tres para la longitud
    int gradosCount = (direccion == 'N' || direccion == 'S') ? 2 : 3;
    strncpy(gradosStr, gradosMinutosStr, gradosCount);
    strncpy(minutosStr, gradosMinutosStr + gradosCount, 9);

    double grados = atof(gradosStr);
    double minutos = atof(minutosStr);

    double decimal = grados + (minutos / 60.0);

    if (direccion == 'S' || direccion == 'W') {
        decimal = -decimal;
    }

    return decimal;
}

char extraerDato(GPSData* datos, char** token, void (*extractor)(GPSData*, const char*)) {
    *token = strtok(NULL, ","); //coge el siguiente token
    if (!*token) {
        fprintf(stderr, "Error al extraer un dato.\n");
        return 'E';
    }
    extractor(datos, *token); //llamar a extractor con token actual
    return 'P'; // Procesado
    printf("Token extraído: %s\n", token);
}

void extraerTiempo(GPSData* datos, const char* token) {
    strncpy(datos->tiempo, token, sizeof(datos->tiempo) - 1); //copiar tiempo en estructura
    datos->tiempo[sizeof(datos->tiempo) - 1] = '\0';
}

void extraerEstado(GPSData* datos, const char* token) {
    datos->estado = token[0];
}

void extraerLongitud(GPSData* datos, const char* token) {
    strncpy(datos->longitudStr, token, SIZE - 1);
    datos->direccionLongitud = *(strtok(NULL, ","));
    datos->longitud = convertirADecimal(datos->longitudStr, datos->direccionLongitud);
}

void extraerLatitud(GPSData* datos, const char* token) {
    // Copia la cadena de latitud al struct
    strncpy(datos->latitudStr, token, SIZE - 1);
    //dirección de la latitud, siguiente en el NMEA
    datos->direccionLatitud = *(strtok(NULL, ","));
    //latitud a un valor decimal
    datos->latitud = convertirADecimal(datos->latitudStr, datos->direccionLatitud);
}

void extraerVelocidad(GPSData* datos, const char* token) {
    sscanf(token, "%lf", &datos->velocidad);
    datos->velocidad = datos->velocidad * 1.15078;
}

void extraerDireccion(GPSData* datos, const char* token) {
    sscanf(token, "%lf", &datos->direccion);
}


void formatearTiempoISO(char* destino, const char* tiempoNmea) {
    // tiempo al formato HH:MM:SS+00
    int horas, minutos, segundos;
    sscanf(tiempoNmea, "%2d%2d%2d", &horas, &minutos, &segundos);
    snprintf(destino, 25, "%02d:%02d:%02d+00", horas, minutos, segundos);
}

void extraerFecha(GPSData* datos, const char* token) {
    strncpy(datos->fecha, token, sizeof(datos->fecha) - 1);
    datos->fecha[sizeof(datos->fecha) - 1] = '\0';
}
char parseNMEASentence(const char* cadenaNmea, GPSData* datos) {
    memset(datos, 0, sizeof(GPSData)); // Limpia los datos anteriores
    char* cadenaNmeaCopia = strdup(cadenaNmea);
    if (!cadenaNmeaCopia) {
        perror("Error al duplicar la cadena NMEA");
        return 'E';
    }
    char* token = strtok(cadenaNmeaCopia, ",");
    if (!token) {
        fprintf(stderr, "Error: Sentencia NMEA vacía.\n");
        free(cadenaNmeaCopia);
        return 'I'; // Ignorado
    }

    if (strncmp(token, "$GPRMC", 6) != 0) {
        printf("Ignorando sentencia no GPRMC: %s\n", token); // Mensaje de depuración
        free(cadenaNmeaCopia);
        return 'I'; // Ignorado
    }

    if (extraerDato(datos, &token, extraerTiempo) == 'E' ||
        extraerDato(datos, &token, extraerEstado) == 'E' ||
        extraerDato(datos, &token, extraerLatitud) == 'E' ||
        extraerDato(datos, &token, extraerLongitud) == 'E' ||
        extraerDato(datos, &token, extraerVelocidad) == 'E' ||
        extraerDato(datos, &token, extraerDireccion) == 'E' ||
        extraerDato(datos, &token, extraerFecha) == 'E') {
        free(cadenaNmeaCopia);
        return 'E'; // Error al procesar un campo específico
    }

    free(cadenaNmeaCopia);
    return 'P'; // Procesado correctamente
}


void SendString(const char* str) {
    size_t len = strlen(str);  // Calcula la longitud de la cadena
    unsigned int sent = 0;     // Variable para almacenar el número de bytes enviados

    if (len > 0) {
        byte err = AS1_SendBlock(str, len, &sent);  // Envía el bloque de datos
        if (err != ERR_OK) {
            // Manejo de errores
            fprintf(stderr, "Error al enviar datos: %d\n", err);
        } else {
            printf("Bytes enviados: %u\n", sent);
        }
    }
}


size_t escribeJSON(const GPSData* data, char* buffer, size_t buffer_size, int contadorName) {
    // Convertir datos numéricos a cadenas
    if (isnan(data->latitud) || isnan(data->longitud) || isnan(data->velocidad) || isnan(data->direccion)) {
        fprintf(stderr, "Error: Datos numéricos inválidos en GPSData\n");
        return 0;
    }

    char latStr[20] = {0}, lonStr[20] = {0}, speedStr[20] = {0}, dirStr[20] = {0};
    snprintf(latStr, sizeof(latStr), "%.6f", data->latitud);
    snprintf(lonStr, sizeof(lonStr), "%.6f", data->longitud);
    snprintf(speedStr, sizeof(speedStr), "%.1f mph", data->velocidad * 1.15078); // Conversión de nudos a mph
    snprintf(dirStr, sizeof(dirStr), "%.2f degrees", data->direccion);

    char tiempoISO[25];
    formatearTiempoISO(tiempoISO, data->tiempo); // Asegúrate de que esta función funcione correctamente

    // Asegurarse de que el buffer es suficientemente grande para la salida esperada
    if (buffer_size < LONG_MAX_CADENA) {
        fprintf(stderr, "Error: buffer insuficiente para JSON\n");
        return 0;
    }

    // Formatear la cadena JSON
    int escritos = snprintf(buffer, buffer_size,
        "{ \"type\": \"Feature\", \"properties\": { "
        "\"Name\": \"%d\", "
        "\"timestamp\": \"%s\", "
        "\"Field_1\": \"Longitude: %s\", "
        "\"Field_2\": \"Latitude: %s\", "
        "\"Field_3\": \"Speed: %s\", "
        "\"Field_4\": \"Direction: %s\" }, "
        "\"geometry\": { \"type\": \"Point\", \"coordinates\": [ %s, %s ] } }",
        contadorName,
        tiempoISO,
        lonStr,
        latStr,
        speedStr,
        dirStr,
        lonStr,
        latStr);

    if (escritos < 0 || escritos >= buffer_size) {
        // Si hay un error en la escritura o si el buffer no es suficientemente grande.
        fprintf(stderr, "Error al escribir JSON o buffer insuficiente\n");
        return 0;
    }

    return (size_t)escritos;
}



int main(void) {
    /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
    PE_low_level_init();
    /*** End of Processor Expert internal initialization.                    ***/

    char bufferDeSentencia[LONG_MAX_CADENA]; // Buffer para almacenar una sentencia NMEA
    size_t indexBuffer = 0; // Índice actual en el buffer
    memset(bufferDeSentencia, 0, LONG_MAX_CADENA); // Inicializar el buffer

    char caracterRecibido; // Variable para almacenar el caracter recibido
    GPSData datos;
    int contadorName = 0; // el contador para Name
    char featureBuffer[LONG_MAX_CADENA];
    bool inicioSentenciaDetectado = false; // Nuevo flag para detectar el inicio de una sentencia

    // Inicia el JSON de FeatureCollection
    SendString("{ \"type\": \"FeatureCollection\", \"name\": \"Points\", "
               "\"crs\": { \"type\": \"name\", \"properties\": { \"name\": \"urn:ogc:def:crs:OGC:1.3:CRS84\" } }, "
               "\"features\": [");

    while (true) {
        // Espera y lee un caracter
        if (AS2_RecvChar(&caracterRecibido) == ERR_OK) {
            // Verifica si el caracter indica el inicio de una sentencia NMEA
            if (caracterRecibido == '$') {
                inicioSentenciaDetectado = true; // Establece el flag cuando se detecta el inicio de una sentencia
                indexBuffer = 0; // Reinicia el índice del buffer
            }

            if (inicioSentenciaDetectado) {
                // Agregar el caracter al buffer y avanzar el índice
                bufferDeSentencia[indexBuffer++] = caracterRecibido;

                // Verifica si el caracter indica el final de una sentencia NMEA
                if (caracterRecibido == '\n' || caracterRecibido == '\r') {
                    bufferDeSentencia[indexBuffer] = '\0'; // Asegura que la cadena está terminada
                    char resultado = parseNMEASentence(bufferDeSentencia, &datos);
                    if (resultado == 'P') {
                        memset(featureBuffer, 0, LONG_MAX_CADENA);
                        size_t escribe = escribeJSON(&datos, featureBuffer, LONG_MAX_CADENA, contadorName);
                        if (escribe > 0) {
                            if (contadorName > 0) {
                                SendString(", ");
                            }
                            SendString(featureBuffer);
                            contadorName++;
                        }
                    }
                    inicioSentenciaDetectado = false; // Reinicia el flag después de procesar una sentencia
                    memset(bufferDeSentencia, 0, LONG_MAX_CADENA); // Limpia el buffer para la siguiente sentencia
                }


                if (indexBuffer >= LONG_MAX_CADENA) {
                    fprintf(stderr, "Error: buffer de sentencia NMEA desbordado.\n");
                    inicioSentenciaDetectado = false; // Reinicia el flag si se desborda el buffer
                    memset(bufferDeSentencia, 0, LONG_MAX_CADENA); // Limpia el buffer
                    indexBuffer = 0; // Reinicia el índice del buffer
                }
            }
        } else {

        }
    }

    // Finaliza el JSON de FeatureCollection
    SendString(" ] }");

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

