#define RED P2_1
#define GREEN P2_3
#define BLUE P2_5

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW);
  analogReference(200000);
}

char buffer[50];
int i;
int index = 0;
int flag_serial = 0;
int num = 0;
int cifra = 0;
int multiplicador = 0;
int pwm = 0;
int lux = 0; 
int duty_cycle = 0;
int duty_cycle_ant = 0;
void loop() {
  // put your main code here, to run repeatedly: 

    //Delay para sincronizarse con Cliente
    //delay(200);
     
    //Recepción de la cadena con valor del sensor y guardado en un buffer
    while (Serial.available()){
      buffer[index] = Serial.read();  //Lee un byte del buffer del puerto serie
      if(buffer[index] == ';') break;
      index++;
      flag_serial = 1;
    }
    
    if(flag_serial == 1){
      duty_cycle = decodifica(buffer, index);
      //Serial.println(duty_cycle);
      /*
      if(duty_cycle > 66) {
//          pwm = 255;
          //Serial.println(2000);
          digitalWrite(GREEN, HIGH);
          digitalWrite(RED, LOW);
          digitalWrite(BLUE, LOW);          
        }
      if(duty_cycle >= 33 && duty_cycle<=66) {
//          pwm = 160;
          //Serial.println(2000);
          digitalWrite(BLUE, HIGH);
          digitalWrite(RED, LOW);
          digitalWrite(GREEN, LOW);
        }
      if(duty_cycle < 33) {
//        pwm = 80;
        //Serial.println(2000);
        digitalWrite(RED, HIGH);
        digitalWrite(GREEN, LOW);
        digitalWrite(BLUE, LOW); 
      }*/
  }
    
    //if(duty_cycle != duty_cycle_ant){
    pwm = map(duty_cycle,0,100,0,255);
    analogWrite(P1_6, pwm);
    //}
    
   //Vaciar el buffer del puerto serie de la placa
    while (Serial.available() > 0){
      Serial.read();
    }
    //"Vaciar" el buffer de cadena recibida
    index = 0;
    flag_serial = 0;
    for(i = 0; i < 50; i++){
      buffer[i] = '0';
    }

    //duty_cycle_ant = duty_cycle;
    delay(300);
}

int decodifica(char buffer[], int index){
int i,j,k;
char lut[] = "0123456789ABCDF";
int valor = 0;
int termino = 1;
int traducido = 0;
    if(buffer[4] != '0'){
      for(k=0;k<=15;k++){
        if(buffer[4] == lut[k]) traducido = k;        
      }  
      valor += 16 * traducido;
    }
    if(buffer[5] != '0'){
      for(k=0;k<=15;k++){
        if(buffer[5] == lut[k]) traducido = k;        
      }  
      valor += traducido;
    }
return valor;
}
