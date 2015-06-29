#include <Ethernet.h>
#include <SPI.h>
#include <stdio.h>

#define SP_VCC    9          //sensor de presen�a
#define SP_TRIG   10
#define SP_ECHO   11
#define SP_GND    12
#define VALVULA   7          //valvula solenoide
#define SF_PULSO  2          //sensor de fluxo


#define ESPERA    1000       // Delay (em ms) entre um POST e outro
#define PORTA     3000       // Porta do servidor
#define BAUD_RATE 9600       // Taxa de comunicação serial

byte mac[]  = {  0x10, 0x1F, 0x74, 0x40, 0x7A, 0xD5 };     // Endereço MAC do shield ethernet
byte ipArduino[]   = {  192, 168, 1, 48};                  // IP estático do arduino (está configurado para o IP do grad 1)
char dadosDoBanho[1000];                                   // String onde escrevemos os dados (no formato JSON) para mandar pro servidor
char ipServidor[30] = "192.168.1.48";                      // IP local da máquina onde o servidor está rodando (MUDAR)

int estado = 0;                                            // Determina o estado em que a maquina se encontra 
int timeout = 0;                                           // contador para contar tempo
float vazao = 0;                                           // Variavel para armazenar o valor em L/min
float media=0;                                             // Variavel para tirar a media a cada 1 minuto
int contaPulso;                                            // Variavel para a quantidade de pulsos
int tempo= 30 * 100;
int validacaoBanho = 0;

EthernetClient client;

void setup()
{
  Ethernet.begin(mac);                                    // IP dinâmico  //Ethernet.begin(mac,ipArduino);  // IP estático
  Serial.begin(BAUD_RATE);
  Serial.print("Usando o seguinte IP: ");
  Serial.println(Ethernet.localIP());
  delay(2000);
  Serial.println("Conectando a rede local...");
  //Pinos sensor de presen�a  
  pinMode (SP_VCC,OUTPUT);                                // vcc
  pinMode(SP_TRIG, OUTPUT);                               // TriG
  pinMode (SP_ECHO, INPUT);                               // Echo
  pinMode (SP_GND,OUTPUT);                                // GND
  //valvula solenoide
  pinMode(VALVULA,OUTPUT);
  Serial.begin(9600);  
  //sensor de fluxo
  pinMode(SF_PULSO, INPUT);
  attachInterrupt(0, incpulso, RISING);                   // Configura o pino 2(Interrup��o 0) para trabalhar como interrup��o
  Serial.println("\n\nInicio\n\n");                       // Imprime Inicio na serial
  
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
  contaPulso++; //Incrementa a vari�vel de contagem dos pulsos
} 

void enviar(char *dados)
{
  if (client.connect(ipServidor,PORTA))                         //Envia o pacote de dados ao servidor 
  {           
    Serial.print("Enviando POST ao servidor: " );
    Serial.println(ipServidor );
    client.println("POST / HTTP/1.1");          
    client.print("Host: ");
    client.println(ipServidor);
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println("User-Agent: Arduino/1.0");
    client.print("Content-Length: ");
    client.println(strlen(dados));
    client.println();
    client.print(dados);
    client.println();
    client.stop();
  }                               
  else
  {
    Serial.print("Nao foi possivel conectar ao servidor!");               
  }
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
      
  if(estado==0)   //Autenticacao
  {
    delay(1000);            
    estado=1; // Sempre Autenticado, Banho eterno, Podemos botar uma condicional aqui para acabar o banho em um determinado momento

                           
    /*if(ServidorRetornaPositivamente)                    //Se o servidor permite o banho
        estado = 1;                                     //inicio do Banho (espera do chuveiro)
    else                                                //caso o servidor Negue o banho
    {
      if(validacaoBanho)                                //Checa se o banho Comecou e terminou, ou, se ele simplesmente nao comecou ainda
          estado = 3                                    //Caso o banho ja tenha comecado e foi negado agora por causa do desligamento do cliente (o cliente encerrou o banho), entao, precisamos atualizar o banho
    }                                                  //Se nao houve banho, esse laco esta em estado de espera para o inicio de um banho, entao, deve ficar por aqui mesmo
    */
  }

  if(estado == 1) // Espera Cliente entrar debaixo do chuveiro
  {
      validacaoBanho = 1;                               // ? Se o chuveiro nao abriu nenhuma vez, isso foi um banho ?
      digitalWrite(VALVULA,HIGH);                       //Fecha a valvula
      timeout++;                                        //Contando tempo
      delay(100);
      if(timeout > tempo)                               //Valvula fechada e timeout correndo
         estado = 3;                                    //Estado 3 = atualizacao
      else if(cm < 100)                                 //Cliente entra no alcance do chuveiro
         estado = 2;                                    //Estado 2 = inicio do banho
  }

  if(estado==2) // Banho acontecendo
  {
     
      timeout = 0;                                      //tempo zerado para um nova recontagem
      digitalWrite(VALVULA,LOW);                        //abre a valvula
      contaPulso = 0;                                   //Zera a vari�vel para contar os giros por segundos
      sei();                                            //Habilita interrupcao
      delay (1000);                                     //Aguarda 1 segundo
      cli();                                            //Desabilita interrupcao
      vazao += contaPulso/(6.12*60);                    //Converte para L/s
      media=media+vazao;                                //Soma a vazao para o calculo da media 
      Serial.print("Vazao: ");
      Serial.println(vazao);                            //Imprime na serial o valor da vazao
      
      //Serial.print(" L/min - "); //Imprime L/min
      //Serial.print(i); //Imprime a contagem i (segundos)
      //Serial.println("s"); //Imprime s indicando que esta em segundos
      
      if(cm > 100)                                      //Se o usuario sair do alcancedo chuveiro
      {
          digitalWrite(VALVULA,HIGH);                    //Fecha a valvula
          estado = 0;                                    //Vai para autenticacao
      }
         
  }
  if(estado==3)           
  {
      validacaoBanho = 0;
      sprintf(dadosDoBanho, "{\"Valor\" : 1 ,\"nome\" : \"fulano\" , \"Consumo\" : %d,  \"fluxoMedio\" : %d   \"duracao\":%d}", 10,20,30);
      delay(ESPERA);
      enviar(dadosDoBanho);
      estado=0;
  }
  delay(100);
}
 


