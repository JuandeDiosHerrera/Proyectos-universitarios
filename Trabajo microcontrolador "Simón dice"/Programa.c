#include <msp430.h> 

#include <msp430.h>

#include "grlib.h"

#include "Crystalfontz128x128_ST7735.h"
#include "HAL_MSP430G2_Crystalfontz128x128_ST7735.h"
#include <stdio.h>

#include <stdlib.h>

int lee_ch(char canal){
    ADC10CTL0 &= ~ENC;                  //deshabilita el ADC
    ADC10CTL1&=(0x0fff);                //Borra canal anterior
    ADC10CTL1|=canal<<12;               //selecciona nuevo canal
    ADC10CTL0|= ENC;                    //Habilita el ADC
    ADC10CTL0|=ADC10SC;                 //Empieza la conversión
    LPM0;                               //Espera fin en modo LPM0
    return(ADC10MEM);                   //Devuelve valor leido
    }

void inicia_ADC(char canales){
    ADC10CTL0 &= ~ENC;      //deshabilita ADC
    ADC10CTL0 = ADC10ON | ADC10SHT_3 | SREF_0|ADC10IE; //enciende ADC, S/H lento, REF:VCC, con INT
    ADC10CTL1 = CONSEQ_0 | ADC10SSEL_0 | ADC10DIV_0 | SHS_0 | INCH_0;
    //Modo simple, reloj ADC, division 1, Disparo soft, Canal 0
    ADC10AE0 = canales; //habilita los canales indicados
    ADC10CTL0 |= ENC; //Habilita el ADC
}

void conf_reloj(char VEL){
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
    switch(VEL){
    case 1:
        if (CALBC1_1MHZ != 0xFF) {
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
            DCOCTL = CALDCO_1MHZ;
        }
        break;
    case 8:

        if (CALBC1_8MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_8MHZ;      /* Set DCO to 8MHz */
            DCOCTL = CALDCO_8MHZ;
        }
        break;
    case 12:
        if (CALBC1_12MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_12MHZ;     /* Set DCO to 12MHz */
            DCOCTL = CALDCO_12MHZ;
        }
        break;
    case 16:
        if (CALBC1_16MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_16MHZ;     /* Set DCO to 16MHz */
            DCOCTL = CALDCO_16MHZ;
        }
        break;
    default:
        if (CALBC1_1MHZ != 0xFF) {
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
            DCOCTL = CALDCO_1MHZ;
        }
        break;

    }
    BCSCTL1 |= XT2OFF | DIVA_0;
    BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;

}

#define DO 30578
#define MI 24270
#define SOL 20408
#define SI 16202

Graphics_Context g_sContext;
char estado=0, estado_aux=0, estado_prima=0;
volatile int tiempo=0;                                       //La variable tiempo es la variable esencial de mi programa, esta controla el tiempo que se está
                                                             //en cada estado (que lo necesite), el tiempo para mostrar la secuencia, el tiempo que le queda al
int main(void)                                               //jugador, etc...
{Graphics_Rectangle contornoprogreso = {10,5,70,25};
Graphics_Rectangle rectanguloprogreso = {11,6,69,24};

Graphics_Rectangle contornorojo = {20,33,50,83};
Graphics_Rectangle rectangulorojo = {21,34,49,82};

Graphics_Rectangle contornoamarillo = {55,33,105,63};
Graphics_Rectangle rectanguloamarillo = {56,34,104,62};

Graphics_Rectangle contornoazul = {75,68,105,118};
Graphics_Rectangle rectanguloazul = {76,69,104,117};

Graphics_Rectangle contornoverde = {20,88,70,118};
Graphics_Rectangle rectanguloverde = {21,89,69,117};                      //Defino las variables que voy a utilizar y todos los rectángulos que aparecerán

char progreso=0, i, ronda=1, problemavision=0;
char secuencia[32], cadena[8], cadenapuntuacion[10], cadenaprogreso[20];
short puntuacion=0;
char t, eje=0, fallo=0, fase=1;                                           //"fase" aumenta si se completan las 32 rondas de una partida
unsigned long rojo, azul, amarillo, verde;                                //El tiempo para mostrar los elementos de la secuencia y para cada paso del jugador
int x=0, y=0;                                                             //lo he puesto de manera que, se reduce en múltiplos de 2: (2*tiempo)/(2*fase)
                                                                          //Poniendo 2*tiempo, consigo que en la primera fase el tiempo sea el deseado
WDTCTL = WDTPW | WDTHOLD;
conf_reloj(16);

TA1CTL=TASSEL_2|ID_3| MC_1;         //SMCLK, DIV=8 (2MHz), UP
TA1CCR0=19999;       //periodo=20000: 10ms
TA1CCTL0=CCIE;      //CCIE=1


// Configura buzzer
TA0CTL=TASSEL_2|ID_0|MC_1;    //SMCLK, divisor 1, modo UP
TA0CCR0=0;
TA0CCR1=0;
TA0CCTL0=0;
TA0CCTL1=OUTMOD_7;

P2DIR|=BIT6;             //P2.6 salida
P2SEL|= BIT6;            //P2.7 gpio
P2SEL2&=~(BIT6+BIT7);    //P2.6 pwm
P2SEL&=~(BIT7);          //P2.7 gpio

// Configura botones:
      P1DIR &= ~(BIT1+BIT2);           //Pines de entrada
      P1REN |= BIT1+BIT2;
      P1OUT |= BIT1+BIT2;              //Configuración de los botones P1.1 P1.2 y P2.5 para las interrupciones
      P1IE= BIT1+BIT2;
      P1IES= BIT1+BIT2;
      P1IFG=0;

      P2DIR &= ~(BIT5);                //Borramos flags pendientes, todos activos por flanco de bajada, de pull-up y activamos interrupciones en P1.1 P1.2 y P2.5
      P2REN |= BIT5;
      P2OUT |= BIT5;
      P2IE= BIT5;
      P2IES= BIT5;
      P2IFG=0;

inicia_ADC(BIT0+BIT3);

__bis_SR_register(GIE);                         //Habilita interrupciones

Crystalfontz128x128_Init();
Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);               //Inicio pantalla
Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);

Graphics_setFont(&g_sContext, &g_sFontCm18b);                                //Elegimos fuente
Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_AQUAMARINE);
Graphics_clearDisplay(&g_sContext);

rojo=0x0000;
rojo=255;
rojo<<=8;
rojo+=83;
rojo<<=8;
rojo+=83;

azul=0x0000;
azul=100;
azul<<=8;
azul+=177;
azul<<=8;                                        //Defino unos colores claros de rojo, azul y verde
azul+=251;
                                                 //El amarillo es un amarillo oscuro en su caso
amarillo=0x0000;
amarillo=212;
amarillo<<=8;
amarillo+=170;
amarillo<<=8;
amarillo+=0;

verde=0x0000;
verde=181;
verde<<=8;
verde+=255;
verde<<=8;
verde+=94;

estado=1;                                                     //Inicializo la variable estado, que será la que rija la máquina de estados principal
                                                              //La inicializo ahora porque sino la varible tiempo ya llevaría un cierto valor y queremos
while(1){                                                     //que sea en el estado uno cuando comience a contar
    switch(estado){
    case 1:
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
        Graphics_drawStringCentered(&g_sContext,"Bienvenido", 10, 63, 17, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext,"a Simon dice.", 13, 63, 47, OPAQUE_TEXT);        //Texto de bienvenida
        Graphics_drawStringCentered(&g_sContext,"Pulse el", 8, 63, 77, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext,"joystick...", 11, 63, 107, OPAQUE_TEXT);

        fase=1;

        if((tiempo>=0)&&(tiempo<=50))                    //0 a 0.5 segundos suena la nota DO
        {TA0CCR0=DO;
        TA0CCR1=DO*0.5;                                  //Duty cycle al 50%
        }

        if((tiempo>=50)&&(tiempo<=100))                  //0.5 a 1 segundos suena la nota MI
        {TA0CCR0=MI;
        TA0CCR1=MI*0.5;
        }

        if((tiempo>=100)&&(tiempo<=150))                 //1 a 1.5 segundos suena la nota SOL
        {TA0CCR0=SOL;
        TA0CCR1=SOL*0.5;
        }

        if(tiempo>=150)                                   //1.5 a 2 segundos suena la nota SI
        {TA0CCR0=SI;
        TA0CCR1=SI*0.5;
        tiempo=0;
        }

        if(!(P2IN&BIT5))                                  //Si se pulsa el botón del joystick
            {TA0CCR0=0;                                   //Buzzer no emite sonido
            TA0CCR1=0;

            srand(tiempo);                                //Cojo la semilla a partir de la variable tiempo

            for(i=0;i<32;i++)                             //El bucle for irá de 0 hasta 32 (sin incluirlo), es decir, una longitud de 32 que es la buscada
                {secuencia[i]=rand()%5;                   //Obtendremos un número de 0-4 ,que son los posibles restos al dividir entre 5
                if(secuencia[i]==0) secuencia[i]=1;       //Si el número es 0, pasará a valer 1
                }

            estado=8;
            tiempo=0;                                                            //Reseteo la variable tiempo
            Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);      //Una vez creo la secuencia, paso al estado 8 y borro la pantalla
            Graphics_clearDisplay(&g_sContext);
            }
        break;
    case 8:                                                                                 //El estado 7 y 8 se encuentran aquí porque los añadí al final,
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);                     //y por comodidad ya los dejé con ese valor
        Graphics_drawStringCentered(&g_sContext, "Problemas", 9, 63, 20, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, "para distinguir", 15, 63, 43, OPAQUE_TEXT);   //Pantalla para preguntar si la persona tiene problemas de visión
        Graphics_drawStringCentered(&g_sContext, "colores:", 7, 63, 66, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, "NO: Boton 1", 12, 63, 89, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, "SI: Boton 2", 12, 63, 112, OPAQUE_TEXT);

        if(!(P1IN&BIT1))
        {problemavision=0;                                                                      //Botón 1 si la persona no tiene problemas de visión y paso a estado 2
        estado=2;

        tiempo=0;

        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);                         //Borro pantalla
        Graphics_clearDisplay(&g_sContext);
        }
        if(!(P1IN&BIT2))                                                                        //Botón 1 si la persona no tiene problemas de visión y paso a estado 2
        {problemavision=1;
        estado=2;

        tiempo=0;

        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);                         //Borro pantalla
        Graphics_clearDisplay(&g_sContext);
        }
        break;
    case 7:
        if((tiempo>=20)&&(tiempo<=60))                                                          //El buzzer emite DO
        {TA0CCR0=DO;
        TA0CCR1=DO*0.5;
        }

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_drawStringCentered(&g_sContext, "Enhorabuena", 11, 63, 40, OPAQUE_TEXT);      //Mensaje de enhorabuena al superar una ronda
        Graphics_drawStringCentered(&g_sContext, "pasas de", 8, 63, 63, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext, "ronda", 5, 63, 86, OPAQUE_TEXT);

        if((tiempo>=60)&&(tiempo<=95))                                                         //El buzzer emite SI
        {TA0CCR0=SI;
        TA0CCR1=SI*0.5;
        }

        if(tiempo>=95)                                                                          //El buzzer deja de emitir sonido
        {TA0CCR0=0;
        TA0CCR1=0;
        }

        if(tiempo>=150)                                                                         //Pasado 1,5 segundos en este estado, paso al estado 2
        {tiempo=0;
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_clearDisplay(&g_sContext);
        estado=2;
        }
        break;
    case 2:
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        sprintf(cadena,"Ronda %d",ronda);                                                        //Muestro la ronda por la que va el jugador
        Graphics_drawStringCentered(&g_sContext, cadena, 8, 63, 63, OPAQUE_TEXT);

        if(tiempo>=100)                                                                          //Pasado 1 segundo en este estado, paso al estado 3
        {tiempo=0;
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_clearDisplay(&g_sContext);
        t=0;
        estado=3;
        }
        break;
    case 3:                                                                                    //Este estado está destinado a mostrar la secuecia por pantalla
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_drawRectangle(&g_sContext, &contornoprogreso);
        Graphics_drawRectangle(&g_sContext, &contornorojo);
        Graphics_drawRectangle(&g_sContext, &contornoamarillo);
        Graphics_drawRectangle(&g_sContext, &contornoazul);
        Graphics_drawRectangle(&g_sContext, &contornoverde);

        Graphics_drawString(&g_sContext,"0%", 3, 75, 6, OPAQUE_TEXT);                          //Pinto la pantalla de juego básica: cuatro rectángulos, rectángulo de
                                                                                               //progreso de ronda y porcentaje de progreso de ronda
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_RED);
        Graphics_fillRectangle(&g_sContext, &rectangulorojo);

        Graphics_setForegroundColor(&g_sContext, amarillo);
        Graphics_fillRectangle(&g_sContext, &rectanguloamarillo);

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_BLUE);
        Graphics_fillRectangle(&g_sContext, &rectanguloazul);

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_GREEN);
        Graphics_fillRectangle(&g_sContext, &rectanguloverde);

        if(tiempo>=50)                                                                        //Al cabo de 0,5 segundos en este estado, entro en la máquina de estados
        {tiempo=0;                                                                            //del bucle for

        estado_aux=1;

        for(t=0;t<ronda;)                                                                     //Bucle for para mostrar la secuencia a repetir que llegará hasta la ronda
            {switch(estado_aux){                                                              //en la que nos encontremos. La variable "estado_aux" rige esta máquina de
            case 1:                                                                           //estados
            if(secuencia[t]==1)
            {if(problemavision==0)                                                            //Color rojo=1 en la secuencia
                {Graphics_setForegroundColor(&g_sContext,rojo);
                Graphics_fillRectangle(&g_sContext, &rectangulorojo);                         //Pinto el rectángulo rojo de un rojo más claro que destaque del resto de
                }                                                                             //rectángulos

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);               //En este caso se coloca un círculo que permita al jugador distinguir
                Graphics_fillCircle(&g_sContext, 35, 58, 10);                                 //el rectángulo que se ha activado
                }

            TA0CCR0=DO;                                                                       //El rojo tiene asociado la nota DO
            TA0CCR1=DO*0.5;
            }
            if(secuencia[t]==2)
            {if(problemavision==0)                                                            //Color azul=2 en la secuencia
                {Graphics_setForegroundColor(&g_sContext,azul);
                Graphics_fillRectangle(&g_sContext, &rectanguloazul);                         //Pinto el rectángulo azul de un azul más claro que destaque del resto de
                }                                                                             //rectángulos

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);               //En este caso se coloca un círculo que permita al jugador distinguir
                Graphics_fillCircle(&g_sContext, 90, 93, 10);                                 //el rectángulo que se ha activado
                }

            TA0CCR0=MI;                                                                       //El azul tiene asociado la nota MI
            TA0CCR1=MI*0.5;
            }
            if(secuencia[t]==3)                                                               //Color amarillo=3 en la secuencia
            {if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_YELLOW);              //Pinto el rectángulo amarillo de un amarillo más claro que destaque del
                Graphics_fillRectangle(&g_sContext, &rectanguloamarillo);                     //resto de rectángulos
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);               //En este caso se coloca un círculo que permita al jugador distinguir
                Graphics_fillCircle(&g_sContext, 80, 48, 10);                                 //el rectángulo que se ha activado
                }

            TA0CCR0=SOL;                                                                      //El amarillo tiene asociado la nota SOL
            TA0CCR1=SOL*0.5;
            }
            if(secuencia[t]==4)                                                               //Color verde=4 en la secuencia
            {if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext, verde);                             //Pinto el rectángulo verde de un verde más claro que destaque del resto
                Graphics_fillRectangle(&g_sContext, &rectanguloverde);                        //de rectángulos
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);               //En este caso se coloca un círculo que permita al jugador distinguir
                Graphics_fillCircle(&g_sContext, 45, 103, 10);                                //el rectángulo que se ha activado
                }

            TA0CCR0=SI;                                                                       //El verde tiene asociado la nota SI
            TA0CCR1=SI*0.5;
            }

            if(tiempo>=(400/(2*fase)))                                                        //2 segundos mostrando cada color en la primera fase
            {tiempo=0;
            estado_aux=2;                                                                     //Paso al estado_aux 2
            TA0CCR0=0;                                                                        //Buzzer deja de emitir el último sonido asociado al último elemento
            TA0CCR1=0;                                                                        //mostrado de la secuencia
            }

            break;
            case 2:
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_RED);
            Graphics_fillRectangle(&g_sContext, &rectangulorojo);

            Graphics_setForegroundColor(&g_sContext, amarillo);                              //Cada cuadro vuelve a su color original, en caso de haber pintado el
            Graphics_fillRectangle(&g_sContext, &rectanguloamarillo);                        //círculo lo borrará.

            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_BLUE);
            Graphics_fillRectangle(&g_sContext, &rectanguloazul);

            Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_DARK_GREEN);
            Graphics_fillRectangle(&g_sContext, &rectanguloverde);

            if(tiempo>=(100/(2*fase)))
            {tiempo=0;                                                                      //0.5 segundos entre cada elemento de la secuencia para la fase 1
            estado_aux=1;
            t++;                                                                            //Aumentamos la variable "t" que va recorriendo la cadena
            }                                                                               //Dicha variable nos permitirá salir del bucle for

            break;
            }
            }

        if(t==ronda)                                                                       //Cuando salimos del bucle, t es igual a ronda
        {Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);                   //En ese caso, borramos la pantalla y pasamos al estado 4
        Graphics_clearDisplay(&g_sContext);
        estado=4;
        }
        }

        break;                                                                             //Break del estado 3
    case 4:
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_drawStringCentered(&g_sContext,"Tu turno", 8, 63, 63, OPAQUE_TEXT);       //Se  indica al jugador que va a comenzar a jugar
        if(tiempo>=100)
        {tiempo=0;                                                                         //Pasado 1 segundo, reseteo "tiempo", paso al estado 5 y borro la pantalla
        estado=5;
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_clearDisplay(&g_sContext);
        }
        break;
    case 5:                                                                                  //Este es el estado dedicado a que el jugador juegue
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_drawRectangle(&g_sContext, &contornoprogreso);
        Graphics_drawRectangle(&g_sContext, &contornorojo);
        Graphics_drawRectangle(&g_sContext, &contornoamarillo);                            //Pinto contornos de los 5 rectángulos, 4 del juego y el de progreso de
        Graphics_drawRectangle(&g_sContext, &contornoazul);                                //ronda
        Graphics_drawRectangle(&g_sContext, &contornoverde);

        Graphics_Rectangle rectanguloprogreso = {11,6,11,24};
        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_CHOCOLATE);                //Progreso inicial es 0
        Graphics_fillRectangle(&g_sContext, &rectanguloprogreso);

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);                    //Antes de empezar la ronda el progreso es 0%
        Graphics_drawString(&g_sContext,"0%", 3, 75, 6, OPAQUE_TEXT);

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_RED);
        Graphics_fillRectangle(&g_sContext, &rectangulorojo);

        Graphics_setForegroundColor(&g_sContext, amarillo);
        Graphics_fillRectangle(&g_sContext, &rectanguloamarillo);                          //Relleno los 4 rectángulos del juego

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_BLUE);
        Graphics_fillRectangle(&g_sContext, &rectanguloazul);

        Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_DARK_GREEN);
        Graphics_fillRectangle(&g_sContext, &rectanguloverde);

        estado_prima=1;                                                                    //Entro en la máquina de estados de la parte del jugador. Esta está regida
                                                                                           //por la variable "estado_prima"
        for(t=0;t<ronda;)
            {switch(estado_prima){
            case 1:
            x=lee_ch(0);                                                                   //Leo el eje x del joystick y lo guardo en la variable "x"
            y=lee_ch(3);                                                                   //Leo el eje y del joystick y lo guardo en la variable "y"

            if(x<200)                                                                      //Si muevo el joystick hacia la izquierda
            {TA0CCR0=DO;
            TA0CCR1=DO*0.5;                                                                //Suena DO

            if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext,rojo);                            //En caso de que el jugador no tenga problemas de visión, si pinta más
                Graphics_fillRectangle(&g_sContext, &rectangulorojo);                      //claro el rectángulo
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);            //Si tiene problemas de visión, se pinta un círculo sobre el rectángulo
                Graphics_fillCircle(&g_sContext, 35, 58, 10);                              //que seleccione
                }

            eje=1;                                                                         //Asignamos a la variable "eje" el valor 1, para comparar con el valor de
            }                                                                              //la secuencia

            if(x>800)                                                                      //Si muevo el joystick hacia la derecha
            {TA0CCR0=MI;
            TA0CCR1=MI*0.5;                                                                //Suena MI

            if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext,azul);                            //En caso de que el jugador no tenga problemas de visión, si pinta más
                Graphics_fillRectangle(&g_sContext, &rectanguloazul);                      //claro el rectángulo
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);            //Si tiene problemas de visión, se pinta un círculo sobre el rectángulo
                Graphics_fillCircle(&g_sContext, 90, 93, 10);                              //que seleccione
                }

            eje=2;                                                                         //Asignamos a la variable "eje" el valor 2, para comparar con el valor de
            }                                                                              //la secuencia

            if(y<200)                                                                      //Si muevo el joystick hacia abajo
            {TA0CCR0=SI;
            TA0CCR1=SI*0.5;                                                                //Suena SI

            if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext, verde);                          //En caso de que el jugador no tenga problemas de visión, si pinta más
                Graphics_fillRectangle(&g_sContext, &rectanguloverde);                     //claro el rectángulo
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);            //Si tiene problemas de visión, se pinta un círculo sobre el rectángulo de
                Graphics_fillCircle(&g_sContext, 45, 103, 10);                             //la secuencia
                }

            eje=4;                                                                         //Asignamos a la variable "eje" el valor 4, para comparar con el valor de
            }                                                                              //la secuencia

            if(y>800)                                                                      //Si muevo el joystick hacia arriba
            {TA0CCR0=SOL;
            TA0CCR1=SOL*0.5;                                                               //Suena SOL

            if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_YELLOW);           //En caso de que el jugador no tenga problemas de visión, si pinta más
                Graphics_fillRectangle(&g_sContext, &rectanguloamarillo);                  //claro el rectángulo
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);            //Si tiene problemas de visión, se pinta un círculo sobre el rectángulo de
                Graphics_fillCircle(&g_sContext, 80, 48, 10);                              //la secuencia
                }

            eje=3;                                                                         //Asignamos a la variable "eje" el valor 3, para comparar con el valor de
            }                                                                              //la secuencia

            if((x>200)&&(x<800)&&(y>200)&&(y<800))                                         //Si el joystick está más o menos centrado, "eje" vale 0
            {eje=0;
            TA0CCR0=0;                                                                     //No suena nada
            TA0CCR1=0;
            }

            if((tiempo>=(800/(2*fase)))||((eje!=secuencia[t])&&(eje!=0)))                  //Si falla o pasan 4 segundos, termina la partida
            {TA0CCR0=0;
            TA0CCR1=0;                                                                     //"progreso" llevo el número de aciertos de la ronda actual

            tiempo=0;
            puntuacion+=progreso;                                                          //La variable "progreso" cuenta cuánto se debe sumar a la puntuación final
                                                                                           //que se corresponde con la variable "puntuacion"
            Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_clearDisplay(&g_sContext);                                            //Borro pantalla

            fallo=1;                                                                       //La variable "fallo" indicará si nos hemos equivocado

            t=ronda;                                                                       //Si t vale ronda saldremos del bucle for y por tanto de la máquina
            }                                                                              //de estados de la parte del jugador

            if((eje==secuencia[t])&&(tiempo<=(800/(2*fase)))) estado_prima=2;              //Si estamos dentro de tiempo y acertamos el elemento de la secuencia
                                                                                           //pasamos al "estado_prima" 2
            break;
            case 2:                                                                        //Solo paso al estado 2 si acierto
            x=lee_ch(0);
            y=lee_ch(3);                                                                   //Vuelvo a leer el joystick

            if(x<200)
            {TA0CCR0=DO;
            TA0CCR1=DO*0.5;

            if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext,rojo);
                Graphics_fillRectangle(&g_sContext, &rectangulorojo);
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);
                Graphics_fillCircle(&g_sContext, 35, 58, 10);
                }

            eje=1;
            }

            if(x>800)
            {TA0CCR0=MI;
            TA0CCR1=MI*0.5;

            if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext,azul);
                Graphics_fillRectangle(&g_sContext, &rectanguloazul);
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);
                Graphics_fillCircle(&g_sContext, 90, 93, 10);
                }

            eje=2;
            }

            if(y<200)
            {TA0CCR0=SI;                                                                       //Idéntico a lo del "estado_prima"=1
            TA0CCR1=SI*0.5;

            if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext,verde);
                Graphics_fillRectangle(&g_sContext, &rectanguloverde);
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);
                Graphics_fillCircle(&g_sContext, 45, 103, 10);
                }

            eje=4;
            }

            if(y>800)
            {TA0CCR0=SOL;
            TA0CCR1=SOL*0.5;

            if(problemavision==0)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_YELLOW);
                Graphics_fillRectangle(&g_sContext, &rectanguloamarillo);
                }

            if(problemavision==1)
                {Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_WHITE);
                Graphics_fillCircle(&g_sContext, 80, 48, 10);
                }

            eje=3;
            }

            if((x>200)&&(x<800)&&(y>200)&&(y<800))
            {eje=0;
            TA0CCR0=0;
            TA0CCR1=0;
            }

            if(eje==0)                                                                                    //Si he acertado y pongo el joystick en el centro,
            {Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_RED);                           //vuelve todo a su color
            Graphics_fillRectangle(&g_sContext, &rectangulorojo);

            Graphics_setForegroundColor(&g_sContext, amarillo);
            Graphics_fillRectangle(&g_sContext, &rectanguloamarillo);

            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_DARK_BLUE);
            Graphics_fillRectangle(&g_sContext, &rectanguloazul);

            Graphics_setForegroundColor(&g_sContext,GRAPHICS_COLOR_DARK_GREEN);
            Graphics_fillRectangle(&g_sContext, &rectanguloverde);
            }

            if((eje==0)&&(tiempo<=(800/(2*fase))))                                                      //Si sigo en tiempo y el joystick ha vuelto al centro,
            {tiempo=0;                                                                                  //"estado_prima"=3
                                                                                                        //Es aquí donde reseteamos el tiempo para cada paso
            estado_prima=3;
            }

            if((tiempo>=(800/(2*fase)))&&(eje!=0))                                                      //En caso de estar acertando pero no volver a poner
            {TA0CCR0=0;                                                                                 //el joystick en el centro, no se contabilizará como acierto
            TA0CCR1=0;

            tiempo=0;
            puntuacion+=progreso;                                                                       //Se suma el progreso que se llevase en esa ronda

            fallo=1;                                                                                    //Se considera fallo

            t=ronda;                                                                                    //t vale ronda para salir del bucle for y así salir de la
                                                                                                        //máquina de estados del jugador
            Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_clearDisplay(&g_sContext);                                                         //Borro pantalla
            }
            break;
            case 3:
            progreso++;                                                                                 //Aumentamos el progreso de la ronda
            sprintf(cadenaprogreso,"%d          ",100*progreso/ronda);

            Graphics_drawString(&g_sContext, cadenaprogreso, 3, 75, 6, OPAQUE_TEXT);                    //Definimos la cadena del progreso en tanto por ciento
            Graphics_drawString(&g_sContext, "%", 1, 100, 6, OPAQUE_TEXT);                              //y pintamos el rectángulo del progreso acorde con la cadena
                                                                                                        //del progreso
            Graphics_Rectangle rectanguloprogreso = {11,6,11+58*progreso/ronda,24};
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_CHOCOLATE);
            Graphics_fillRectangle(&g_sContext, &rectanguloprogreso);

            t++;                                                                                        //Aumentamos t para seguir recorriendo la secuencia
            estado_prima=1;                                                                             //Volvemos a "estado_prima"=1 para jugar para el siguiente
            break;                                                                                      //elemento de la secuencia
            }
            }

        if((t==ronda)&&(fallo==0))                                                                      //Si he salido del bucle for y no he fallado, sigo jugando
        {t=0;
        ronda++;                                                                                        //Aumentamos la ronda
        puntuacion+=progreso;                                                                           //Sumamos el progreso de la ronda

        tiempo=0;

        progreso=0;                                                                                     //Reseteamos el progreso para la ronda siguiente

        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);                                 //Borro pantalla
        Graphics_clearDisplay(&g_sContext);

        estado=7;                                                                                       //Paso al estado 7
        }

        if(fallo==1)                                                                                    //Si había fallado, la partida termina pasando al estado 6
        {progreso=0;
        fallo=0;                                                                                        //Reseteo "progreso" para la nueva partida y "fallo"

        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);                                 //Borro la pantalla
        Graphics_clearDisplay(&g_sContext);

        estado=6;
        }

        if((t==32)&&(fallo==0))                                                                         //Si he acabado la secuencia de 32 elementos
        {t=0;
        ronda=1;                                                                                        //Reseteamos las rondas

        tiempo=0;

        fase++;                                                                                         //Aumentamos la fase

        progreso=0;                                                                                     //Reseteamos el progreso para la fase siguiente

        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);                                 //Borro pantalla
        Graphics_clearDisplay(&g_sContext);

        estado=9;
        }
        break;                                                                                          //Break estado 5
    case 9:
        Graphics_drawStringCentered(&g_sContext,"Enhorabuena", 11, 63, 30, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext,"has completado", 14, 63, 53, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext,"la secuencia", 12, 63, 76, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext,"completa", 8, 63, 99, OPAQUE_TEXT);

        srand(tiempo);                                                                                 //Cojo la semilla a partir de la variable tiempo

        for(i=0;i<32;i++)                                           //El bucle for irá de 0 hasta 32 (sin incluirlo), es decir, una longitud de 32 que es la buscada
                {secuencia[i]=rand()%5;                             //Obtendremos un número de 0-4 ,que son los posibles restos al dividir entre 5
                if(secuencia[i]==0) secuencia[i]=1;                 //Si el número es 0, pasará a valer 1
                }

        if((tiempo>=20)&&(tiempo<=60))                                                          //El buzzer emite SI
        {TA0CCR0=SI;
        TA0CCR1=SI*0.5;
        }

        if((tiempo>=60)&&(tiempo<=95))                                                         //El buzzer emite DO
        {TA0CCR0=DO;
        TA0CCR1=DO*0.5;
        }

        if(tiempo>=95)                                                                         //El buzzer deja de emitir sonido
        {TA0CCR0=0;
        TA0CCR1=0;
        }

        if(tiempo>=150)                                                                        //Pasados 1,5 segundos vuelvo al estado 2 para comenzar una nueva fase
        {tiempo=0;

        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_clearDisplay(&g_sContext);

        estado=2;
        }
        break;
    case 6:
        TA0CCR0=0;                                                                                      //Buzzer deja de emitir sonido
        TA0CCR1=0;

        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
        Graphics_drawStringCentered(&g_sContext,"Tu puntacion", 13, 63, 40, OPAQUE_TEXT);
        Graphics_drawStringCentered(&g_sContext,"ha sido de:", 13, 63, 63, OPAQUE_TEXT);                //Se muestra la puntuación alcanzada por el jugador
        sprintf(cadenapuntuacion,"%d puntos",puntuacion);                                               //en la partida
        Graphics_drawStringCentered(&g_sContext,cadenapuntuacion, 10, 63, 86, OPAQUE_TEXT);

        problemavision=0;                                                                              //Reseteo "problemavision"
        ronda=1;                                                                                       //La ronda volverá a ser 1 ya que empieza una
        t=0;                                                                                           //nueva partida
        fallo=0;                                                                                       //Reseteo "fallo"

        if(tiempo>=200)                                                                                //Pasados 2 segundos
        {tiempo=0;
        puntuacion=0;                                                                                  //Reseteo "puntuacion", "progreso" y "tiempo"
        progreso=0;
        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_AQUAMARINE);                           //Pinto el fondo de la pantalla de inicio
        Graphics_clearDisplay(&g_sContext);
        estado=1;                                                                                      //Volvemos al estado inicial
        }
        break;
    }

LPM0;                                                                                                 //Modo de bajo consumo

}

    return 0;
}

char tms=0;
#pragma vector=TIMER1_A0_VECTOR    //Interrupción del timer
__interrupt void T1A0 (void)
{
tms++;

if(tms>=30)                      //300 ms
{
LPM0_EXIT;                       //Despierta al micro a los 300 ms

tms=0;                           //Reseteamos tms
}

if(estado==1||estado==7||estado==2||estado==3||estado_aux==1||estado_aux==2||estado==4||estado_prima==1||estado_prima==2||estado==9||estado==6) tiempo++;
                                                                                                                //Estados temporizados
}                                                                                                               //Todos ellos regidos por "tiempo"

#pragma vector=PORT1_VECTOR
__interrupt void Interrupcion_P1_12(void)        //Interrupción de P1.1 y P1.2
{
    if(P1IFG&BIT1)           //Si interrumpe el botón 1
    {

    LPM0_EXIT;               //Salgo del modo de bajo consumo

    P1IFG&=~BIT1;            //Borra flag de interrupción
    }

    if(P1IFG&BIT2)        //Si interrumpe el botón 2
    {

    LPM0_EXIT;            //Salgo del modo de bajo consumo

    P1IFG&=~BIT2;         //Borra flag de interrupción
    }
}

#pragma vector=PORT2_VECTOR
__interrupt void Interrupcion_P2_5(void)
{
    if(P2IFG&BIT5)          //Si interrumpe el botón del joystick
    {
    LPM0_EXIT;              //Salgo del modo de bajo consumo
    P2IFG&=~BIT5;           //Borra flag de interrupción

    }
}

#pragma vector = ADC10_VECTOR           //Interrupción del convertidor
__interrupt void ConvertidorAD (void)
{

LPM0_EXIT;                 // Despierta al micro al finalizar la conversión

}

