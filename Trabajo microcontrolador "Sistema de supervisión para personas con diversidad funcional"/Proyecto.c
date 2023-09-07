//***************************************************************************//
//                                                                           //
//        Proyecto de microcontroladores SEPA 4� GIERM. Realizado por:       //
//          - Francisco Javier Rom�n Cort�s. (fraromcor3)                    //
//          - Juan de Dios Herrera Hurtado. (juaherhur)                      //
//                                                                           //
//***************************************************************************//
/*****************************************************************************/
//
//      --> Resumen/Explicaci�n del proyecto planteado:
//
// Aunque puede parecer algo �spero desde fuera, ten�amos claro que quer�amos
// explorar el perif�rico Ethernet y sus posibilidades. En primer lugar, comenzamos
// planteando la idea de comunicar 2 micros usando protocolo UDP, de manera que
// un cuidador pudiese tener comunicaci�n directa con una persona con necesidades especiales.
// Sin embargo, la complejidad de establecer un intercambio de informaci�n entre ambos micros
// (al menos via Ethernet(UDP)) se hizo notoria y hubo que reenfocar el proyecto.
// En ese punto, vimos como otra opci�n montar una p�gina web simple que estar� embebida
// en el micro de manera que es el que la hostea. Dicha p�gina ofrece una interfaz simple
// mediante la cual una persona con necesidades tiene contacto directo con una "centralita"
// donde se recibir�n las notificaciones de avisos, separ�ndolos en "Urgentes" y "No urgentes",
// de manera que se tiene definida una prioridad sobre qu� avisos son m�s importantes.
// Esta persona podr� rellenar un peque�o formulario con sus datos e indicar� si el aviso es urgente
// o no, adem�s tiene la posibilidad de enviar un mensaje de texto con ciertos comentarios que vea
// conveniente.
// Para el punto de vista de la centralita, se ha desarrollado lo que se podr�a denominar como
// una aplicaci�n o interfaz con diferentes men�s que permiten gestionar el registro de los diferentes
// avisos recibidos, ya sea vi�ndolos y volviendo al registro sin actuar sobre dicho aviso, viendo los
// detalles (nombre, edad, direcci�n...), as� como viendo el mensaje adjunto (si lo hubiese).
// En esta aplicaci�n, se emula lo que ser�a el env�o de ayuda a la solicitud entrante, moviendo el servomotr
// (puede emular la apertura de una puerta, por ejemplo), as� como haciendo sonar cierto sonido.
// Tambi�n, cuando se recibe un aviso o solicitud de ayuda, la interfaz muestra una alerta parpadeante visual y sonora
// que permita reconocer con claridad la llegada de dicho aviso.
// Finalmente, entre otros detalles y animaciones desarrollados, destaca el borrado autom�tico del aviso atendido una
// vez se ha atendido y enviado ayuda a dicho aviso, reordenando en la pantalla de registros el resto de avisos.
//
//      --> Principios t�cnicos de funcionamiento:
//
// En t�rminos generales, nos basamos en el controlador Ethernet disponible en
// Microcontrolador TIVA ek-tm4c1294xl y en la pila lwIP TCP/IP.  El protocolo
// que se utiliza para obtener una direcci�n de Ethernet es DHCP
// (Dynamic Host Configuration Protocol). Si DHCP llega a un timeout (45s) sin
// lograr obtener una direcci�n IP, se elige una IP est�tica usando AutoIP.
// La direcci�n obtenida, se muestra por la UART, de manera que se puede acceder
// a la p�gina web confeccionada, hosteada por el microcontrolador a trav�s de
// cualquier navegador.
// Respecto a esto �ltimo, la p�gina web s�lo es accesible desde el pc conectado
// al microcontrolador via Ethernet, lo cual limita las posibilidades del
// desarrollo de la aplicaci�n realizada a una especie de prototipo. Es decir,
// podr�a ser un ejemplo de c�mo prototipar un producto que podr�a escalar a
// una versi�n real y mucho m�s potente.
// A donde se quiere llegar con esto es que, si se dispusiese de m�s tiempo,
// recursos y conocimientos sobre protocolos de red (principalmente hosteo de webs
// embebidas en un microcontrolador de manera que sea accesible de manera remota),
// no ser�a inviable escalar este proyecto/prototipo a una fase en la cual, la aplicaci�n
// quedase hosteada en una p�gina web habitual embebida en este microcontrolador y accesible
// de manera remota desde cualquier terminal (PC, m�vil, etc.), haci�ndolo un sistema mucho m�s pr�ctico.
//
//      --> Conceptos acerca de c�mo se obtiene la informaci�n procedente de la p�gina web:
//
// Una vez entramos en la p�gina web, nos aparece la p�gina inicial, con una presentaci�n de la p�gina.
// Para poder conformar y enviar un aviso, debemos ir al enlace situado en la parte izquierda:
// "Servicios de Ayuda//Contacto con supervisor//Alerta de emergencia".
// En este enlace, aparece un formulario, en el cual se solicitan los datos necesarios para poder enviar
// la ayuda, as� como la posibilidad de indicar si es urgente o no y de adjuntar un mensaje explicativo si se considera necesario.
//
// Para recoger la informaci�n procedente de este formulario, se usan formularios HTML est�ndar que pasan par�metros
// a un handler (o manejador de interrupci�n) CGI (Common Gateway Interface) que se ejecuta en la placa.
// En el programa, se registra el handler, de manera que cuando se pulse el bot�n de "Solicitar Atenci�n",
// dicho formulario, definido por: <form method="get" action="recoger_datos.cgi" name="recoger_datos">, ejecuta la acci�n
// "recoger_datos.cgi", que activa el handler donde se decodifica la informaci�n (se explicar� con m�s detalle en el handler).
// Para decodificar dicha informaci�n se parsea la p�gina HTML con los datos incluidos y se decodifica el mensaje introducido
// asociado a cada "id" de cada "text box": <input value="" maxlength="34" size="35" name="nombre" required>, donde el "nombre", ser�a
// la "id" a buscar al parsear, y "value" el valor de la string recibida en dicho campo.
// Con esto, podemos recoger dicha informaci�n y pasar al problema de su procesamiento y almacenamiento de manera ordenada.
//
// Destacar tambi�n que, al enviar el formulario, la p�gina web se refresca, quedando preparada para el env�o de un nuevo aviso.
//
//      --> Elaboraci�n de la p�gina web: (HTML b�sico, modificaci�n de estilos...)
//
// Se incluyen en la carpeta "fs\" los c�digos fuentes que conforman dicha p�gina web.
// Si se modifica alguno de estos archivos (para modificar la web), el archivo de imagen del sistema (iofsdata.h)
// debe ser recompilado mediante la utilidad disponible "makefsfile" disponible en la carpeta de TivaWare.
// Para usar esta herramienta, tenemos que desplazarnos al directorio donde est� alojado el proyecto (con cd \..)
// y tras ello: >> ../../../../tools/bin/makefsfile -i fs -o io_fsdata.h -r -h -q
// Posiblemente se puede modificar el path si se desplaza el archivo "makefsfile" a otro lugar deseado.
//
// Referencias sobre la pila lwIP(Lightweight IP):
// http://savannah.nongnu.org/projects/lwip/
// https://git.savannah.nongnu.org/cgit/lwip.git
//
//*************************************************************************************************************************************************//
/////////////////////////////////////////////////////// DECLARACI�N DE LIBRERIAS ////////////////////////////////////////////////////////////
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <FT800_TIVA.h>
#include "driverlib2.h"
#include "utils/locator.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "httpserver_raw/httpd.h"
#include "drivers/pinout.h"
#include "io.h"
#include "cgifuncs.h"

//*************************************************************************************************************************************************//

#define dword long
#define byte char

// Defines para los botones presentes en la interfaz de la aplicaci�n desarrollada:
#define Boton_volver Boton(HSIZE/2-130, 7*VSIZE/9+14, 63, 28, 23, "Volver")
#define Boton_ver_mensaje Boton(HSIZE/2+20, 7*VSIZE/9+14, 115, 28, 23, "Ver mensaje")
#define Boton_enviar_ayuda Boton(HSIZE/2+20, 7*VSIZE/9+14, 115, 28, 23, "Enviar ayuda")
#define Boton_IP_introducida Boton(HSIZE/2-70, VSIZE/2+30, 140, 40, 23, "IP introducida")

// ==============================================================================================================================================
// Declaraci�n/Inicializaci�n de variables para pantalla FT800
// ==============================================================================================================================================

char chipid = 0;                        // Holds value of Chip ID read from the FT800

unsigned long cmdBufferRd = 0x00000000;         // Store the value read from the REG_CMD_READ register
unsigned long cmdBufferWr = 0x00000000;         // Store the value read from the REG_CMD_WRITE register
unsigned int t=0;

unsigned long POSX, POSY, BufferXY;
unsigned int CMD_Offset = 0;
int32_t REG_TT[6];
const int32_t REG_CAL[6]={22073,59,-1044578,400,-16911,16162095};
const int32_t REG_CAL5[6]={32146, -1428, -331110, -40, -18930, 18321010};   // Calibraciones de las pantallas (se ha realizado en la de 5")

//==============================================================================================================================================
// Lista de variables:
// CHAR:
char cadena_primera_linea[50];  //Al mostrar el mensaje de un aviso (si lo hay), se han creado cinco strings
char cadena_segunda_linea[50];  //que permiten separar todo el mensaje recibido de manera que se pueda
char cadena_tercera_linea[50];  //mostrar correctamente por la pantalla sin que el mensaje se salga
char cadena_cuarta_linea[50];   //de los l�mites de la pantalla, que es lo que ocurrir�a si no se separase
char cadena_quinta_linea[50];   //el mensaje en trozos
char cadena_mensaje[200];       //En esta string se guarda el mensaje del aviso que, posteriormente se dividir� en las cinco cadenas previamente comentadas
char cadena_urgente[] = "Mensaje urgente";      //Seg�n si el aviso es urgente o no, el manejador de
char cadena_no_urgente[] = "Mensaje rutinario"; //interrupci�n da un valor u otro a "cadena_tipo_mensaje"
char cadena_tipo_mensaje[50];                   //Toma el valor de alguna de las dos cadenas predefinidas seg�n si el mensaje es urgente o no, todo ello se realiza en el manejador de interrupci�n

// VOLATILE CHAR (para string modificada en interrupciones):
volatile char cadena_hora[50];  //Cuando llega un nuevo aviso, el manejador de interrupci�n rellena el campo "Hora_s" de la estructura del aviso con esta cadena (se basa en el tiempo de ejecuci�n del programa que est� controlado por el TIMER1 A)
volatile char Nombre[50];
volatile char Apellidos[50];    //Cadenas en las que se guardan los valores del aviso que luego se usar�n para rellenar los diferentes campos de la
volatile char Direccion[50];    //de la estructura que caracteriza al aviso (faltar�a por caracterizar el campo "Tipo_mensaje_s" que se rellena con
volatile char Edad[50];         //"cadena_tipo__mensaje")
volatile char Telefono[50];
volatile char Mensaje[200];

// VOLATILE INT (para enteros modificados en interrupciones (CGI handler y timer)):
volatile int Max_pos = 4200;        //Posici�n izquierda del servomotor
volatile int Med_pos = 2750;        //Posici�n central del servomotor
volatile int Min_pos = 1300;        //Posici�n derecha del servomotor
volatile int flag_estado_alerta = 0;//Variable que permite realizar el cambio de color del fondo de la pantalla en el estado de alerta que avisa de que ha llegado un nuevo aviso
volatile int contador_estado8 = 0;  //Contadores que permiten realizar animaciones en el estado 2 (pantalla de alerta de aviso entrante),
volatile int contador_estado2 = 0;  //en el estado 8 (pantalla de env�o de ayuda)
volatile int contador_borrando = 0; //y en el estado 5 (pantalla de borrado)
volatile int horas=0, minutos=0, segundos=0; //Variables que se actualizan en el TIMER1 A para llevar el tiempo de ejecuci�n
volatile int estado = 0;            //Variable fundamental que rige la m�quina de estados (FSM)
volatile int direccion_obtenida = 0;//Esta variable hace que permanezcamos en el estado 0 (pantalla de carga inicial) hasta que el protocolo DHCP nos proporciona una direcci�n IP para nuestra p�gina web
volatile int cambio_estado = 0;     //Cuando llega un aviso, el manejador de interrupci�n activa esta variable que har� que en la m�quina de estados pasemos a la pantalla que alerta de que hay un nuevo aviso (para que salte la pantalla de alerta debemos de estar en la pantalla del registro de mensajes)
volatile int cont_circulos = 0;     //Variable para realizar la animaci�n de los "..." en la pantalla de carga inicial. La secuencia que realiza es "", ".", "..", "..." y as� c�clicamente
volatile int mensajes_urgentes = 0; //Almacena la cantidad de avisos urgentes que tenemos almacenados
volatile int mensajes_no_urgentes = 0; //Almacena la cantidad de avisos no urgentes que tenemos almacenados
volatile int activa_estado_1 = 0;   //Estas variables se utilizan para habilitar la pulsaci�n de los botones y del quinto mensaje
volatile int activa_estado_6 = 0;   //que aparece por pantalla en la columna de los mensajes urgentes. Esto se hace porque al
volatile int activa_estado_7 = 0;   //tener botones colocados en la misma posici�n en diferentes estados, al pulsarlos, inmediatamente
volatile int activa_estado_9 = 0;   //se activaba tambi�n el bot�n del estado al que se pasaba que se encontraba en coordenadas similares al bot�n pulsado
volatile int urgente = 0;           //Toma valor '-1' si no se marca la casilla de aviso urgente y '0' si se marca

// INT:
int registro = 0;                   //En la pantalla de registro se muestran 5 avisos de cada tipo, seg�n qu� aviso se pulse para ver, esta variable toma valor en el rango [1,5], siendo 1 el aviso de m�s arriba (el m�s antiguo) y 5 el aviso de la parte inferior de la pantalla (no importa si el aviso es urgente o no, se usa para ambos)
int volver_mensaje_no_urgente = 0;  //En el estado 9 mostramos el mensaje del aviso en caso de tener, si volvemos para ver los detalles del aviso (nombre, apellidos...),
int volver_mensaje_urgente = 0;     //necesitamos saber si el aviso del que se deben mostrar los detalles era urgente (estado 6) o no (estado 7). Todo esto es posible conociendo tambi�n el valor de "registro"
int ver_mensaje_urgente = 0;        //Estas dos pr�ximas variables son an�logas pero al pasar del estado 6/7 donde se ven los detalles del aviso,
int ver_mensaje_no_urgente = 0;     //al estado 9, donde debemos saber qu� mensaje debemos mostrar si urgente o no. Nuevamente nos apoyamos tambi�n en la variable "registro"
int borrado_urgente = 0;            //Misma idea que los dos pares de variables que preceden a estas dos, al salir del estado 5 (animaci�n de que estamos borrando el aviso una vez se env�a la ayuda), debemos saber si pasar al
int borrado_no_urgente = 0;         // estado 3 (urgente) o 4 (no urgente), que es donde realmente se borra el registro y se desplazan todos los avisos que llegaron despu�s al que se borra una posici�n hacia arriba en el array de structs
int PeriodoPWM;                     //Periodo con el que se configura el PWM para mover el servomotor
int IndiceUrgente = 0;              //Se les da valor a la vez que "registro", es decir, al pulsar para ver
int IndiceNoUrgente = 0;            //los detalles de un aviso. Toman valor igual a registro - 1
int terminador = 0;                 //Esta variable nos permite saber en qu� posici�n se encuntra el terminador del mensaje del aviso, pudiendo tomar valores en el rango [0,199]
int flag_primera_linea = 0;
int flag_segunda_linea = 0;         //Variables para realizar el guardado del mensaje del aviso en diferentes partes de manera secuencial
int flag_tercera_linea = 0;         //Por ejemplo para el caso de la segunda l�nea, no se le dar� valor hasta que se haya terminado de manipular
int flag_cuarta_linea = 0;          //y rellenar la primera l�nea. As� con el resto de l�neas hasta llegar a la quinta que debe esperar a la cuarta l�nea
int flag_quinta_linea = 0;
int i = 0;
int j = 0;                          //�ndices para recorrer bucles for para manipular los arrays de structs y las l�neas con el mensaje del aviso
int k = 0;
int caracter = 0;                   //Al rellenar la segunda l�nea del mensaje por ejemplo, debemos saber hasta qu� �ndice del mensaje se ha guardado en la primera l�nea, esta variable nos lo indica

// STRUCT:
struct StructResumen {
   char  Tipo_mensaje_s[50];
   char  Hora_s[50];
   char  Nombre_s[50];
   char  Apellidos_s[50];           //Struct que caracteriza cada aviso que llega de la p�gina web (8 campos).
   char  Direccion_s[50];           //Se rellena en el manejador que salta cada vez que se env�a una aviso desde la p�gina
   char  Edad_s[50];
   char  Telefono_s[50];
   char  Mensaje_s[200];
};

// ARRAY OF STRUCTS:
//Array de struct de tama�o 20 para los avisos urgentes
struct StructResumen RegistroUrgente[20];

//Array de struct de tama�o 20 para los avisos no urgentes
struct StructResumen RegistroNoUrgente[20];

////////////////////////////////////// CONFIGURACI�N DE LA lwIP STACK (Ethernet) ////////////////////////////////////////

// Defines para hacer setup del reloj del sistema
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)

// Definiciones de las prioridades de interrupci�n. Los 3 bits superiores de estos valores son significativos con los valores
// m�s bajos indicando mayor prioridad de interrupci�n
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0

// Referencia a la funci�n que inicializa el servidor "httpd" (HTTP Apache)
extern void httpd_init(void);

// Prototipo para la funci�n handler CGI (Common Gateway Interface).
static char *HandlerCGI_Receptor_Datos(int32_t iIndex, int32_t i32NumParams, char *pcParam[], char *pcValue[]);

// Se registra la funci�n handler anterior como el manejador CGI de manera que informa
// al servidor HTTPD de que cierta URI: "/recoger_datos.cgi" debe ser tratada a trav�s del handler
// registrado si dicha URI se "activa". Es decir, al pulsar el bot�n en la web de "Solicitar Atenci�n",
// tenemos un "action": "recoger_datos.cgi" que har� que el handler procese los datos de inter�s del
// formulario dise�ado en dicha p�gina web.
static const tCGI Config_URIs[] =
{
    { "/recoger_datos.cgi", (tCGIHandler)HandlerCGI_Receptor_Datos },
};

// Numero de URIS CGI individuales configuradas para este sistema.
#define Numero_de_URIS_configuradas    (sizeof(Config_URIs) / sizeof(tCGI))

// Una vez se completa la acci�n del handler CGI, se devuelve al navegador el siguiente archivo (URI de la p�gina a cargar).
// Esto provoca que la p�gina web se recargue en la pesta�a del formulario, con este vac�o, tras haber recogido
// los datos introducidos, quedando lista para volver a ser usada.
#define Respuesta_Handler_CGI    "/solicitud_cgi.ssi"

// Timeout para la solicitud de direcci�n mediante DHCP (en segundos).
#ifndef DHCP_EXPIRE_TIMER_SECS
#define DHCP_EXPIRE_TIMER_SECS  45
#endif

// Variable para recoger la direcci�n IP actual.
uint32_t Direccion_IP_actual;

// Variable para la frecuencia del reloj del sistema:
uint32_t Reloj;

// Rutina de error que es llamada si se encuentra un error durante el debug
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

// Handler CGI que es llamado cada vez que el navegador web solicita "recoger_datos.cgi" (con m�todo GET)
static char *HandlerCGI_Receptor_Datos(int32_t iIndex, int32_t i32NumParams, char *pcParam[], char *pcValue[]){

    long lStringParam;  //Variable para guardar el indice si la cadena se encuentra (referida a su par�metro ("nombre","apellidos")), valdr� -1 si no se encuentra (no ocurre, la p�gina obliga a rellenar todos los datos)
    urgente = FindCGIParameter("urgente", pcParam, i32NumParams);   // busca el par�metro "urgente" para ver si se le ha pasado desde el formulario de la web y guarda su �ndice,
                                                                    // es decir, la variable urgente valdr� 0 (siempre es el primer par�metro del formulario) si se ha marcado la casilla de �es urgente?
                                                                    // o -1 si no se ha marcado y por tanto ese par�metro no ha sido pasado a este handler.

    if (urgente == -1) strcpy(cadena_tipo_mensaje, cadena_no_urgente);  // Sabiendo si es urgente o no, copiamos un mensaje predefinido en "cadena_tipo_mensaje", que identifica si el mensaje es urgente o no
    else if (urgente == 0) strcpy(cadena_tipo_mensaje, cadena_urgente);

    // A partir de aqu�, nos interesa recoger los mensajes introducidos por el usuario en los diferentes campos, para ello,
    // indicamos que se deben buscar los par�metros asociados a cada campo de texto, y al encontrarlos (Todos los datos son obligatorios
    // de rellenar (hemos forzado que la p�gina web obligue a rellenarlos)), se decodifica el mensaje incluido en cada campo de texto que
    // se ha rellenado, guard�ndolo en diferentes variables de inter�s.
    // Por ejemplo para el nombre, tenemos en el archivo "solicitud_cgi.ssi" lo siguiente:
    // <input value="" maxlength="34" size="35" name="nombre" required>
    // de manera que al introducir texto (limitado a 34 car�cteres), su campo "value" recoger� ese texto
    // y con la funci�n "DecodeFormString" extraemos una string con dicha informaci�n, que almacenamos.
    lStringParam = FindCGIParameter("nombre", pcParam, i32NumParams);
    DecodeFormString(pcValue[lStringParam], Nombre, 50);

    lStringParam = FindCGIParameter("apellidos", pcParam, i32NumParams);
    DecodeFormString(pcValue[lStringParam], Apellidos, 50);

    lStringParam = FindCGIParameter("direccion", pcParam, i32NumParams);
    DecodeFormString(pcValue[lStringParam], Direccion, 50);

    lStringParam = FindCGIParameter("edad", pcParam, i32NumParams);
    DecodeFormString(pcValue[lStringParam], Edad, 50);

    lStringParam = FindCGIParameter("numerotelefono", pcParam, i32NumParams);
    DecodeFormString(pcValue[lStringParam], Telefono, 50);

    lStringParam = FindCGIParameter("TextoSupervisor", pcParam, i32NumParams);
    DecodeFormString(pcValue[lStringParam], Mensaje, 200);

    if (urgente == 0){       // Si el mensaje es urgente:
        // Copiamos los datos decodificados en strings en nuestra componente, indexada por el n�mero de avisos
        // urgentes recibidos, del array de structs correspondiente a Registros Urgentes.
        strcpy(RegistroUrgente[mensajes_urgentes].Tipo_mensaje_s, cadena_tipo_mensaje);
        strcpy(RegistroUrgente[mensajes_urgentes].Hora_s, cadena_hora);
        strcpy(RegistroUrgente[mensajes_urgentes].Nombre_s, Nombre);
        strcpy(RegistroUrgente[mensajes_urgentes].Apellidos_s, Apellidos);
        strcpy(RegistroUrgente[mensajes_urgentes].Direccion_s, Direccion);
        strcpy(RegistroUrgente[mensajes_urgentes].Edad_s, Edad);
        strcpy(RegistroUrgente[mensajes_urgentes].Telefono_s, Telefono);
        strcpy(RegistroUrgente[mensajes_urgentes].Mensaje_s, Mensaje);

        mensajes_urgentes = mensajes_urgentes + 1;  //Se incrementa el n�mero de mensajes urgentes recibidos en 1.
    }
    else{                   // Si el mensaje no es urgente:
        // Copiamos los datos decodificados en strings en nuestra componente, indexada por el n�mero de avisos
        // no urgentes recibidos, del array de structs correspondiente a Registros No Urgentes.
        strcpy(RegistroNoUrgente[mensajes_no_urgentes].Tipo_mensaje_s, cadena_tipo_mensaje);
        strcpy(RegistroNoUrgente[mensajes_no_urgentes].Hora_s, cadena_hora);
        strcpy(RegistroNoUrgente[mensajes_no_urgentes].Nombre_s, Nombre);
        strcpy(RegistroNoUrgente[mensajes_no_urgentes].Apellidos_s, Apellidos);
        strcpy(RegistroNoUrgente[mensajes_no_urgentes].Direccion_s, Direccion);
        strcpy(RegistroNoUrgente[mensajes_no_urgentes].Edad_s, Edad);
        strcpy(RegistroNoUrgente[mensajes_no_urgentes].Telefono_s, Telefono);
        strcpy(RegistroNoUrgente[mensajes_no_urgentes].Mensaje_s, Mensaje);

        mensajes_no_urgentes = mensajes_no_urgentes + 1; //Se incrementa el n�mero de mensajes no urgentes recibidos en 1.
    }

    cambio_estado = 1;  //Flag para entrar en el estado de alerta en la aplicaci�n cada vez que recibimos un aviso nuevo.

    return(Respuesta_Handler_CGI);  //Se devuelve la respuesta al servidor ("/solicitud_cgi.ssi"), recargando la p�gina
}

// Handler para la interrupci�n de SysTick.
void SysTickIntHandler(void)
{
    // Se llama al timer handler de lwIP:
    lwIPTimer(SYSTICKMS);
}


// Mostrar la direcci�n IP.
void DisplayIPAddress(uint32_t Addr)
{
    char pcBuf[16];
    // Convertir la direcci�n IP en una string.
    usprintf(pcBuf, "%d.%d.%d.%d", Addr & 0xff, (Addr >> 8) & 0xff,
            (Addr >> 16) & 0xff, (Addr >> 24) & 0xff);

    // Mostrar la string (direcci�n IP) por la UART.
    if(estado != 10) UARTprintf(pcBuf);

    //Se muestra por pantalla la direcci�n IP a introducir en el navegador para usar la Web
    if (estado == 10){
        ComColor(0,0,0);
        ComTXT(HSIZE/2, VSIZE/2-40, 23, OPT_CENTER,"Introduzca la direccion IP en su navegador:");
        ComColor(255,255,255);
        ComTXT(HSIZE/2, VSIZE/2-10, 23, OPT_CENTER, pcBuf);
    }

}

// Funci�n requerida por la liber�a lwIP para dar soporte a funciones de temporizaci�n (Proceso de obtener la IP):
void lwIPHostTimerHandler(void)
{
    uint32_t Nueva_Direccion_IP;

    // Obtener la direcci�n IP.
    Nueva_Direccion_IP = lwIPLocalIPAddrGet();

    // Comprobar si la direcci�n IP ha cambiado.
    if(Nueva_Direccion_IP != Direccion_IP_actual)
    {
        // Comprobar si hay una direcci�n IP asignada.
        if(Nueva_Direccion_IP == 0xffffffff)
        {
            // Indicar que a�n no hay enlace de conexi�n.
            UARTprintf("Esperando enlace\n");
        }
        else if(Nueva_Direccion_IP == 0)
        {
            // No hay direcci�n IP a�n, indicar que DHCP est� buscando.
            UARTprintf("Esperando direcci�n IP\n");
        }
        else
        {
            // Mostrar la nueva direcci�n IP obtenida.
            UARTprintf("Direcci�n IP: ");
            DisplayIPAddress(Nueva_Direccion_IP);
            UARTprintf("\n");
            UARTprintf("Abra un buscador e inserte la direcci�n IP\n");
            direccion_obtenida = 1; // Se activa esta variable para avanzar del modo "Conectando..." de la aplicaci�n.
        }

        // Guardar la nueva direcci�n IP.
        Direccion_IP_actual = Nueva_Direccion_IP;
    }

    // Si no hay una nueva direcci�n IP:
    if((Nueva_Direccion_IP == 0) || (Nueva_Direccion_IP == 0xffffffff))
    {
        // No hacer nada y seguir esperando.
    }
}

//Prototipos de funciones:
void Funcion_nota1(void);
void Funcion_nota2(void);
void IntTimer0(void);
void IntTimer1(void);
void Pantalla_inicial(void);
void Pantalla_registro(void);
void Pantalla_alerta1(void);
void Pantalla_alerta2(void);
void Pantalla_borrado(void);
void Pantalla_detalle(void);
void Pantalla_Mensaje(void);
void ComProgress(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t options,uint16_t val,uint16_t range);
int map(int valor, int entradaMin, int entradaMax, int salidaMin, int salidaMax);
void MuestraDetalles(int registro, int urgente);
void MuestraMensaje(int urgente, int no_urgente);
void Pantalla_ayuda(void);
void Reset_Cadenas_Mensaje(void);
void Comprueba_Pulsacion_Registros_Urgentes(void);
void Comprueba_Pulsacion_Registros_No_Urgentes(void);

int main(void)
{
    uint32_t User0, User1;      // Variables para configurar la direcci�n MAC del hardware del controlador Ethernet de la placa
    uint8_t pui8MACArray[8];

    // Se requiere que el oscilador principal est� activo, ya que esto es necesario para el PHY (Physical Layer / Circuito integrado)
    // El par�metro "SYSCTL_MOSC_HIGHFREQ" se utiliza cuando la frecuencia del cristal es >= 10 MHz.
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);

    // Se configura el reloj del sistema para que use PLL a 120MHz
    Reloj = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

    ////////////////////////////////////////////////////////HABILITAMOS PUERTOS////////////////////////////////////////////////////////////////
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);    //Para la UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);    //Para el PWM que controla el servomotor
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);     //M�dulo PWM 0

    ///////////////////////////////////////////////////////CONFIGURACI�N DE LA UART/////////////////////////////////////////////////////////////
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);    //Se habilita la UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);                //Pin A0 para recibir
    GPIOPinConfigure(GPIO_PA1_U0TX);                //Pin A1 para enviar
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);  //Pin A0 y A1 para el uso de la UART
    UARTStdioConfig(0, 115200, Reloj);  //A 115200 baudios

    ///////////////////////////////////////////////CONFIGURACI�N DEL PERIF�RICO PWM/////////////////////////////////////////////////////////////
    PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_64);    //Al PWM le llega un reloj de 1.875MHz
    GPIOPinConfigure(GPIO_PG0_M0PWM4);           //Configurar el pin a PWM (m�dulo 0 - generador PWM 2)
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_0); //Pin PG0 para PWM
    // Configurar el PWM 0, contador descendente y sin sincronizaci�n (actualizaci�n autom�tica)
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC); //Se usa el generador PWM 2
    PeriodoPWM=37499;                           // 50Hz a 1.875MHz
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, PeriodoPWM); //frec:50Hz
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT , true);   //Habilita la salida 4, ya que se usar� M0PWM4
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);        //Habilita el generador PWM 2

    //////////////////////////////////// CONFIGURACI�N DE LA PANTALLA //////////////////////////////////////////////////////////
    HAL_Init_SPI(1, Reloj);  //Boosterpack a usar, Velocidad del MC
    Inicia_pantalla();       //Arranca la pantalla
    SysCtlDelay(Reloj/3);    //Peque�o delay para asegurarnos de que la pantalla est� lista

    // Escritura en registros de los valores de calibraci�n de la pantalla de 5.0"
    #ifdef VM800B50
        for(i=0;i<6;i++)    Esc_Reg(REG_TOUCH_TRANSFORM_A+4*i, REG_CAL5[i]);
    #endif

    //////////////////////////////////////////TIMER 0 PARA CONTAR SEGUNDOS (SE USA PARA LAS ANIMACIONES)////////////////////////////////////////
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);         //Se usa el timer 0
    TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM); //T0 a 120MHz
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);      //T0 peri�dico y 32bits
    TimerLoadSet(TIMER0_BASE, TIMER_A, Reloj-1); //Se carga 1 segundo en la cuenta
    TimerIntRegister(TIMER0_BASE,TIMER_A,IntTimer0);      //"IntTimer0" nombre de la rutina de interrupci�n del timer 0
    IntEnable(INT_TIMER0A);                               //Habilita la interrupci�n del timer
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);      //El timeout (fin de la cuenta) provoca interrupci�n en el timer 0

    //////////////////////////////////////////TIMER 1 PARA LA HORA (TIEMPO DE EJECUCI�N DEL PROGRAMA)////////////////////////////////////////////
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);         //Se usa el timer 1
    TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM); //T1 a 120MHz
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);      //T1 peri�dico y 32bits
    TimerLoadSet(TIMER1_BASE, TIMER_A, Reloj-1); //Se carga 1 segundo en la cuenta
    TimerIntRegister(TIMER1_BASE,TIMER_A,IntTimer1);      //"IntTimer1" nombre de la rutina de interrupci�n del timer 1
    IntEnable(INT_TIMER1A);                               //Habilita la interrupci�n del timer
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);      //El timeout (fin de la cuenta) provoca interrupci�n en el timer 1
    TimerEnable(TIMER1_BASE, TIMER_A);                    //Habilita el timer 1 (empieza a contar)
    IntMasterEnable();                                    //Habilita globalmente las interrupciones

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Se configura SysTick para interrupciones peri�dicas
    MAP_SysTickPeriodSet(Reloj / SYSTICKHZ);
    MAP_SysTickEnable();
    MAP_SysTickIntEnable();

    // Configurar la direcci�n MAC hardware para que el controlador Ethernet filtre paquetes entrantes.
    // La direcci�n MAC se guarda en los registros USER0 y USER1.
    MAP_FlashUserGet(&User0, &User1);
    if((User0 == 0xffffffff) || (User1 == 0xffffffff))
    {
        // Hacer saber que no hay direcci�n MAC:
        UARTprintf("Direcci�n MAC no programada\n");

        while(1)
        {
        }
    }

    // Informar de que estamos buscando la IP (por UART):
    UARTprintf("Esperando IP\n");

    // Se convierte la direcci�n MAC separada en fragmentos 24/24 procedente de la Non Volatile RAM, en una
    // direcci�n MAC separada en fragmentos 32/16, que es el formato necesario para programar los registros hardware.
    // Tras ello se programa la direcci�n MAC en los registros del controlador Ethernet.
    pui8MACArray[0] = ((User0 >>  0) & 0xff);
    pui8MACArray[1] = ((User0 >>  8) & 0xff);
    pui8MACArray[2] = ((User0 >> 16) & 0xff);
    pui8MACArray[3] = ((User1 >>  0) & 0xff);
    pui8MACArray[4] = ((User1 >>  8) & 0xff);
    pui8MACArray[5] = ((User1 >> 16) & 0xff);

    // Inicializar la librer�a lwIP, usando DHCP.
    lwIPInit(Reloj, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);

    // Hacer Setup del servicio de Locator para el dispositivo (la placa)
    LocatorInit();
    LocatorMACAddrSet(pui8MACArray);
    LocatorAppTitleSet("EK-TM4C1294XL Servicio_Atencion");

    // Inicializar el servidor httpd (HTTP Apache)
    httpd_init();

    // Setear la prioridad de interrupciones:
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

    // Pasar nuestro CGI handler al servidor HTTP con la configuraci�n necesaria:
    http_set_cgi_handlers(Config_URIs, Numero_de_URIS_configuradas);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Bucle infinito, donde se ejecuta la FSM de la aplicaci�n desarrollada, la interacci�n con la p�gina web
    // se realiza mediante los handlers asociados a las interrupciones correspondientes.

    while(1){
        switch (estado){
        /*********************************************************** ESTADO 0 ***********************************************************/
        case 0: // Pantalla de inicio del programa donde permanecemos hasta que el protocolo DHCP nos proporciona la direcci�n IP de la p�gina web
            while (direccion_obtenida == 0){
                PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, Min_pos);  //Posici�n derecha del servomotor
                Pantalla_inicial();                               //Definimos lo que aparecer� por pantalla
                Dibuja();                                         //Lo dibujamos
            }
            estado = 10;                                           //Una vez tenemos la direcci�n IP, pasamos al estado 10, donde se muestra la IP por pantalla
            break;
        /*********************************************************** ESTADO 1 ***********************************************************/
        case 1: // Pantalla de registro con los avisos
            TimerEnable(TIMER0_BASE, TIMER_A);  //Se habilita el timer que habilitar� la pulsaci�n del aviso 5 urgente
            borrado_urgente = 0;
            borrado_no_urgente = 0;             //Reseteo de ciertas variables
            activa_estado_6 = 0;
            activa_estado_7 = 0;
            Pantalla_registro();                //Definimos lo que aparecer� por pantalla

            Dibuja();                           //Lo dibujamos

            Reset_Cadenas_Mensaje();    //Vaciamos las cadenas que se rellenan para mostrar por pantalla el mensaje del aviso en caso de que tenga

            Lee_pantalla();             //Leemos si se ha pulsado la pantalla y guardamos d�nde se ha pulsado en las variables "POSX" y "POSY"

            // 5 campos de pulsaciones para ver la informaci�n de cada aviso (PARA URGENTE). Se dise�a para que no permita pulsar en cierto campo
            // si no hay mensaje recibido en dicho campo. Seg�n el aviso pulsado, se le da valor a "IndiceUrgente", a "registro" y pasamos al estado 6
            Comprueba_Pulsacion_Registros_Urgentes();

            // 5 campos de pulsaciones para ver la informaci�n de cada aviso (PARA NO URGENTE). Se dise�a para que no permita pulsar en cierto campo
            // si no hay mensaje recibido en dicho campo. Seg�n el aviso pulsado, se le da valor a "IndiceNoUrgente", a "registro" y pasamos al estado 7
            Comprueba_Pulsacion_Registros_No_Urgentes();

            if(activa_estado_1 == 2)  TimerDisable(TIMER0_BASE, TIMER_A);  //Pasados dos segundos, habilitamos la pulsaci�n del aviso 5 del tipo Urgente

            if (cambio_estado == 1){
                contador_estado2 = 0;   //Si nos llega un nuevo aviso procedente de la p�gina web, pasamos al estado 2
                estado = 2;
            }
            break;

        /*********************************************************** ESTADO 2 ***********************************************************/
        case 2: // Pantalla de alerta de aviso entrante
            TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitamos el timer 0 para realizar las animaciones de este estado
            cambio_estado = 0;                  //Reseteamos
            Funcion_nota1();                    //Se llama a la primera funci�n que har� que la pantalla emita sonido
            if (flag_estado_alerta == 0){
                Pantalla_alerta1();
                Dibuja();                       //Se alterna entre dos colores de fondo de pantalla mostrando lo mismo por pantalla
            }
            if (flag_estado_alerta == 1){
                Pantalla_alerta2();
                Dibuja();
            }
            if (contador_estado2 == 4){         //Cuando hemos pasado 4 segundos en este estado, volvemos al estado 1 (pantalla de registro)
                flag_estado_alerta = 0;         //Reseteamos "flag_estado_alerta"
                FinNota();                      //Dejamos de emitir sonido
                TimerDisable(TIMER0_BASE, TIMER_A); //Se deshabilita el timer
                estado = 1;
            }
            break;

        /*********************************************************** ESTADO 3 ***********************************************************/
        case 3: // Estado para borrar un aviso urgente cuando se pulsa en el estado 9 el bot�n "Enviar ayuda".
            // Adicionalmente se desplazan todos los avisos posteriores (avisos que llegaron luego al que se borra) una posici�n hacia arriba en el array de structs de avisos urgentes
            for(i = IndiceUrgente; i < mensajes_urgentes - 1; i++){
                strcpy(RegistroUrgente[i].Tipo_mensaje_s, RegistroUrgente[i+1].Tipo_mensaje_s);
                strcpy(RegistroUrgente[i].Nombre_s,       RegistroUrgente[i+1].Nombre_s);
                strcpy(RegistroUrgente[i].Apellidos_s,    RegistroUrgente[i+1].Apellidos_s);
                strcpy(RegistroUrgente[i].Direccion_s,    RegistroUrgente[i+1].Direccion_s);
                strcpy(RegistroUrgente[i].Mensaje_s,      RegistroUrgente[i+1].Mensaje_s);
                strcpy(RegistroUrgente[i].Hora_s,         RegistroUrgente[i+1].Hora_s);
                strcpy(RegistroUrgente[i].Edad_s,         RegistroUrgente[i+1].Edad_s);
                strcpy(RegistroUrgente[i].Telefono_s,     RegistroUrgente[i+1].Telefono_s);
            }

            // Si el n�mero de avisos urgentes es inferior a 5 (no hay encolados) y se borra uno, borramos el �ltimo para que aparezca en blanco en el registro
            if(mensajes_urgentes <= 5){
                memset(RegistroUrgente[mensajes_urgentes-1].Tipo_mensaje_s, 0, 50);
                memset(RegistroUrgente[mensajes_urgentes-1].Nombre_s,       0, 50);
                memset(RegistroUrgente[mensajes_urgentes-1].Apellidos_s,    0, 50);
                memset(RegistroUrgente[mensajes_urgentes-1].Direccion_s,    0, 50);
                memset(RegistroUrgente[mensajes_urgentes-1].Mensaje_s,      0, 50);
                memset(RegistroUrgente[mensajes_urgentes-1].Hora_s,         0, 50);
                memset(RegistroUrgente[mensajes_urgentes-1].Edad_s,         0, 50);
                memset(RegistroUrgente[mensajes_urgentes-1].Telefono_s,     0, 50);
            }

            mensajes_urgentes = mensajes_urgentes - 1;  //Decrementamos el n�mero de mensajes urgentes
            contador_borrando = 0;                      //Se resetea "contador_borrando" que permite realizar la animaci�n de la pantalla del estado 5
            estado = 1;                                 //Se vuelve al registro
            break;

        /*********************************************************** ESTADO 4 ***********************************************************/
        case 4: // Estado para borrar un aviso no urgente cuando se pulsa en el estado 9 el bot�n "Enviar ayuda".
            // Adicionalmente se desplazan todos los avisos posteriores (avisos que llegaron luego al que se borra) una posici�n hacia arriba en el array de structs de avisos no urgentes
            for(i = IndiceNoUrgente; i < mensajes_no_urgentes - 1; i++){
                strcpy(RegistroNoUrgente[i].Tipo_mensaje_s, RegistroNoUrgente[i+1].Tipo_mensaje_s);
                strcpy(RegistroNoUrgente[i].Nombre_s,       RegistroNoUrgente[i+1].Nombre_s);
                strcpy(RegistroNoUrgente[i].Apellidos_s,    RegistroNoUrgente[i+1].Apellidos_s);
                strcpy(RegistroNoUrgente[i].Direccion_s,    RegistroNoUrgente[i+1].Direccion_s);
                strcpy(RegistroNoUrgente[i].Mensaje_s,      RegistroNoUrgente[i+1].Mensaje_s);
                strcpy(RegistroNoUrgente[i].Hora_s,         RegistroNoUrgente[i+1].Hora_s);
                strcpy(RegistroNoUrgente[i].Edad_s,         RegistroNoUrgente[i+1].Edad_s);
                strcpy(RegistroNoUrgente[i].Telefono_s,     RegistroNoUrgente[i+1].Telefono_s);
            }

            // Si el n�mero de avisos urgentes es inferior a 5 (no hay encolados) y se borra uno, borramos el �ltimo para que aparezca en blanco en el registro
            if(mensajes_no_urgentes <= 5){
                memset(RegistroNoUrgente[mensajes_no_urgentes-1].Tipo_mensaje_s, 0, 50);
                memset(RegistroNoUrgente[mensajes_no_urgentes-1].Nombre_s,       0, 50);
                memset(RegistroNoUrgente[mensajes_no_urgentes-1].Apellidos_s,    0, 50);
                memset(RegistroNoUrgente[mensajes_no_urgentes-1].Direccion_s,    0, 50);
                memset(RegistroNoUrgente[mensajes_no_urgentes-1].Mensaje_s,      0, 50);
                memset(RegistroNoUrgente[mensajes_no_urgentes-1].Hora_s,         0, 50);
                memset(RegistroNoUrgente[mensajes_no_urgentes-1].Edad_s,         0, 50);
                memset(RegistroNoUrgente[mensajes_no_urgentes-1].Telefono_s,     0, 50);
            }

            mensajes_no_urgentes = mensajes_no_urgentes - 1;    //Decrementamos el n�mero de mensajes no urgentes
            contador_borrando = 0;                              //Se resetea "contador_borrando" que permite realizar la animaci�n de la pantalla del estado 5
            estado = 1;                                         //Se vuelve al registro
            break;

        /*********************************************************** ESTADO 5 ***********************************************************/
        case 5: // Pantalla animada que indica que se est� borrando el aviso para el que se acaba de enviar la ayuda
            TimerEnable(TIMER0_BASE, TIMER_A);      //Habilitamos el timer 0 para realizar las animaciones de este estado
            Pantalla_borrado();                     //Definimos lo que aparecer� por pantalla
            Dibuja();                               //Lo dibujamos
            if (contador_borrando == 3){            //Pasados 3 segundos desde que entramos en este estado, vamos al estado 3 si el aviso que hemos atendido era urgente, o al estado 4 si era no urgente
                TimerDisable(TIMER0_BASE, TIMER_A); //Deshabilitamos el timer
                contador_borrando = 0;              //Reseteamos "contador_borrando"
                if(borrado_urgente == 1){
                    estado = 3;
                }
                else if(borrado_no_urgente == 1){
                    estado = 4;
                }
                else {}
            }
            break;

        /*********************************************************** ESTADO 6 ***********************************************************/
        case 6: // Estado en el que se muestran los detalles de un aviso urgente pulsado en el registro (nombre, apellidos, edad...)
            TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitamos el timer 0 para habilitar la pulsaci�n de los botones de esta pantalla pasados dos segundos
            ver_mensaje_urgente = 0;            //Reseteo de variables
            volver_mensaje_urgente = 0;
            Pantalla_detalle();                 //Definimos lo que aparecer� por pantalla
            MuestraDetalles(registro,1);        //Llamamos a la funci�n "MuestraDetalles" que define la pantalla que contiene toda la informaci�n del aviso a excepci�n del mensaje

            if(Boton_volver && activa_estado_6 >= 2){   //Si pulsamos el bot�n "Volver", nos devuelve al registro
                activa_estado_1 = 0;                    //Reseteamos "activa_estado_1"
                estado = 1;                             //Volvemos a la pantalla de registros
            }

            if(Boton_ver_mensaje && activa_estado_6 >= 2){
                borrado_urgente = 1;
                volver_mensaje_urgente = 1;
                ver_mensaje_urgente = 1;
                flag_primera_linea = 0;                 //Activamos y reseteamos variables que ser�n de utilidad en el estado
                flag_segunda_linea = 0;                 //en el que mostramos el mensaje del aviso urgente en caso de tener
                flag_tercera_linea = 0;
                flag_cuarta_linea = 0;
                flag_quinta_linea = 0;
                i = 0;
                k = 0;
                activa_estado_9 = 0;
                terminador = 0;
                estado = 9;                             //Si pulsamos el bot�n "Ver mensaje" pasamos al estado 9
            }

            if(activa_estado_6 == 2) TimerDisable(TIMER0_BASE, TIMER_A);    //Pasados dos segundos en este estado desactivamos el timer

            Dibuja();                                   //Dibujamos la pantalla
            break;

        /*********************************************************** ESTADO 7 ***********************************************************/
        case 7: // Estado en el que se muestran los detalles de un aviso no urgente pulsado en el registro (nombre, apellidos, edad...)
            TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitamos el timer 0 para habilitar la pulsaci�n de los botones de esta pantalla pasados dos segundos
            ver_mensaje_no_urgente = 0;         //Reseteo de variables
            volver_mensaje_no_urgente = 0;
            Pantalla_detalle();                 //Definimos lo que aparecer� por pantalla
            MuestraDetalles(registro,0);        //Llamamos a la funci�n "MuestraDetalles" que define la pantalla que contiene toda la informaci�n del aviso a excepci�n del mensaje

            if(Boton_volver && activa_estado_7 >= 2){   //Si pulsamos el bot�n "Volver", nos devuelve al registro
                activa_estado_1 = 0;                    //Reseteamos "activa_estado_1"
                estado = 1;                             //Volvemos a la pantalla de registros
            }

            if(Boton_ver_mensaje && activa_estado_7 >= 2){
                borrado_no_urgente = 1;
                volver_mensaje_no_urgente = 1;
                ver_mensaje_no_urgente = 1;
                flag_primera_linea = 0;                 //Activamos y reseteamos variables que ser�n de utilidad en el estado
                flag_segunda_linea = 0;                 //en el que mostramos el mensaje del aviso no urgente en caso de tener
                flag_tercera_linea = 0;
                flag_cuarta_linea = 0;
                flag_quinta_linea = 0;
                i = 0;
                k = 0;
                activa_estado_9 = 0;
                terminador = 0;
                estado = 9;                             //Si pulsamos el bot�n "Ver mensaje" pasamos al estado 9
            }

            if(activa_estado_7 == 2) TimerDisable(TIMER0_BASE, TIMER_A);    //Pasados dos segundos en este estado desactivamos el timer

            Dibuja();                                   //Dibujamos la pantalla
            break;

        /*********************************************************** ESTADO 8 ***********************************************************/
        case 8: // Este estado emula el env�o de ayuda al lugar que indica el aviso atendido. Puede ser por ejemplo la apertura de una compuerta que permite salir a la ambulancia ante un aviso de una persona a la que le ha dado un infarto
            TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitamos el timer 0 para realizar las animaciones de este estado
            Funcion_nota2();                    //Se llama a la segunda funci�n que har� que la pantalla emita sonido
            if (contador_estado8 == 0){         //Al entrar en este estado movemos el servomotor a la posici�n izquierda
                PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, Max_pos);
                Pantalla_ayuda();               //Definimos lo que aparecer� por pantalla (animaci�n de un cochecito movi�ndose)
                Dibuja();                       //Dibujamos la pantalla
            }
            if (contador_estado8 == 1){
                Pantalla_ayuda();               //A cada segundo que pasa, el cochecito avanza hacia la derecha de la pantalla
                Dibuja();
            }
            if (contador_estado8 == 2){
                Pantalla_ayuda();
                Dibuja();
            }
            if (contador_estado8 == 3){
                Pantalla_ayuda();
                Dibuja();
            }
            if (contador_estado8 == 4){                             //Pasados 4 segundos
                PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, Min_pos);    //Llevamos el servomotor a su posici�n de reposo
                FinNota();                                          //La pantalla deja de emitir sonido
                TimerDisable(TIMER0_BASE, TIMER_A);                 //Deshabilitamos el timer 0
                contador_borrando = 0;                              //Reseteamos "contador_borrando"
                estado = 5;                                         //Pasamos al estado donde se anima el borrado del aviso atendido
            }
            break;

        /*********************************************************** ESTADO 9 ***********************************************************/
        case 9: //Estado en el que se muestra el mensaje del aviso en caso de contener
            TimerEnable(TIMER0_BASE, TIMER_A);          //Habilitamos el timer 0 para habilitar la pulsaci�n de los botones de esta pantalla pasados dos segundos
            Pantalla_Mensaje();                         //Definimos lo que aparecer� por pantalla

            //Llamamos a la funci�n "MuestraMensaje", esta define el valor de las 5 cadenas que imprimen
            //el mensaje ("cadena_primera_linea, "cadena_segunda_linea"...) y las saca por la pantalla
            MuestraMensaje(ver_mensaje_urgente, ver_mensaje_no_urgente);

            if(Boton_volver && activa_estado_9 >= 2){       //Al pulsar el bot�n "Volver", seg�n qu� tipo de aviso est�bamos viendo
                if (volver_mensaje_urgente == 1){           //volvemos al estado 6 (caso urgente) o al estado 7 (caso no urgente)
                    estado = 6;
                }
                else if (volver_mensaje_no_urgente == 1) {
                    estado = 7;
                }
            }

            if(Boton_enviar_ayuda && activa_estado_9 >= 2){ //Si pulsamos el bot�n "Enviar ayuda" pasamos al estado 8 de env�o de ayuda
                contador_estado8 = 0;                       //Reseteamos "contador_estado8"
                estado = 8;
            }

            if(activa_estado_9 == 2){                       //Pasados 2 segundos
                activa_estado_6 = 0;                        //Reseteamos variables
                activa_estado_7 = 0;
                TimerDisable(TIMER0_BASE, TIMER_A);         //Desactivamos el timer 0
            }

            Dibuja();                                       //Dibujamos la pantalla
            break;
        /*********************************************************** ESTADO 10 ***********************************************************/
        case 10:    // Estado que muestra la direcci�n IP a introducir en el navegador para que no sea necesario a acudir a la UART para verla
            Nueva_pantalla(51,222,204);
            DisplayIPAddress(Direccion_IP_actual);  // Se pinta la direcci�n IP en la pantalla
            if(Boton_IP_introducida){               // Una vez introducida la IP en el navegador, se pulsa este bot�n para hacer uso de la aplicaci�n de manera habitual.
                estado = 1;
            }
            Dibuja();
            break;
        }
    }
}


/////////////////////////////////////////////////////////////// FUNCIONES ///////////////////////////////////////////////////////////////////////

// Funci�n para emitir sonido en la pantalla de alerta (estado 2)
void Funcion_nota1(){
    VolNota(100);
    TocaNota(S_BEEP, N_SI);
}

// Funci�n para emitir sonido en la pantalla de env�o de ayuda (estado 8)
void Funcion_nota2(){
    VolNota(30);
    TocaNota(S_XILO, N_SI);
}

void IntTimer0(void) //Cuenta segundos al entrar en los estados 1, 2, 5, 6, 7, 8 y 9
{
   TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Borra flag

   contador_estado2 = contador_estado2 + 1;
   contador_borrando = contador_borrando + 1;
   contador_estado8 = contador_estado8 + 1;
   activa_estado_9 = activa_estado_9 + 1;       //Se incrementan las variables necesarias en una unidad
   activa_estado_6 = activa_estado_6 + 1;
   activa_estado_7 = activa_estado_7 + 1;
   activa_estado_1 = activa_estado_1 + 1;

   flag_estado_alerta = !flag_estado_alerta;    //Se conmuta el valor de "flag_estado_alerta" entre '0' y '1' para la pantalla de alerta (estado 2)
}

void IntTimer1(void) //Cuenta el tiempo de ejecuci�n del programa desde su arranque
{
   TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Borra flag

   cont_circulos = cont_circulos + 1;              // Variable que anima los puntitos de la pantalla inicial (estado 0)

   if (cont_circulos == 4) cont_circulos = 0;      // Si toma valor 4, se resetea

   segundos++;                                     // Incrementa variables de segundos en 1
  if(segundos == 60)                               // Si se llega a 60 segundos:
  {
      minutos++;                                   // Se incrementa minutos en 1
      segundos = 0;                                // Se resetea segundos a 0
  }
  if(minutos == 60)                                // Si se llega a 60 minutos:
  {
      horas++;                                     // Se incrementa horas en 1
      minutos = 0;                                 // Se resetea minutos a 0
  }
  if(horas == 100)                                 // Si horas llega a 100 (parece sensato contar m�s de 24h por si el dispositivo se deja encendido)
  {
      horas = 0;                                   // Se resetea horas a 0
  }

  sprintf(cadena_hora, "%02d:%02d:%02d",horas, minutos, segundos); // Se define la cadena que se usar� para rellenar el campo "Hora_s" conforme llegan los avisos
}

// Funci�n que define la pantalla inicial (estado 0)
void Pantalla_inicial(void)
{
    Nueva_pantalla(31,58,208);
    ComColor(255,255,255);
    ComTXT(HSIZE/2, 2, 24, OPT_CENTERX,"Atencion de personas");
    ComTXT(HSIZE/2, 27, 24, OPT_CENTERX,"con necesidades especiales");

    ComCirculo(HSIZE/2, 144, 67);

    ComColor(255, 0, 0);
    ComRect(HSIZE/2-50, 129, HSIZE/2+50, 161, true);
    ComRect(HSIZE/2-15, 94, HSIZE/2+15, 196, true);

    ComColor(0, 0, 0);
    ComTXT(HSIZE/2-38, VSIZE/2+10, 24, OPT_CENTER,"F");
    ComTXT(HSIZE/2+38, VSIZE/2+10, 24, OPT_CENTER,"J");
    ComTXT(HSIZE/2, 105, 24, OPT_CENTER,"J");
    ComTXT(HSIZE/2, 181, 24, OPT_CENTER,"D");

    ComColor(255, 255, 255);
    ComTXT(HSIZE/2-25, 225, 24, OPT_CENTERX,"Conectando");

    if (cont_circulos == 1) ComCirculo(HSIZE/2+50, 244, 5); //Seg�n el valor de "cont_c�rculos", los puntitos aparecen y desaparecen
    else if (cont_circulos == 2){
        ComCirculo(HSIZE/2+50, 244, 5);
        ComCirculo(HSIZE/2+65, 244, 5);
    }
    else if (cont_circulos == 3){
        ComCirculo(HSIZE/2+50, 244, 5);
        ComCirculo(HSIZE/2+65, 244, 5);
        ComCirculo(HSIZE/2+80, 244, 5);
    }
    else {}
}

// Funci�n que define la pantalla del registro (estado 1) con los 5 primeros avisos de cada tipo, tanto urgentes como no urgentes
void Pantalla_registro(void)
{
    Nueva_pantalla(51,222,204);

    ComColor(0,0,0);
    ComTXT(7*HSIZE/8, VSIZE/20, 23, OPT_CENTER, cadena_hora);
    ComTXT(HSIZE/13, VSIZE/20, 23, OPT_CENTERY, "Registro de mensajes");

    ComColor(255,0,0);
    ComTXT(HSIZE/4+15, VSIZE/6, 23, OPT_CENTER, "Urgente");
    ComTXT(HSIZE/6-20, VSIZE/4+9, 23, OPT_CENTER, "N");
    ComColor(59,32,103);
    ComTXT(3*HSIZE/9-1, VSIZE/4+9, 23, OPT_CENTER, "Hora llegada");

    ComColor(255,255,255);
    ComTXT(HSIZE/6-20, 2*VSIZE/5+2, 23, OPT_CENTER, "1"); //(59,98)
    ComTXT(3*HSIZE/9-1, 2*VSIZE/5+2, 23, OPT_CENTER, RegistroUrgente[0].Hora_s);
    ComTXT(HSIZE/6-20, VSIZE/2+5, 23, OPT_CENTER, "2");
    ComTXT(3*HSIZE/9-1, VSIZE/2+5, 23, OPT_CENTER, RegistroUrgente[1].Hora_s);
    ComTXT(HSIZE/6-20, 4*VSIZE/6-7, 23, OPT_CENTER, "3");
    ComTXT(3*HSIZE/9-1, 4*VSIZE/6-7, 23, OPT_CENTER, RegistroUrgente[2].Hora_s);
    ComTXT(HSIZE/6-20, 3*VSIZE/4+2, 23, OPT_CENTER, "4");
    ComTXT(3*HSIZE/9-1, 3*VSIZE/4+2, 23, OPT_CENTER, RegistroUrgente[3].Hora_s);
    ComTXT(HSIZE/6-20, 3*VSIZE/4+32, 23, OPT_CENTER, "5");
    ComTXT(3*HSIZE/9-1, 3*VSIZE/4+32, 23, OPT_CENTER, RegistroUrgente[4].Hora_s);

    ComColor(255,197,0);
    ComTXT(3*HSIZE/4-55, VSIZE/6, 23, OPT_CENTERY, "No urgente");
    ComTXT(HSIZE/2+20, VSIZE/4+9, 23, OPT_CENTER, "N");
    ComColor(59,32,103);
    ComTXT(7*HSIZE/9-6, VSIZE/4+9, 23, OPT_CENTER, "Hora llegada");

    ComColor(255,255,255);
    ComTXT(HSIZE/2+20, 2*VSIZE/5+2, 23, OPT_CENTER, "1");
    ComTXT(7*HSIZE/9-6, 2*VSIZE/5+2, 23, OPT_CENTER, RegistroNoUrgente[0].Hora_s);
    ComTXT(HSIZE/2+20, VSIZE/2+5, 23, OPT_CENTER, "2");
    ComTXT(7*HSIZE/9-6, VSIZE/2+5, 23, OPT_CENTER, RegistroNoUrgente[1].Hora_s);
    ComTXT(HSIZE/2+20, 4*VSIZE/6-7, 23, OPT_CENTER, "3");
    ComTXT(7*HSIZE/9-6, 4*VSIZE/6-7, 23, OPT_CENTER, RegistroNoUrgente[2].Hora_s);
    ComTXT(HSIZE/2+20, 3*VSIZE/4+2, 23, OPT_CENTER, "4");
    ComTXT(7*HSIZE/9-6, 3*VSIZE/4+2, 23, OPT_CENTER, RegistroNoUrgente[3].Hora_s);
    ComTXT(HSIZE/2+20, 3*VSIZE/4+32, 23, OPT_CENTER, "5");
    ComTXT(7*HSIZE/9-6, 3*VSIZE/4+32, 23, OPT_CENTER, RegistroNoUrgente[4].Hora_s);

    ComColor(0,0,255);
    ComRect(HSIZE/13, VSIZE/9, HSIZE-23, 8*VSIZE/10+36, false);
    ComLine(HSIZE/2, VSIZE/9, HSIZE/2, 8*VSIZE/10+36, 1);
    ComRect(HSIZE/13, VSIZE/5+7, HSIZE-23, VSIZE/3+3, false);
    ComRect(HSIZE/13, VSIZE/2-11, HSIZE-23, VSIZE/2+21, false);
    ComRect(HSIZE/13, 7*VSIZE/10-1, HSIZE-23, 8*VSIZE/10+3, false);
    ComRect(HSIZE/6+2, VSIZE/5+7, 4*HSIZE/7+7, 8*VSIZE/10+36, false);
    ComRect(HSIZE/6+2, 8*VSIZE/10+3, 4*HSIZE/7+7, 8*VSIZE/10+36, false);
}

// Funci�n que define la primera pantalla del estado de alerta (estado 2)
void Pantalla_alerta1(void)
{
    Nueva_pantalla(255,0,255);
    ComColor(0,0,0);
    ComTXT(HSIZE/2, 25, 25, OPT_CENTERX,"!SOLICITUD DE");
    ComTXT(HSIZE/2, 60, 25, OPT_CENTERX,"AYUDA");
    ComTXT(HSIZE/2, 95, 25, OPT_CENTERX,"ENTRANTE!");
    ComColor(255,255,255);
    ComCirculo(HSIZE/2-25, 170, 10);
    ComRect(HSIZE/2-30, 190, HSIZE/2-20, 230, 1);
    ComRect(HSIZE/2+10, 160, HSIZE/2+20, 200, 1);
    ComCirculo(HSIZE/2+15, 220, 10);
}

// Funci�n que define la segunda pantalla del estado de alerta (estado 2), tan solo var�a el color del fondo con respecto a la primera pantalla
void Pantalla_alerta2(void)
{
    Nueva_pantalla(57,208,31);
    ComColor(0,0,0);
    ComTXT(HSIZE/2, 25, 25, OPT_CENTERX,"!SOLICITUD DE");
    ComTXT(HSIZE/2, 60, 25, OPT_CENTERX,"AYUDA");
    ComTXT(HSIZE/2, 95, 25, OPT_CENTERX,"ENTRANTE!");
    ComColor(255,255,255);
    ComCirculo(HSIZE/2-25, 170, 10);
    ComRect(HSIZE/2-30, 190, HSIZE/2-20, 230, 1);
    ComRect(HSIZE/2+10, 160, HSIZE/2+20, 200, 1);
    ComCirculo(HSIZE/2+15, 220, 10);
}

// Funci�n que define la pantalla del estado de borrado (estado 5)
void Pantalla_borrado(void)
{
    Nueva_pantalla(31,58,208);
    ComColor(255, 255, 255);
    ComTXT(HSIZE/2-30, 205, 24, OPT_CENTERX,"Borrando registro");

    ComCirculo(HSIZE/2+80, 224, 5);
    ComCirculo(HSIZE/2+95, 224, 5);
    ComCirculo(HSIZE/2+110, 224, 5);

    //Barra de progreso de borrando. Se rellena (1/3) cada segundo
    ComProgress(HSIZE/6,VSIZE/2,300,30,OPT_FLAT,map(contador_borrando, 0, 3, 0, 100), 100);
}

// Funci�n que define el color del fondo y el t�tulo en los estados de los detalles del mensaje (estado 6/7)
void Pantalla_detalle(void)
{
    Nueva_pantalla(51,222,204);
    ComColor(255, 255, 255);
    ComTXT(HSIZE/2+10, VSIZE/9, 24, OPT_CENTER,"Detalles del mensaje");
}

// Funci�n que define el color del fondo y el t�tulo en el estado en el que se muestra el mensaje del aviso consultado (estado 9)
void Pantalla_Mensaje(void)
{
    Nueva_pantalla(51,222,204);
    ComColor(0, 0, 0);
    ComTXT(HSIZE/2, VSIZE/9, 24, OPT_CENTER,"Mensaje adjunto:");
}

// Funci�n que define la barra de progreso del estado de borrado (estado 5)
void ComProgress(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t options,uint16_t val,uint16_t range)
{
    EscribeRam32(CMD_PROGRESS);
    EscribeRam16(x);
    EscribeRam16(y);
    EscribeRam16(w);            //8 parametros de 16bits, 16 bytes: BIEN
    EscribeRam16(h);            //si fuesen 7 parametros de 16 bits, hay que mandar un ultimo EscribeRam16 con ceros.
    EscribeRam16(options);      //En este caso s� hay que mandarle los ceros al final
    EscribeRam16(val);
    EscribeRam16(range);
    EscribeRam16(0);
}

// Funci�n de mapeo
int map(int valor, int entradaMin, int entradaMax, int salidaMin, int salidaMax)   //Por las caracter�sticas de la progress bar de "borrando..." se puede mapear con enteros
{
    return ((((valor-entradaMin)*(salidaMax-salidaMin))/(entradaMax-entradaMin))+salidaMin);
}

// Funci�n que muestra todos los campos de un aviso a excepci�n del mensaje
void MuestraDetalles(int registro, int urgente)
{
    // Conociendo el valor de "urgente" y "registro" se sabe de qu� aviso de los 10 posibles a consultar debemos imprimir su informaci�n
    if(urgente == 1){
        ComColor(0,0,0);
        ComTXT(HSIZE/20, 2*VSIZE/9, 22, OPT_CENTERY, "Tipo de mensaje: ");
        ComTXT(HSIZE/20, 3*VSIZE/9, 22, OPT_CENTERY, "Hora: ");
        ComTXT(HSIZE/20, 4*VSIZE/9, 22, OPT_CENTERY, "Nombre: ");
        ComTXT(HSIZE/20, 5*VSIZE/9, 22, OPT_CENTERY, "Apellidos: ");
        ComTXT(HSIZE/20, 6*VSIZE/9, 22, OPT_CENTERY, "Direccion: ");
        ComTXT(HSIZE/20, 7*VSIZE/9, 22, OPT_CENTERY, "Edad: ");
        ComTXT(HSIZE/2, 7*VSIZE/9, 22, OPT_CENTERY, "Telefono: ");
        ComColor(255,255,255);
        ComTXT(HSIZE/20+135, 2*VSIZE/9, 22, OPT_CENTERY,RegistroUrgente[registro-1].Tipo_mensaje_s);
        ComTXT(HSIZE/20+45, 3*VSIZE/9, 22, OPT_CENTERY,RegistroUrgente[registro-1].Hora_s);
        ComTXT(HSIZE/20+67, 4*VSIZE/9, 22, OPT_CENTERY,RegistroUrgente[registro-1].Nombre_s);
        ComTXT(HSIZE/20+73, 5*VSIZE/9, 22, OPT_CENTERY,RegistroUrgente[registro-1].Apellidos_s);
        ComTXT(HSIZE/20+75, 6*VSIZE/9, 22, OPT_CENTERY,RegistroUrgente[registro-1].Direccion_s);
        ComTXT(HSIZE/20+47, 7*VSIZE/9, 22, OPT_CENTERY,RegistroUrgente[registro-1].Edad_s);
        ComTXT(HSIZE/2+72, 7*VSIZE/9, 22, OPT_CENTERY,RegistroUrgente[registro-1].Telefono_s);
    }
    else{
        ComColor(0,0,0);
        ComTXT(HSIZE/20, 2*VSIZE/9, 22, OPT_CENTERY, "Tipo de mensaje: ");
        ComTXT(HSIZE/20, 3*VSIZE/9, 22, OPT_CENTERY, "Hora: ");
        ComTXT(HSIZE/20, 4*VSIZE/9, 22, OPT_CENTERY, "Nombre: ");
        ComTXT(HSIZE/20, 5*VSIZE/9, 22, OPT_CENTERY, "Apellidos: ");
        ComTXT(HSIZE/20, 6*VSIZE/9, 22, OPT_CENTERY, "Direccion: ");
        ComTXT(HSIZE/20, 7*VSIZE/9, 22, OPT_CENTERY, "Edad: ");
        ComTXT(HSIZE/2, 7*VSIZE/9, 22, OPT_CENTERY, "Telefono: ");
        ComColor(255,255,255);
        ComTXT(HSIZE/20+135, 2*VSIZE/9, 22, OPT_CENTERY,RegistroNoUrgente[registro-1].Tipo_mensaje_s);
        ComTXT(HSIZE/20+45, 3*VSIZE/9, 22, OPT_CENTERY,RegistroNoUrgente[registro-1].Hora_s);
        ComTXT(HSIZE/20+67, 4*VSIZE/9, 22, OPT_CENTERY,RegistroNoUrgente[registro-1].Nombre_s);
        ComTXT(HSIZE/20+73, 5*VSIZE/9, 22, OPT_CENTERY,RegistroNoUrgente[registro-1].Apellidos_s);
        ComTXT(HSIZE/20+75, 6*VSIZE/9, 22, OPT_CENTERY,RegistroNoUrgente[registro-1].Direccion_s);
        ComTXT(HSIZE/20+47, 7*VSIZE/9, 22, OPT_CENTERY,RegistroNoUrgente[registro-1].Edad_s);
        ComTXT(HSIZE/2+72, 7*VSIZE/9, 22, OPT_CENTERY,RegistroNoUrgente[registro-1].Telefono_s);
    }
}

// Funci�n que se encarga de separar el mensaje del aviso de tama�o 200, en 5 cadenas para se pueda visualizar de
// manera correcta por la pantalla sin que haya texto que sobrepase los l�mites de la pantalla en el estado 9
void MuestraMensaje(int ver_urgente, int ver_no_urgente){

    //Sabiendo el valor de "registro" y sabiendo si venimos del estado 6 (aviso urgente) o
    //del 7 (aviso no urgente) guardamos en "cadena_mensaje" el mensaje correspondiente
    if(ver_urgente == 1){
        strcpy(cadena_mensaje, RegistroUrgente[registro-1].Mensaje_s);
    }

    if(ver_no_urgente == 1){
        strcpy(cadena_mensaje, RegistroNoUrgente[registro-1].Mensaje_s);
    }

    //Si el primer car�cter del mensaje es una letra, significa que no est� vac�o el mensaje
    if ((cadena_mensaje[0]>='a'&& cadena_mensaje[0]<='z') || (cadena_mensaje[0]>='A' && cadena_mensaje[0]<='Z')){
        //B�squeda de la longitud de la cadena del mensaje env�ado (se busca el terminador)
        while (i <= 199){
            if (cadena_mensaje[i] == '\0' && terminador == 0) {
                terminador = i;     //Se guarda el valor del termindor del mensaje en "terminador"
                break;
            }
            i = i + 1;
        }

        //Bucle desde el comienzo de la cadena hasta el terminador
        while(k <= terminador){

            /************************************************* RENGL�N 1 ***********************************************************/
            //Se separa el primer rengl�n del mensaje entrante que cabe en la pantalla (en torno al
            //car�cter 40 se llega al lateral derecho de la pantalla con un cierto margen de seguridad)
            if(terminador >= 0 && terminador <= 40){             //Si el mensaje cabe en la primera l�nea
                for (j = 0; j<=terminador; j++){
                    cadena_primera_linea[j] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_primera_linea"
                }
            }
            else{                                                //Si el mensaje no cabe en la primera l�nea
                if(k >= 40 && cadena_mensaje[k] == ' ' && flag_primera_linea == 0){
                    for (j = 0; j<=k; j++){
                        cadena_primera_linea[j] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_primera_linea" hasta el primer
                    }                                                //espacio encontrado del car�cter n�mero 40 en adelante
                    caracter = k;              //Almacenamos en "caracter" el �ndice desde el que debemos empezar a guardar en la siguiente l�nea
                    flag_primera_linea = 1;    //Activamos "flag_primera_linea" para realizar el almacenamiento del mensaje de manera secuencial
                }
            }

            /************************************************* RENGL�N 2 ***********************************************************/
            //Se separa el segundo rengl�n del mensaje entrante que cabe en la pantalla (en torno al
            //car�cter 80 se llega al lateral derecho de la pantalla con un cierto margen de seguridad)
            if(flag_primera_linea == 1){    //Esperamos a que se hayan realizado las operaciones necesarias para rellenar la primera l�nea
                if(terminador > 40 && terminador <= 80){        //Si el mensaje termina en la segunda l�nea
                    for (j = caracter+1; j<=terminador; j++){
                        cadena_segunda_linea[j-caracter-1] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_segunda_linea"
                    }
                }
                else{                                            //Si el mensaje no termina en la segunda l�nea
                    if(k >= 80 && cadena_mensaje[k] == ' ' && flag_segunda_linea == 0){
                        for (j = caracter+1; j<=k; j++){
                            cadena_segunda_linea[j-caracter-1] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_segunda_linea" hasta el primer
                        }                                                           //espacio encontrado del car�cter n�mero 80 en adelante
                        caracter = k;           //Almacenamos en "caracter" el �ndice desde el que debemos empezar a guardar en la siguiente l�nea
                        flag_segunda_linea = 1; //Activamos "flag_segunda_linea" para realizar el almacenamiento del mensaje de manera secuencial
                    }
                }
            }

            /************************************************* RENGL�N 3 ***********************************************************/
            //Se separa el tercer rengl�n del mensaje entrante que cabe en la pantalla (en torno al
            //car�cter 120 se llega al lateral derecho de la pantalla con un cierto margen de seguridad)
            if(flag_segunda_linea == 1){    //Esperamos a que se hayan realizado las operaciones necesarias para rellenar la segunda l�nea
                if(terminador > 80 && terminador <= 120){       //Si el mensaje termina en la tercera l�nea
                    for (j = caracter+1; j<=terminador; j++){
                        cadena_tercera_linea[j-caracter-1] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_tercera_linea"
                    }
                }
                else{                                            //Si el mensaje no termina en la tercera l�nea
                    if(k >= 120 && cadena_mensaje[k] == ' ' && flag_tercera_linea == 0){
                        for (j = caracter+1; j<=k; j++){
                            cadena_tercera_linea[j-caracter-1] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_tercera_linea" hasta el primer
                        }                                                           //espacio encontrado del car�cter n�mero 120 en adelante
                        caracter = k;           //Almacenamos en "caracter" el �ndice desde el que debemos empezar a guardar en la siguiente l�nea
                        flag_tercera_linea = 1; //Activamos "flag_tercera_linea" para realizar el almacenamiento del mensaje de manera secuencial
                    }
                }
            }

            /************************************************* RENGL�N 4 ***********************************************************/
            //Se separa el cuarto rengl�n del mensaje entrante que cabe en la pantalla (en torno al
            //car�cter 160 se llega al lateral derecho de la pantalla con un cierto margen de seguridad)
            if(flag_tercera_linea == 1){    //Esperamos a que se hayan realizado las operaciones necesarias para rellenar la tercera l�nea
                if(terminador > 120 && terminador <= 160){      //Si el mensaje termina en la cuarta l�nea
                    for (j = caracter+1; j<=terminador; j++){
                        cadena_cuarta_linea[j-caracter-1] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_cuarta_linea"
                    }
                }
                else{                                            //Si el mensaje no termina en la cuarta l�nea
                    if(k >= 160 && cadena_mensaje[k] == ' ' && flag_cuarta_linea == 0){
                        for (j = caracter+1; j<=k; j++){
                            cadena_cuarta_linea[j-caracter-1] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_cuarta_linea" hasta el primer
                        }                                                          //espacio encontrado del car�cter n�mero 120 en adelante
                        caracter = k;           //Almacenamos en "caracter" el �ndice desde el que debemos empezar a guardar en la siguiente l�nea
                        flag_cuarta_linea = 1;  //Activamos "flag_cuarta_linea" para realizar el almacenamiento del mensaje de manera secuencial
                    }
                }
            }

            /************************************************* RENGL�N 5 ***********************************************************/
            //Se separa el quinto rengl�n del mensaje entrante.
            //Aqu� ya debemos guardar desde donde se qued� la cuarta l�nea hasta el terminador
            if(flag_cuarta_linea == 1){     //Esperamos a que se hayan realizado las operaciones necesarias para rellenar la cuarta l�nea
                for (j = caracter+1; j<=terminador; j++){
                    cadena_quinta_linea[j-caracter-1] = cadena_mensaje[j]; //Se guarda el mensaje en "cadena_quinta_linea" hasta el final
                }
            }
            k = k + 1;
        }
    }

    // Si el aviso no contiene mensaje
    else {
        ComColor(255, 255, 255);
        ComTXT(HSIZE/2, VSIZE/2, 23, OPT_CENTER, "No hay mensaje adjunto");
    }

    ComColor(255, 255, 255);
    ComTXT(HSIZE/10, VSIZE/5+10, 23, OPT_CENTERY, cadena_primera_linea);
    ComTXT(HSIZE/10, 2*VSIZE/5-10, 23, OPT_CENTERY, cadena_segunda_linea);
    ComTXT(HSIZE/10, 3*VSIZE/5-30, 23, OPT_CENTERY, cadena_tercera_linea);
    ComTXT(HSIZE/10, 6*VSIZE/10+4, 23, OPT_CENTERY, cadena_cuarta_linea);
    ComTXT(HSIZE/10, 7*VSIZE/10+11, 23, OPT_CENTERY, cadena_quinta_linea);
}

// Funci�n que define la pantalla del estado de env�o de ayuda (estado 8)
void Pantalla_ayuda(void){
    Nueva_pantalla(51,222,204);
    ComColor(255, 255, 255);
    ComTXT(HSIZE/2-30, VSIZE/2-20, 24, OPT_CENTER, "Enviando ayuda");

    ComCirculo(HSIZE/2+67, VSIZE/2-13, 5);
    ComCirculo(HSIZE/2+82, VSIZE/2-13, 5);
    ComCirculo(HSIZE/2+97, VSIZE/2-13, 5);

    //Seg�n el valor de "contador_estado8", se anima el movimiento del cochecito por la pantalla
    ComColor(209,170,0);
    ComRect(contador_estado8*HSIZE/7+HSIZE/10-6, VSIZE/2+20, contador_estado8*HSIZE/7+3.25*HSIZE/10, 2.25*VSIZE/3+10, true);
    ComColor(0,0,0);
    ComCirculo(contador_estado8*HSIZE/7+HSIZE/10+12, 2.25*VSIZE/3+13, 15);
    ComCirculo(contador_estado8*HSIZE/7+3.25*HSIZE/10-18, 2.25*VSIZE/3+13, 15);
}

// Funci�n que permite vaciar las cadenas que muestran el mensaje adjunto, si lo hubiese
void Reset_Cadenas_Mensaje(void){
    memset(cadena_primera_linea, 0, 50);
    memset(cadena_segunda_linea, 0, 50);
    memset(cadena_tercera_linea, 0, 50);    //Vaciamos las cadenas que se rellenan para mostrar por pantalla el mensaje del aviso en caso de que tenga
    memset(cadena_cuarta_linea, 0, 50);
    memset(cadena_quinta_linea, 0, 50);
    memset(cadena_mensaje, 0, 50);
}

// Funci�n que permite comprobar si se ha pulsado alguno de los registros Urgentes, y cu�l de ellos ha sido el que se ha pulsado
void Comprueba_Pulsacion_Registros_Urgentes(void){
    if(POSX > HSIZE/13 && POSX <= HSIZE/2 && POSY > VSIZE/3+3 && POSY <= VSIZE/2-11 && mensajes_urgentes > 0){
        registro = 1;
        IndiceUrgente = 0;
        estado = 6;
    }
    if(POSX > HSIZE/13 && POSX <= HSIZE/2 && POSY > VSIZE/2-11 && POSY <= VSIZE/2+21 && mensajes_urgentes > 1){
        registro = 2;
        IndiceUrgente = 1;
        estado = 6;
    }
    if(POSX > HSIZE/13 && POSX <= HSIZE/2 && POSY > VSIZE/2+21 && POSY <= 7*VSIZE/10-1 && mensajes_urgentes > 2){
        registro = 3;
        IndiceUrgente = 2;
        estado = 6;
    }
    if(POSX > HSIZE/13 && POSX <= HSIZE/2 && POSY > 7*VSIZE/10-1 && POSY <= 8*VSIZE/10+3 && mensajes_urgentes > 3){
        registro = 4;
        IndiceUrgente = 3;
        estado = 6;
    }
    if(POSX > HSIZE/13 && POSX <= HSIZE/2 && POSY > 8*VSIZE/10+3 && POSY <= 8*VSIZE/10+36 && mensajes_urgentes > 4 && activa_estado_1 >= 2){
        registro = 5;
        IndiceUrgente = 4;
        estado = 6;
    }
}

// Funci�n que permite comprobar si se ha pulsado alguno de los registros NO Urgentes, y cu�l de ellos ha sido el que se ha pulsado
void Comprueba_Pulsacion_Registros_No_Urgentes(void){
    if(POSX >= HSIZE/2 && POSX <= HSIZE-23 && POSY > VSIZE/3+3 && POSY <= VSIZE/2-11 && mensajes_no_urgentes > 0){
        registro = 1;
        IndiceNoUrgente = 0;
        estado = 7;
    }
    if(POSX >= HSIZE/2 && POSX <= HSIZE-23 && POSY > VSIZE/2-11 && POSY <= VSIZE/2+21 && mensajes_no_urgentes > 1){
        registro = 2;
        IndiceNoUrgente = 1;
        estado = 7;
    }
    if(POSX >= HSIZE/2 && POSX <= HSIZE-23 && POSY > VSIZE/2+21 && POSY <= 7*VSIZE/10-1 && mensajes_no_urgentes > 2){
        registro = 3;
        IndiceNoUrgente = 2;
        estado = 7;
    }
    if(POSX >= HSIZE/2 && POSX <= HSIZE-23 && POSY > 7*VSIZE/10-1 && POSY <= 8*VSIZE/10+3 && mensajes_no_urgentes > 3){
        registro = 4;
        IndiceNoUrgente = 3;
        estado = 7;
    }
    if(POSX >= HSIZE/2 && POSX <= HSIZE-23 && POSY > 8*VSIZE/10+3 && POSY <= 8*VSIZE/10+36 && mensajes_no_urgentes > 4){
        registro = 5;
        IndiceNoUrgente = 4;
        estado = 7;
    }
}
