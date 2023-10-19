//Pines que tienen PWM --> P1_2, P1_6, P2_1, P2_2, P2_4, P2_5 y P2_6
#define RED P2_1
#define GREEN P2_3
#define BLUE P2_5
#define Pulsador P1_3
#define Sensor P1_5

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(Pulsador, INPUT_PULLUP);
  //pinMode(Sensor, INPUT);
  pinMode(BLUE, OUTPUT);
  analogReference(DEFAULT);  
  digitalWrite(BLUE, LOW);
}

int pwm = 0;
int valor = 0;
int sum = 0;
int medida = 0;
int lux = 0;
int n_medidas = 0;
unsigned long int suma_media = 0;
unsigned long int media = 0;
void loop() {
    n_medidas = 0;
    suma_media = 0;

    //Filtro de la media con 100 medidas
    while(n_medidas<100){
      valor = analogRead(A7);
      suma_media += valor;
      n_medidas++;
    }
    media = suma_media/n_medidas;
    lux = map(media,0,1023,1,6000);
    
    Serial.print("1:");
    Serial.print(lux, HEX);
    Serial.print(";");
    delay(50);
}
