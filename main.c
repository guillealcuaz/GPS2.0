/* ###################################################################
**     Filename    : main.c
**     Project     : DataLogger
**     Processor   : MK64FN1M0VLL12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2018-10-23, 19:57, # CodeGen: 0
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
** @author Alejandro Fernandez Lampreave y Guillermo Alcuaz Temiño
** @version 2.0
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
#include "stdio.h"
#include "Cpu.h"
#include "Events.h"
#include "FAT1.h"
#include "SD1.h"
#include "SS1.h"
#include "CD1.h"
#include "UTIL1.h"
#include "PPS.h"
#include "ExtIntLdd1.h"
#include "GPS.h"
#include "ASerialLdd2.h"
#include "AS1.h"
#include "ASerialLdd1.h"
#include "LEDR.h"
#include "LEDpin1.h"
#include "BitIoLdd1.h"
#include "LEDG.h"
#include "LEDpin2.h"
#include "BitIoLdd2.h"
#include "UTIL2.h"
#include "FRTOS1.h"
#include "KSDK1.h"
#include "TmDt1.h"
#include "WAIT1.h"
#include "TMOUT1.h"
#include "HF1.h"
#include "MCUC1.h"
#include "CS1.h"
#include "SM1.h"
#include "TI1.h"
#include "TimerIntLdd1.h"
#include "Bit2.h"
#include "BitIoLdd4.h"
#include "Bits1.h"
#include "BitsIoLdd1.h"
#include "ledRojo.h"
#include "BitIoLdd3.h"
#include "ledVerde.h"
#include "BitIoLdd5.h"
#include "TU1.h"
#include "GI2C1.h"
#include "CI2C1.h"
#include "FX1.h"
#include "PORT_PDD.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PDD_Includes.h"
#include "Init_Config.h"
#include "FRTOS1.h"
#include <queue.h>
#include "KSDK1.h"
#include "task.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ff.h"
#include "TmDt1.h"

const static byte longitud = 155;
const static byte tamano   = 1;
static FAT1_FATFS fileSystemObject;
static FIL file;
int16_t x,y,z;
char cadena[256];
static xQueueHandle caracteres;

#define SIZE 64
#define MAX_SIZE 1000
#define LONG_MAX_CADENA 500

// Stack Asignado a cada tarea
#define CHAR_GPS_STACK_SIZE (configMINIMAL_STACK_SIZE *2)
#define IMPRIME_STACK_SIZE (configMINIMAL_STACK_SIZE*4)
#define ACCE_STACK_SIZE (configMINIMAL_STACK_SIZE*2)


/* Definición de la estructura GPSData que almacena los datos del GPS */
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
    char fecha[64];
} GPSData;


/* Declaraciones de manejadores de tareas para FreeRTOS */
TaskHandle_t TaskHandle_charGPS;
TaskHandle_t TaskHandle_HANDLER;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Convierte grados y minutos a formato decimal.
 * @param gradosMinutosStr Cadena que contiene grados y minutos.
 * @param direccion Caracter que indica la dirección (N, S, E, W).
 * @return Valor decimal de los grados.
 */

double convertirADecimal(const char* gradosMinutosStr, char direccion) {
    char gradosStr[4] = { 0 };
    char minutosStr[10] = { 0 };
    int gradosCount = (direccion == 'N' || direccion == 'S') ? 2 : 3;

    strncpy(gradosStr, gradosMinutosStr, gradosCount);
    strncpy(minutosStr, gradosMinutosStr + gradosCount, sizeof(minutosStr - 1));
    double grados = atof(gradosStr);
    double minutos = atof(minutosStr);
    double decimal = grados + (minutos / 60.0);

    if (direccion == 'S' || direccion == 'W') {
        decimal = -decimal;
    }

    return decimal;
}

/**
 * Formatea el tiempo en formato ISO a partir de una cadena de tiempo NMEA.
 * @param destino Buffer donde se almacenará el tiempo formateado.
 * @param tiempoNmea Cadena que contiene el tiempo en formato NMEA.
 */

void formatearTiempoISO(char* destino, const char* tiempoNmea, const char* fecha) {
    // tiempo al formato HH:MM:SS+00
    int horas, minutos, segundos;
    int dia, mes, ano;
    sscanf(tiempoNmea, "%2d%2d%2d", &horas, &minutos, &segundos);
    sscanf(fecha, "%2d%2d%2d", &dia, &mes, &ano);
    ano += 2000;
    snprintf(destino, 50, "%04d-%02d-%02d_%02d:%02d:%02d", ano, mes, dia, horas, minutos, segundos);
}


/**
 * Extrae un dato del GPS y lo procesa con la función extractor.
 * @param datos Estructura GPSData para almacenar los datos extraídos.
 * @param token Token actual de la cadena NMEA procesada.
 * @param extractor Función para procesar el dato extraído.
 * @return Caracter que indica el resultado de la extracción.
 */

char extraerDato(GPSData* datos, char** token, void (*extractor)(GPSData*, const char*)) {
    *token = strtok(NULL, ",");
    if (!*token) {
        fprintf(stderr, "Error al extraer un dato.\n");
        return 'E';
    }
    extractor(datos, *token);
    return 'P';
}

/**
 * Extrae el tiempo de un token y lo almacena en la estructura GPSData.
 * @param datos Estructura GPSData para almacenar el tiempo.
 * @param token Token que contiene el tiempo.
 */

void extraerTiempo(GPSData* datos, const char* token) {
    strncpy(datos->tiempo, token, sizeof(datos->tiempo) - 1);
    datos->tiempo[sizeof(datos->tiempo) - 1] = '\0';
}

/**
 * Extrae el estado de un token y lo almacena en la estructura GPSData.
 * @param datos Estructura GPSData para almacenar el estado.
 * @param token Token que contiene el estado.
 */
void extraerEstado(GPSData* datos, const char* token) {
    if (token[0] == 'A') {
        datos->estado = 'A';
    } else {
        datos->estado = 'V';
        Err();
    }
}


/**
 * Extrae la latitud de un token y la convierte a formato decimal.
 * @param datos Estructura GPSData para almacenar la latitud.
 * @param token Token que contiene la latitud.
 */

void extraerLatitud(GPSData* datos, const char* token) {
    strncpy(datos->latitudStr, token, SIZE - 1);
    datos->latitudStr[SIZE - 1] = '\0';
    datos->direccionLatitud = *(strtok(NULL, ","));
    datos->latitud = convertirADecimal(datos->latitudStr, datos->direccionLatitud);
}

/**
 * Extrae la longitud de un token y la convierte a formato decimal.
 * @param datos Estructura GPSData para almacenar la longitud.
 * @param token Token que contiene la longitud.
 */

void extraerLongitud(GPSData* datos, const char* token) {
	strncpy(datos->longitudStr, token, SIZE - 1);
    datos->longitudStr[SIZE - 1] = '\0';
    datos->direccionLongitud = *(strtok(NULL, ","));
    datos->longitud = convertirADecimal(datos->longitudStr, datos->direccionLongitud);
}



/**
 * Extrae la velocidad de un token y la almacena en la estructura GPSData.
 * @param datos Estructura GPSData para almacenar la velocidad.
 * @param token Token que contiene la velocidad.
 */

void extraerVelocidad(GPSData* datos, const char* token) {
    sscanf(token, "%lf", &datos->velocidad);
    datos->velocidad = datos->velocidad * 1.15078;
}

/**
 * Extrae la dirección de un token y la almacena en la estructura GPSData.
 * @param datos Estructura GPSData para almacenar la dirección.
 * @param token Token que contiene la dirección.
 */
void extraerDireccion(GPSData* datos, const char* token) {
    sscanf(token, "%lf", &datos->direccion);
}

/**
 * Extrae la fecha de un token y la almacena en la estructura GPSData.
 * @param datos Estructura GPSData para almacenar la fecha.
 * @param token Token que contiene la fecha.
 */

void extraerFecha(GPSData* datos, const char* token) {
    strncpy(datos->fecha, token, sizeof(datos->fecha) - 1);
    datos->fecha[sizeof(datos->fecha) - 1] = '\0';
}


/**
 * Analiza una sentencia NMEA y extrae los datos relevantes para almacenarlos en una estructura GPSData.
 * @param cadenaNmea La cadena NMEA a analizar.
 * @param datos Estructura GPSData donde se almacenarán los datos extraídos.
 * @return Caracter que indica el resultado del análisis ('P' para procesado correctamente, 'E' para error, 'I' para ignorado).
 */


char parseNMEASentence(const char* cadenaNmea, GPSData* datos) {
    memset(datos, 0, sizeof(GPSData));
    if (strncmp(cadenaNmea, "$GPRMC", 6) == 0) {
    	char* token = strtok(cadenaNmea, ",");
        if (!token) {
            fprintf(stderr, "Error: Sentencia vacía.\n");
            return 'E';
        }

        if (extraerDato(datos, &token, extraerTiempo) == 'E' ||
            extraerDato(datos, &token, extraerEstado) == 'E' ||
            extraerDato(datos, &token, extraerLatitud) == 'E' ||
            extraerDato(datos, &token, extraerLongitud) == 'E' ||
            extraerDato(datos, &token, extraerVelocidad) == 'E' ||
            extraerDato(datos, &token, extraerDireccion) == 'E' ||
            extraerDato(datos, &token, extraerFecha) == 'E') {
            fprintf(stderr, "Error al procesar un campo específico de la sentencia GPRMC.\n");
            return 'E';
        }
        return 'P';
    } else {
        return 'I';
    }
}

/**
 * Escribe los datos de GPS en formato JSON en un buffer proporcionado.
 * @param data Puntero a la estructura GPSData con los datos a escribir.
 * @param buffer Puntero al buffer donde se escribirá el JSON.
 * @param buffer_size Tamaño del buffer proporcionado.
 * @param contadorName Entero utilizado para el nombre del objeto en el JSON.
 * @return Tamaño de los datos escritos en el buffer, 0 si hay error.
 */

size_t escribeJSON(const GPSData* data, char* buffer, size_t buffer_size, int contadorName) {
	char tiempoISO[50];
	if (isnan(data->latitud) || isnan(data->longitud) || isnan(data->velocidad) || isnan(data->direccion)) {
        return 0;
    }
    char latStr[50] = {0};
    char lonStr[50] = {0};
    char velStr[50] = {0};
    char dirStr[50] = {0};

    snprintf(lonStr, sizeof(lonStr), "%.6f", data->longitud);
    snprintf(latStr, sizeof(latStr), "%.6f", data->latitud);
    snprintf(velStr, sizeof(velStr), "%.1f mph", data->velocidad);
    snprintf(dirStr, sizeof(dirStr), "%.2f degrees", data->direccion);
    formatearTiempoISO(tiempoISO, data->tiempo, data->fecha);


    if (buffer_size < LONG_MAX_CADENA) {
        fprintf(stderr, "buffer insuficiente\n");
        return 0;
    }
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
        velStr,
        dirStr,
        lonStr,
        latStr);

    if (escritos < 0 || escritos >= buffer_size) {
        fprintf(stderr, "Error al escribir  o buffer insuficiente\n");
        return 0;
    }
    return (size_t)escritos;
}

/**
 * Envía una cadena de caracteres por puerto serie
 * @param cadena La cadena de caracteres a enviar.
 */

void SendString(const char* cadena) {
	int largo = strnlen(cadena, MAX_SIZE);
    int enviados;
    if (largo > 0) {
        byte err = AS1_SendBlock(cadena, largo, &enviados);
        if (err != ERR_OK) {
           fprintf(stderr, "Error al enviar datos: %d\n", err);
        } else {
           printf("Bytes enviados: %u\n", enviados);
        }
    }
}


/**
 * Inicia la estructura de un objeto geoJSON que representa una colección de características para datos GPS.
 * La función envía la cabecera inicial de un JSON a través de una comunicación serial y también la escribe en una tarjeta SD.
 */
void cabeceraJSON() {
    const char *cabecera = "{ \"type\": \"FeatureCollection\",\n \"name\": \"GPS\",\n "
                           "\"features\": [\n";
    SendString(cabecera);
    EscribeSD(cabecera);
}
/**
 * Termina la estructura del objeto geoJSON que representa una colección de características para datos GPS.
 * La función envía el pie de cierre de un JSON a través de una comunicación serial y también lo escribe en una tarjeta SD.
 */
void finJSON() {
    const char *pie = " ] }";
    SendString(pie);
    EscribeSD(pie);
}

/**
 * Manejador para eventos de pulsador. Monitorea el estado de dos pulsadores y controla el flujo de la tarea `charGPS`.
 * Si el pulsador 1 está activado, detiene la tarea y finaliza la estructura JSON.
 * Si el pulsador 2 está activado, reanuda la tarea.
 *
 * @param param Parámetros pasados a la función (no utilizados en este contexto).
 */
static void PulsadorHandler(void* param) {
    for(;;) {
        if(!Bits1_GetVal()){
			ledVerde_PutVal(1);
			ledRojo_PutVal(0);
			finJSON();
            vTaskSuspend(TaskHandle_charGPS);
        }
		if(!Bit2_GetVal()){
			ledVerde_PutVal(0);
    		cabeceraJSON();
			vTaskResume(TaskHandle_charGPS);
		}

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Err(void) {
  for(;;){
	  // Bucle infinito que se activa en caso de error
	  // Garantiza que la ejecución se detiene aquí para detectar el rror
  }
}

/*
 * Procedimiento encargado de inicializar el sistema de ficheros y deteccion
 * de tarjeta
 */

void StorageOn(){
/* Deteccion de la tarjeta SD: PTE6 con pull-down! */
 PORT_PDD_SetPinPullSelect(PORTE_BASE_PTR, 6, PORT_PDD_PULL_DOWN);
 PORT_PDD_SetPinPullEnable(PORTE_BASE_PTR, 6, PORT_PDD_PULL_ENABLE);
/* Se inicializa el sistema de almacenamiento*/
 FAT1_Init();
 if (FAT1_mount(&fileSystemObject, "0", 1) != FR_OK){ /* Comprueba el archivo del sistema */
	 Err();
 }
}

/*
 * Procedimiento encargado de escribir en la tarjeta una cadena dada.
 * @param cadena Cadena que contiene los caracteres del GPS
 */

void EscribeSD(char *cadena){
	  UINT bandwidth;
	  /* Abrir fichero */
	  if (FAT1_open(&file, "./log_gps.txt", FA_OPEN_ALWAYS|FA_WRITE)!=FR_OK) {
	    Err();
	  }
	  /* Nos posicionamos en el final del archivo */
	  if (FAT1_lseek(&file, f_size(&file)) != FR_OK || file.fptr != f_size(&file)) {
	    Err();
	  }
	  /* Se escribe la cadena en la microSD */
	  if (FAT1_write(&file, cadena, UTIL1_strlen(cadena), &bandwidth)!=FR_OK) {
		  (void)FAT1_close(&file);
		  Err();
	  }
	  /* Cerrar el fichero */
	  (void)FAT1_close(&file);
}

/*
 * Tarea que actualiza los valores de los ejes en cada instante.
 */

static void Acce(void) {
	FX1_Enable(); //Activa el acelerometro
	/* Calibra los diferentes ejes, suponiendo 0G en 'x' e 'y' y la fuerza
	 * normal de la gravedad 1G en el 'z' */
	FX1_CalibrateZ1g();
	for(;;) {
		  x = FX1_GetX();
		  y = FX1_GetY();
		  z = FX1_GetZ();
		  vTaskDelay(500/portTICK_RATE_MS); //retraso de 1s
	}
}

/*
 * Tarea que imprime los caracteres, tanto por una terminal, como en la
 * tarjeta.
 */

static void Imprime(void) {
    GPSData datos;
    char bufferDeSentencia[LONG_MAX_CADENA];
    char ch;
    char geojsonBuffer[LONG_MAX_CADENA];
    int contadorName = 0;
    int indexBuffer = 0;
    bool inicioSentenciaDetectado = false;
    StorageOn();

    for (;;) {
        if (FRTOS1_xQueueReceive(caracteres, &ch, portMAX_DELAY) == pdTRUE) {
            if (ch == '$') {
                inicioSentenciaDetectado = true;
                indexBuffer = 0;
            }
            if (inicioSentenciaDetectado) {
                bufferDeSentencia[indexBuffer++] = ch;
                if (ch == '\n' || ch == '\r') {
                    bufferDeSentencia[indexBuffer] = '\0';
                    char resultado = parseNMEASentence(bufferDeSentencia, &datos);
                    if (resultado == 'P') {
                        if (contadorName > 0) {
                            SendString(",\n");
                            EscribeSD(",\n");
                        }
                        memset(geojsonBuffer, 0, LONG_MAX_CADENA);
                        size_t escribe = escribeJSON(&datos, geojsonBuffer, LONG_MAX_CADENA, contadorName++);
                        if (escribe > 0) {
                        //	if( x>500 || x<-500 || y>500 || y<-500 ){
                        		SendString(geojsonBuffer);
                        		EscribeSD(geojsonBuffer);
                        //		}
                        }
                    }
                    inicioSentenciaDetectado = false;
                    indexBuffer = 0;
                }

                if (indexBuffer >= LONG_MAX_CADENA) {
                    inicioSentenciaDetectado = false;
                    indexBuffer = 0;
                }
            }
        }
    }


}

/*
 * Tarea encargada de introducir al final de la cola los caracteres del GPS.
 */
static void CharGPS(void) {
	byte err;
	char ch;
	GPS_ClearRxBuf(); //limpiamos el buffer del gps
	for(;;) {
		do {err = GPS_RecvChar(&ch);
		} while((err != ERR_OK));
		FRTOS1_xQueueSendToBack(caracteres, &ch , (portTickType) 0xFFFFFFFF); //metemos el caracter a la cola
	}
}



/* User includes (#include below this line is not maintained by Processor Expert) */
/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */
	caracteres=FRTOS1_xQueueCreate(longitud,tamano);
	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/


  /* Write your code here */


    // Creación de la tarea CharGPS
    if (xTaskCreate(
        CharGPS, "gps", CHAR_GPS_STACK_SIZE, NULL, 3, &TaskHandle_charGPS
    ) != pdPASS) {
        printf("Error al crear la tarea CharGPS\n");
        fflush(stdout);
        for(;;);
    } else {
        printf("Tarea CharGPS creada exitosamente\n");
        fflush(stdout);
    }

    // Creación de la tarea Imprime
    if (xTaskCreate(
        Imprime, "print", IMPRIME_STACK_SIZE, NULL, 4, NULL
    ) != pdPASS) {
        printf("Error al crear la tarea Imprime\n");
        for(;;);
    } else {
       printf("Tarea Imprime creada exitosamente\n");
        fflush(stdout);
    }

    // Creación de la tarea Acce
    if (xTaskCreate(
        Acce, "Acc", ACCE_STACK_SIZE, NULL, 2, NULL
    ) != pdPASS) {
       printf("Error al crear la tarea Acce\n");
        for(;;);
    } else {
        printf("Tarea Acce creada exitosamente\n");
        fflush(stdout);
    }
    if (xTaskCreate(PulsadorHandler, "HandlerPulsador", configMINIMAL_STACK_SIZE, NULL, 4, TaskHandle_HANDLER
    ) != pdPASS) {
    	Err();
    }
  /* For example: for(;;) { } */

  vTaskSuspend(TaskHandle_charGPS);
  FRTOS1_vTaskStartScheduler();



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

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/

