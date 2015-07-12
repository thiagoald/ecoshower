#define SP_VCC		9//sensor de presença
#define SP_TRIG 	10
#define SP_ECHO		11
#define SP_GND		12

#define VALVULA 	7//valvula solenoide

#define SF_PULSO 	2//sensor de fluxo

int estado = 1;
int autentic = 1;
int timeout = 0;
float vazao; //Variável para armazenar o valor em L/min
float media=0; //Variável para tirar a média a cada 1 minuto
int contaPulso; //Variável para a quantidade de pulsos
int i=0; //Variável para contagem



void setup() {
  //Pinos sensor de presença	
  pinMode (SP_VCC,OUTPUT);// vcc
  pinMode(SP_TRIG, OUTPUT); // TriG
  pinMode (SP_ECHO, INPUT);//Echo
  pinMode (SP_GND,OUTPUT);// GND
  //valvula solenoide
  pinMode(VALVULA,OUTPUT);
  Serial.begin(9600);  
  //sensor de fluxo
  pinMode(SF_PULSO, INPUT);
  attachInterrupt(0, incpulso, RISING); //Configura o pino 2(Interrupção 0) para trabalhar como interrupção
  Serial.println("\n\nInicio\n\n"); //Imprime Inicio na serial
  vazao = 0;

}

void loop()
{

digitalWrite(SP_VCC, HIGH);

  long duration, inches, cm;

  digitalWrite(SP_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(SP_TRIG, HIGH);
  delayMicroseconds(5);
  digitalWrite(SP_TRIG, LOW);
 
  duration = pulseIn(SP_ECHO, HIGH);

 
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
  if(estado != 2) /*esperando autenticar*/
  {
    if(cm > 100)
    {
        estado = 1;//autenticado e esperando alguem se aproximar(valvula fechada)
    }
    else
    {
        estado = 0;// autenticado,valvula aberta e com fluxo contando
    }
  }
  else{ //verifica autenticaçao...
        if( autentic == 1)
       {
         estado = 1;
     
       }
       else if(autentic == 0)/*atingiu timeout ou sem autenticaçao*/
       {
         estado = 2;
		 vazao=0;
       }
    }
 
  
  if(estado == 1)//esperando alguem...
  {
     digitalWrite(VALVULA,HIGH); //fecha a valvula
     timeout++;
     delay(100);
     if(timeout > 30 * 100)//valvula fechada e timeout correndo ate 30s, após atingir o timeout vai pro estado 2
     {
       estado = 2;
     }
   }
  else if(estado == 0)//tomando banho...
  {  timeout = 0;
      digitalWrite(VALVULA,LOW);//abre a valvula
      contaPulso = 0;   //Zera a variável para contar os giros por segundos
      sei();      //Habilita interrupção
      delay (1000); //Aguarda 1 segundo
      cli();      //Desabilita interrupção
  
      vazao += contaPulso/(6.12*60); //Converte para L/s
      media=media+vazao; //Soma a vazão para o calculo da media
      i++;
  
      Serial.print("Vazao: ");
      Serial.println(vazao); //Imprime na serial o valor da vazão
  //Serial.print(" L/min - "); //Imprime L/min
  //Serial.print(i); //Imprime a contagem i (segundos)
  //Serial.println("s"); //Imprime s indicando que está em segundos
  
    if(i==6000)
    {
      media = media/60; //Tira a media dividindo por 60
      Serial.print("\nMedia por minuto = "); //Imprime a frase Media por minuto =
      Serial.print(media); //Imprime o valor da media
      Serial.println(" L/min - "); //Imprime L/min
      media = 0; //Zera a variável media para uma nova contagem
      i=0; //Zera a variável i para uma nova contagem
      Serial.println("\n\nInicio\n\n"); //Imprime Inicio indicando que a contagem iniciou
    }
    
    
  
  }
 
/*
  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
 */
  delay(100);
}

long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

void incpulso ()
{ 
  contaPulso++; //Incrementa a variável de contagem dos pulsos
} 

